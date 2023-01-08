// Fill out your copyright notice in the Description page of Project Settings.
#include "MaterialImporter.h"

MaterialImporter::MaterialImporter() : TBaseImporter<UMaterialInstanceConstant>()
{
}

MaterialImporter::MaterialImporter(const UianaSettings* UianaSettings) : TBaseImporter<UMaterialInstanceConstant>(UianaSettings)
{
	
}

void MaterialImporter::ImportMaterials()
{
	TArray<FString> MatPaths, MatOvrPaths;
	TArray<FString> TexturePaths = {};
	FFileManagerGeneric::Get().FindFiles(MatPaths, *Settings->MaterialsPath.Path, TEXT(".json"));
	FFileManagerGeneric::Get().FindFiles(MatOvrPaths, *Settings->MaterialsOvrPath.Path, TEXT(".json"));
	UianaHelpers::AddPrefixPath(Settings->MaterialsPath, MatPaths);
	UianaHelpers::AddPrefixPath(Settings->MaterialsOvrPath, MatOvrPaths);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Found %d textures to import from path %s"), MatPaths.Num(), *Settings->MaterialsPath.Path);
	GetTexturePaths(MatPaths, TexturePaths);
	GetTexturePaths(MatOvrPaths, TexturePaths);
	UBPFL::ImportTextures(TexturePaths);
	
	// Import materials next
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Materials!"));
	CreateMaterials(MatPaths);
	CreateMaterials(MatOvrPaths);
}

/**
 * Gets paths of all textures associated with list of materials
 * @param MatPaths Path of all materials to get corresponding texture paths from
 * @param TexturePaths FString Array of unique texture paths (using / separator) output
 */
void MaterialImporter::GetTexturePaths(const TArray<FString> MatPaths, TArray<FString> &TexturePaths) const
{
	for (const FString matPath : MatPaths)
	{
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *matPath);
		TArray<TSharedPtr<FJsonValue>> matObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(jsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, matObjs) || !matObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize material %s"), *matPath);
			continue;
		}
		for (TSharedPtr<FJsonValue> mat : matObjs)
		{
			const TSharedPtr<FJsonObject> obj = mat.Get()->AsObject();
			if (obj->HasField("Properties") && obj->GetObjectField("Properties")->HasField("TextureParameterValues"))
			{
				const TArray<TSharedPtr<FJsonValue>> texParams = obj->GetObjectField("Properties")->GetArrayField("TextureParameterValues");
				for (auto param : texParams)
				{
					const TSharedPtr<FJsonObject> paramObj = param->AsObject();
					if (!paramObj->HasTypedField<EJson::Object>("ParameterValue")) continue;
					const FString objectPath = paramObj->GetObjectField("ParameterValue")->GetStringField("ObjectPath");
					FString objectPathName = FPaths::GetBaseFilename(objectPath, false) + Settings->TextureFormat;
					const FString texturePath = FPaths::Combine(Settings->ExportAssetsPath.Path,
						objectPathName.Replace(
						TEXT("ShooterGame"), TEXT("Game")).Replace(
							TEXT("/Content"), TEXT("")));
					TexturePaths.AddUnique(texturePath);
				}
			}
		}
	}
}

/**
 * Creates all Materials provided in a list of paths.
 * @param MatPaths List of material paths to create materials from.
 */
void MaterialImporter::CreateMaterials(const TArray<FString> MatPaths)
{
	for(FString MatPath : MatPaths)
	{
		// TODO: Simplify CreateMaterial() and GetTexturePaths() since they share same stuff
		// TODO: Make this a function already dangnabbit
		FString JsonString;
		FFileHelper::LoadFileToString(JsonString, *MatPath);
		TArray<TSharedPtr<FJsonValue>> MatObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(JsonString);
		if (!FJsonSerializer::Deserialize(JsonReader, MatObjs) || !MatObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize material %s"), *MatPath);
			continue;
		}
		const TSharedPtr<FJsonObject> Mat = MatObjs.Pop()->AsObject();
		if (!Mat->HasField("Properties") || !Mat->HasField("Name")) // Not sure about Name skip
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Skipping due to missing properties for material %s"), *MatPath);
			continue;
		}

		TArray<FStringFormatArg> Args = {Mat->GetStringField("Name"), Mat->GetStringField("Name")};
		const FString LocalMatPath = FString::Format(TEXT("/Game/ValorantContent/Materials/{0}.{1}"), Args);
		UMaterialInstanceConstant* MatInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset(LocalMatPath));
		if (MatInstance == nullptr)
		{
			UMaterialInstanceConstantFactoryNew* MaterialFactory = NewObject<UMaterialInstanceConstantFactoryNew>();
			IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			MatInstance = static_cast<UMaterialInstanceConstant*>(AssetTools.CreateAsset(Mat->GetStringField("Name"), "/Game/ValorantContent/Materials/", UMaterialInstanceConstant::StaticClass(), MaterialFactory));
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Created missing material %s"), *LocalMatPath);
		}
		
		FString MatParent = "BaseEnv_MAT_V4";
		if (Mat->GetObjectField("Properties")->HasField("Parent") && !Mat->GetObjectField("Properties")->GetObjectField("Parent")->GetStringField("ObjectName").IsEmpty())
		{
			FString Temp;
			Mat->GetObjectField("Properties")->GetObjectField("Parent")->GetStringField("ObjectName").Split(TEXT(" "), &Temp, &MatParent);
		}
		UMaterialInstanceConstant* ParentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + MatParent));
		if (ParentInstance == nullptr)
		{
			ParentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/BaseEnv_MAT_V4"));
		}
		MatInstance->SetParentEditorOnly(ParentInstance);
		SetMaterial(Mat, MatInstance);
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Created material %s at %s"), *MatPath, *LocalMatPath);
	}
}

/**
 * Sets all material settings and updates it.
 * @param MatData JSON data which includes properties of the material
 * @param Mat Material to set settings on
 */
void MaterialImporter::SetMaterial(const TSharedPtr<FJsonObject> MatData, UMaterialInstanceConstant* Mat)
{
	SetSettingsFromJsonProperties(MatData->GetObjectField("Properties"), Mat);
	const TSharedPtr<FJsonObject> MatProps = MatData.Get()->GetObjectField("Properties");
	if (MatProps.Get()->HasField("StaticParameters"))
	{
		if (MatProps.Get()->GetObjectField("StaticParameters")->HasField("StaticSwitchParameters"))
		{
			for (TSharedPtr<FJsonValue> StaticParameter : MatProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticSwitchParameters"))
			{
				const TSharedPtr<FJsonObject> ParamObj = StaticParameter.Get()->AsObject();
				const TSharedPtr<FJsonObject> ParamInfo = ParamObj.Get()->GetObjectField("ParameterInfo");
				UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(Mat, FName(*ParamInfo.Get()->GetStringField("Name").ToLower()), ParamObj.Get()->GetBoolField("Value"));
			}
		}
		if (MatProps.Get()->GetObjectField("StaticParameters")->HasField("StaticComponentMaskParameters"))
		{
			for (TSharedPtr<FJsonValue> StaticParameter : MatProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticComponentMaskParameters"))
			{
				for (FString Mask : {"R", "G", "B"})
				{
					UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(Mat, FName(*Mask), StaticParameter.Get()->AsObject().Get()->GetBoolField(Mask));
				}
			}
		}
	}
	UMaterialEditingLibrary::UpdateMaterialInstance(Mat);
}

bool MaterialImporter::OverrideArrayProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue,
                                                                    const FProperty* ObjectProp, UMaterialInstanceConstant* BaseObj)
{
	if (JsonPropName.Equals("TextureStreamingData")) // | Materials
	{
		TArray<FMaterialTextureInfo> Textures;
		for (const TSharedPtr<FJsonValue> TextureJson : JsonPropValue.Get()->AsArray())
		{
			FMaterialTextureInfo TextureInfo;
			const TSharedPtr<FJsonObject> TextureObj = TextureJson.Get()->AsObject();
			FJsonObjectConverter::JsonObjectToUStruct(TextureObj.ToSharedRef(), &TextureInfo);
			Textures.Add(TextureInfo);
		}
		BaseObj->SetTextureStreamingData(Textures);
	}
	else if (JsonPropName.Equals("ScalarParameterValues"))
	{
		for (TSharedPtr<FJsonValue> ParameterValue : JsonPropValue.Get()->AsArray())
		{
			const TSharedPtr<FJsonObject> ParamObj = ParameterValue.Get()->AsObject();
			const TSharedPtr<FJsonObject> ParamInfo = ParamObj.Get()->GetObjectField("ParameterInfo");
			UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(BaseObj, FName(*ParamInfo.Get()->GetStringField("Name").ToLower()), ParameterValue.Get()->AsObject()->GetNumberField("ParameterValue"));
		}
	}
	else if (JsonPropName.Equals("VectorParameterValues"))
	{
		for (TSharedPtr<FJsonValue> ParameterValue : JsonPropValue.Get()->AsArray())
		{
			FLinearColor Vec;
			const TSharedPtr<FJsonObject> ParamVal = ParameterValue.Get()->AsObject()->GetObjectField("ParameterValue");
			Vec.R = ParamVal.Get()->GetNumberField("R");
			Vec.G = ParamVal.Get()->GetNumberField("G");
			Vec.B = ParamVal.Get()->GetNumberField("B");
			UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(BaseObj, FName(*ParameterValue.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower()), Vec);
		}
	}
	else if (JsonPropName.Equals("TextureParameterValues"))
	{
		for (const TSharedPtr<FJsonValue> ParameterValue : JsonPropValue.Get()->AsArray())
		{
			if (!ParameterValue.Get()->AsObject()->HasField("ParameterValue"))
			{
				continue;
			}
			FString TextureGamePath = FPaths::ChangeExtension(ParameterValue.Get()->AsObject()->GetObjectField("ParameterValue")->GetStringField("ObjectPath"), Settings->TextureFormat);
			FString LocalPath = FPaths::Combine(Settings->ExportAssetsPath.Path, TextureGamePath.Replace(
							TEXT("ShooterGame"), TEXT("Game")).Replace(
								TEXT("/Content"), TEXT("")));
			const TSharedPtr<FJsonObject> ParamInfo = ParameterValue.Get()->AsObject()->GetObjectField("ParameterInfo");
			FString ParamName = ParameterValue.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower();
			if (FPaths::DirectoryExists(FPaths::GetPath(LocalPath)))
			{
				const FString TexturePath = FPaths::Combine("/Game/ValorantContent/Textures/", FPaths::GetBaseFilename(LocalPath));
				UTexture* LoadedTexture = static_cast<UTexture*>(
						UEditorAssetLibrary::LoadAsset(TexturePath));
				if (LoadedTexture == nullptr) continue;
				UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(BaseObj, *ParamName, LoadedTexture);
			}
			else UE_LOG(LogTemp, Warning, TEXT("Uiana: Missing texture at expected directory %s"), *FPaths::GetPath(LocalPath));
		}
	}
	else
	{
		for (const TSharedPtr<FJsonValue> Parameter : JsonPropValue.Get()->AsArray())
		{
			const TSharedPtr<FJsonObject> ParamObj = Parameter.Get()->AsObject();
			FString OutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(ParamObj.ToSharedRef(), Writer);
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Array Material Property unaccounted for with value: %s"), *OutputString);	
		}
		return false;
	}
	return true;
}