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
	TArray<FString> matPaths, matOvrPaths;
	TArray<FString> texturePaths = {};
	FFileManagerGeneric::Get().FindFiles(matPaths, *Settings->MaterialsPath.Path, TEXT(".json"));
	FFileManagerGeneric::Get().FindFiles(matOvrPaths, *Settings->MaterialsOvrPath.Path, TEXT(".json"));
	UianaHelpers::AddPrefixPath(Settings->MaterialsPath, matPaths);
	UianaHelpers::AddPrefixPath(Settings->MaterialsOvrPath, matOvrPaths);
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Found %d textures to import from path %s"), matPaths.Num(), *Settings->MaterialsPath.Path);
	GetTexturePaths(matPaths, texturePaths);
	GetTexturePaths(matOvrPaths, texturePaths);
	UBPFL::ImportTextures(texturePaths);
	
	// Import materials next
	UE_LOG(LogTemp, Warning, TEXT("Uiana: Importing Materials!"));
	CreateMaterials(matPaths);
	CreateMaterials(matOvrPaths);
}

/**
 * Gets paths of all textures associated with list of materials
 * @param MatPaths Path of all materials to get corresponding texture paths from
 * @param TexturePaths FString Array of unique texture paths (using / separator) output
 */
void MaterialImporter::GetTexturePaths(const TArray<FString> MatPaths, TArray<FString> &TexturePaths)
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
	for(FString matPath : MatPaths)
	{
		// TODO: Simplify CreateMaterial() and GetTexturePaths() since they share same stuff
		// TODO: Make this a function already dangnabbit
		FString jsonStr;
		FFileHelper::LoadFileToString(jsonStr, *matPath);
		TArray<TSharedPtr<FJsonValue>> matObjs;
		const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(jsonStr);
		if (!FJsonSerializer::Deserialize(JsonReader, matObjs) || !matObjs[0].IsValid())
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Failed to deserialize material %s"), *matPath);
			continue;
		}
		const TSharedPtr<FJsonObject> mat = matObjs.Pop()->AsObject();
		if (!mat->HasField("Properties") || !mat->HasField("Name")) // Not sure about Name skip
		{
			UE_LOG(LogScript, Warning, TEXT("UIANA: Skipping due to missing properties for material %s"), *matPath);
			continue;
		}

		TArray<FStringFormatArg> args = {mat->GetStringField("Name"), mat->GetStringField("Name")};
		const FString localMatPath = FString::Format(TEXT("/Game/ValorantContent/Materials/{0}.{1}"), args);
		UMaterialInstanceConstant* matInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset(localMatPath));
		if (matInstance == nullptr)
		{
			UMaterialInstanceConstantFactoryNew* factory = NewObject<UMaterialInstanceConstantFactoryNew>();
			IAssetTools& assetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
			matInstance = static_cast<UMaterialInstanceConstant*>(assetTools.CreateAsset(mat->GetStringField("Name"), "/Game/ValorantContent/Materials/", UMaterialInstanceConstant::StaticClass(), factory));
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Created missing material %s"), *localMatPath);
		}
		
		FString matParent = "BaseEnv_MAT_V4";
		if (mat->GetObjectField("Properties")->HasField("Parent") && !mat->GetObjectField("Properties")->GetObjectField("Parent")->GetStringField("ObjectName").IsEmpty())
		{
			FString temp;
			mat->GetObjectField("Properties")->GetObjectField("Parent")->GetStringField("ObjectName").Split(TEXT(" "), &temp, &matParent);
		}
		UMaterialInstanceConstant* parentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + matParent));
		if (parentInstance == nullptr)
		{
			parentInstance = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/BaseEnv_MAT_V4"));
		}
		matInstance->SetParentEditorOnly(parentInstance);
		SetMaterial(mat, matInstance);
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Created material %s at %s"), *matPath, *localMatPath);
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
	const TSharedPtr<FJsonObject> matProps = MatData.Get()->GetObjectField("Properties");
	if (matProps.Get()->HasField("StaticParameters"))
	{
		if (matProps.Get()->GetObjectField("StaticParameters")->HasField("StaticSwitchParameters"))
		{
			for (TSharedPtr<FJsonValue> param : matProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticSwitchParameters"))
			{
				const TSharedPtr<FJsonObject> paramObj = param.Get()->AsObject();
				const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
				UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(Mat, FName(*paramInfo.Get()->GetStringField("Name").ToLower()), paramObj.Get()->GetBoolField("Value"));
			}
		}
		if (matProps.Get()->GetObjectField("StaticParameters")->HasField("StaticComponentMaskParameters"))
		{
			for (TSharedPtr<FJsonValue> param : matProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticComponentMaskParameters"))
			{
				const TArray<FString> maskList = {"R", "G", "B"};
				for (FString mask : maskList)
				{
					UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(Mat, FName(*mask), param.Get()->AsObject().Get()->GetBoolField(mask));
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
		TArray<FMaterialTextureInfo> textures;
		for (const TSharedPtr<FJsonValue> texture : JsonPropValue.Get()->AsArray())
		{
			FMaterialTextureInfo textureInfo;
			const TSharedPtr<FJsonObject> textureObj = texture.Get()->AsObject();
			FJsonObjectConverter::JsonObjectToUStruct(textureObj.ToSharedRef(), &textureInfo);
			textures.Add(textureInfo);
		}
		BaseObj->SetTextureStreamingData(textures);
	}
	else if (JsonPropName.Equals("ScalarParameterValues"))
	{
		for (TSharedPtr<FJsonValue> param : JsonPropValue.Get()->AsArray())
		{
			const TSharedPtr<FJsonObject> paramObj = param.Get()->AsObject();
			const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
			UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(BaseObj, FName(*paramInfo.Get()->GetStringField("Name").ToLower()), param.Get()->AsObject()->GetNumberField("ParameterValue"));
		}
	}
	else if (JsonPropName.Equals("VectorParameterValues"))
	{
		for (TSharedPtr<FJsonValue> param : JsonPropValue.Get()->AsArray())
		{
			FLinearColor vec;
			const TSharedPtr<FJsonObject> paramVal = param.Get()->AsObject()->GetObjectField("ParameterValue");
			vec.R = paramVal.Get()->GetNumberField("R");
			vec.G = paramVal.Get()->GetNumberField("G");
			vec.B = paramVal.Get()->GetNumberField("B");
			UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(BaseObj, FName(*param.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower()), vec);
		}
	}
	else if (JsonPropName.Equals("TextureParameterValues"))
	{
		for (const TSharedPtr<FJsonValue> param : JsonPropValue.Get()->AsArray())
		{
			if (!param.Get()->AsObject()->HasField("ParameterValue"))
			{
				continue;
			}
			FString textureGamePath = FPaths::ChangeExtension(param.Get()->AsObject()->GetObjectField("ParameterValue")->GetStringField("ObjectPath"), Settings->TextureFormat);
			FString localPath = FPaths::Combine(Settings->ExportAssetsPath.Path, textureGamePath.Replace(
							TEXT("ShooterGame"), TEXT("Game")).Replace(
								TEXT("/Content"), TEXT("")));
			const TSharedPtr<FJsonObject> paramInfo = param.Get()->AsObject()->GetObjectField("ParameterInfo");
			FString paramName = param.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower();
			if (FPaths::DirectoryExists(FPaths::GetPath(localPath)))
			{
				const FString texturePath = FPaths::Combine("/Game/ValorantContent/Textures/", FPaths::GetBaseFilename(localPath));
				UTexture* loadedTexture = static_cast<UTexture*>(
						UEditorAssetLibrary::LoadAsset(texturePath));
				if (loadedTexture == nullptr) continue;
				UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(BaseObj, *paramName, loadedTexture);
			}
			else UE_LOG(LogTemp, Warning, TEXT("Uiana: Missing texture at expected directory %s"), *FPaths::GetPath(localPath));
		}
	}
	else if (JsonPropName.Equals("StreamingTextureData"))
	{
		TArray<FMaterialTextureInfo> StreamingData;
		FJsonObjectConverter::JsonArrayToUStruct(JsonPropValue.Get()->AsArray(), &StreamingData);
		BaseObj->SetTextureStreamingData(StreamingData);
	}
	else
	{
		for (const TSharedPtr<FJsonValue> parameter : JsonPropValue.Get()->AsArray())
		{
			const TSharedPtr<FJsonObject> paramObj = parameter.Get()->AsObject();
			FString OutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
			FJsonSerializer::Serialize(paramObj.ToSharedRef(), Writer);
			UE_LOG(LogTemp, Warning, TEXT("Uiana: Array Material Property unaccounted for with value: %s"), *OutputString);	
		}
		return false;
	}
	return true;
}