// Fill out your copyright notice in the Description page of Project Settings.


#include "MaterialImporter.h"

#include "AssetToolsModule.h"
#include "BPFL.h"
#include "EditorAssetLibrary.h"
#include "JsonObjectConverter.h"
#include "MaterialEditingLibrary.h"
#include "ObjectEditorUtils.h"
#include "UianaHelpers.h"
#include "Components/DecalComponent.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "HAL/FileManagerGeneric.h"
#include "Materials/MaterialInstanceBasePropertyOverrides.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "UObject/PropertyAccessUtil.h"

MaterialImporter::MaterialImporter()
{
	Settings = nullptr;
}

MaterialImporter::MaterialImporter(const UUianaSettings* UianaSettings)
{
	Settings = UianaSettings;
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

void MaterialImporter::GetTexturePaths(const TArray<FString> matPaths, TArray<FString> &texturePaths)
{
	for (const FString matPath : matPaths)
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
					texturePaths.AddUnique(texturePath);
				}
			}
		}
	}
}

void MaterialImporter::CreateMaterials(const TArray<FString> matPaths)
{
	for(FString matPath : matPaths)
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

void MaterialImporter::SetMaterial(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat)
{
	SetTextures(matData, mat);
	SetMaterialSettings(matData->GetObjectField("Properties"), mat);
	const TSharedPtr<FJsonObject> matProps = matData.Get()->GetObjectField("Properties");
	if (matProps.Get()->HasTypedField<EJson::Object>("BasePropertyOverrides"))
	{
		FMaterialInstanceBasePropertyOverrides overrides = SetBasePropertyOverrides(matProps.Get()->GetObjectField("BasePropertyOverrides"));
		mat->BasePropertyOverrides = overrides;
	}
	if (matProps.Get()->HasField("StaticParameters"))
	{
		if (matProps.Get()->GetObjectField("StaticParameters")->HasField("StaticSwitchParameters"))
		{
			for (TSharedPtr<FJsonValue> param : matProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticSwitchParameters"))
			{
				const TSharedPtr<FJsonObject> paramObj = param.Get()->AsObject();
				const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
				UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(mat, FName(*paramInfo.Get()->GetStringField("Name").ToLower()), paramObj.Get()->GetBoolField("Value"));
			}
		}
		if (matProps.Get()->GetObjectField("StaticParameters")->HasField("StaticComponentMaskParameters"))
		{
			for (TSharedPtr<FJsonValue> param : matProps.Get()->GetObjectField("StaticParameters")->GetArrayField("StaticComponentMaskParameters"))
			{
				const TArray<FString> maskList = {"R", "G", "B"};
				for (FString mask : maskList)
				{
					UMaterialEditingLibrary::SetMaterialInstanceStaticSwitchParameterValue(mat, FName(*mask), param.Get()->AsObject().Get()->GetBoolField(mask));
				}
			}
		}
	}
	if (matProps.Get()->HasField("ScalarParameterValues"))
	{
		for (TSharedPtr<FJsonValue> param : matProps.Get()->GetArrayField("ScalarParameterValues"))
		{
			const TSharedPtr<FJsonObject> paramObj = param.Get()->AsObject();
			const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
			UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(mat, FName(*paramInfo.Get()->GetStringField("Name").ToLower()), param.Get()->AsObject()->GetNumberField("ParameterValue"));
		}
	}
	if (matProps.Get()->HasField("VectorParameterValues"))
	{
		for (TSharedPtr<FJsonValue> param : matProps.Get()->GetArrayField("VectorParameterValues"))
		{
			FLinearColor vec;
			const TSharedPtr<FJsonObject> paramVal = param.Get()->AsObject()->GetObjectField("ParameterValue");
			vec.R = paramVal.Get()->GetNumberField("R");
			vec.G = paramVal.Get()->GetNumberField("G");
			vec.B = paramVal.Get()->GetNumberField("B");
			UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(mat, FName(*param.Get()->AsObject()->GetObjectField("ParameterInfo")->GetStringField("Name").ToLower()), vec);
		}
	}
	UMaterialEditingLibrary::UpdateMaterialInstance(mat);
	// TODO: Homogenize the Scalar/Vector/TextureParameterValues sections
}

void MaterialImporter::SetTextures(const TSharedPtr<FJsonObject> matData, UMaterialInstanceConstant* mat)
{
	if (!matData->GetObjectField("Properties")->HasField("TextureParameterValues"))
	{
		return;
	}
	const TArray<TSharedPtr<FJsonValue>> matTexParams = matData->GetObjectField("Properties")->GetArrayField("TextureParameterValues");
	for (const TSharedPtr<FJsonValue> param : matTexParams)
	{
		// TODO: Cleanup 1-line continues to be less big
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
			UMaterialEditingLibrary::SetMaterialInstanceTextureParameterValue(mat, *paramName, loadedTexture);
		}
		else UE_LOG(LogTemp, Warning, TEXT("Uiana: Missing texture at expected directory %s"), *FPaths::GetPath(localPath));
	}
}

FMaterialInstanceBasePropertyOverrides MaterialImporter::SetBasePropertyOverrides(const TSharedPtr<FJsonObject> matProps)
{
	FMaterialInstanceBasePropertyOverrides overrides = FMaterialInstanceBasePropertyOverrides();
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : matProps->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		FProperty* objectProp = overrides.StaticStruct()->FindPropertyByName(propName);
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number)
		{
			double value = prop.Value.Get()->AsNumber();
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				value = 14680;
			}
			// TODO: Fix importing float properties, cannot set OpacityClipValue due to some assertion
			if (!UianaHelpers::SetStructProperty<FDoubleProperty, double>(&overrides, objectProp, value))// && !UianaHelpers::SetStructProperty<FFloatProperty, float>(&overrides, objectProp, value))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type number for Override struct!"), *prop.Key);
			continue;
		}
		if (propType == EJson::Boolean)
		{
			bool value = prop.Value.Get()->AsBool();
			if (!UianaHelpers::SetStructProperty<FBoolProperty, bool>(&overrides, objectProp, value))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type bool for Override struct!"), *prop.Key);
			continue;
		}
		if (propType == EJson::String && propValue.Get()->AsString().Contains("::"))
		{
			FString temp, splitResult;
			propValue.Get()->AsString().Split("::", &temp, &splitResult);
			FJsonValueString valString = FJsonValueString(splitResult);
			propValue = MakeShareable<FJsonValueString>(&valString);
		}
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			const TArray<FName> params = {"R", "G", "B"};
			if (!UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type FLinearColor for Override struct!"), *prop.Key);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			const TArray<FName> params = {"X", "Y", "Z", "W"};
			if(!UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type FVector4 for Override struct!"), *prop.Key);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			const TArray<FName> params = {"R", "G", "B", "A"};
			if (!UianaHelpers::SetStructPropertiesFromJson<FDoubleProperty, double>(&overrides, objectProp, obj, params))
				UE_LOG(LogTemp, Warning, TEXT("Uiana: Failed to set property %s of type Color for Override struct!"), *prop.Key);
		}
		else
		{
			if (propType == EJson::String)
			{
				const FString dataStr = propValue.Get()->AsString();
				if (prop.Key.Equals("ShadingModel"))
				{
					overrides.ShadingModel = UianaHelpers::ParseShadingModel(dataStr);
				}
				else if (prop.Key.Equals("BlendMode"))
				{
					overrides.BlendMode = UianaHelpers::ParseBlendMode(dataStr);
				}
				else
				{
					UE_LOG(LogTemp, Display, TEXT("Uiana: Needed Override is String with value %s and CPP Class %s %s"), *propValue.Get()->AsString(), *objectProp->GetNameCPP(), *objectProp->GetCPPTypeForwardDeclaration());
				}
			}
			else UE_LOG(LogTemp, Display, TEXT("Uiana: Need to set Override %s somehow!"), *prop.Key);
		}
	}
	return overrides;
}

void MaterialImporter::SetMaterialSettings(const TSharedPtr<FJsonObject> matProps, UMaterialInstanceConstant* mat)
{
	// Loop through all JSON values (type <FString, TSharedPtr<FJsonValue>>)
	for (auto const& prop : matProps->Values)
	{
		TSharedPtr<FJsonValue> propValue = prop.Value;
		const FName propName = FName(*prop.Key);
		const FProperty* objectProp = PropertyAccessUtil::FindPropertyByName(propName, mat->GetClass());
		if (objectProp == nullptr) continue;
		const EJson propType = propValue.Get()->Type;
		if (propType == EJson::Number || propType == EJson::Boolean)
		{
			if (prop.Key.Equals("InfluenceRadius") && propValue.Get()->AsNumber() == 0)
			{
				FObjectEditorUtils::SetPropertyValue(mat, propName, 14680);
				continue;
			}
			FObjectEditorUtils::SetPropertyValue(mat, propName, prop.Value.Get()->AsNumber());
			continue;
		}
		if (propType == EJson::String && propValue.Get()->AsString().Contains("::"))
		{
			FString temp, splitResult;
			propValue.Get()->AsString().Split("::", &temp, &splitResult);
			FJsonValueString valString = FJsonValueString(splitResult);
			propValue = MakeShareable<FJsonValueString>(&valString);
		}
		if (objectProp->GetClass()->GetName().Equals("FLinearColor"))
		{
			FLinearColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			FObjectEditorUtils::SetPropertyValue(mat, propName, color);
		}
		else if (objectProp->GetClass()->GetName().Equals("FVector4"))
		{
			FVector4 vector;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			vector.X = obj->GetNumberField("X");
			vector.Y = obj->GetNumberField("Y");
			vector.Z = obj->GetNumberField("Z");
			vector.W = obj->GetNumberField("W");
			FObjectEditorUtils::SetPropertyValue(mat, propName, vector);
		}
		else if (objectProp->GetClass()->GetName().Contains("Color"))
		{
			FColor color;
			const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
			color.R = obj->GetNumberField("R");
			color.G = obj->GetNumberField("G");
			color.B = obj->GetNumberField("B");
			color.A = obj->GetNumberField("A");
			FObjectEditorUtils::SetPropertyValue(mat, propName, color);
		}
		else if (propType == EJson::Object)
		{
			if (prop.Key.Equals("IESTexture"))
			{
				FString temp, newTextureName;
				propValue.Get()->AsObject()->GetStringField("ObjectName").Split("_", &temp, &newTextureName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
				FString assetPath = "/UianaCPP/IESProfiles/" + newTextureName + "." + newTextureName;
				UTexture* newTexture = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset(assetPath));
				FObjectEditorUtils::SetPropertyValue(mat, propName, newTexture);
			}
			else if (prop.Key.Equals("Cubemap"))
			{
				FString newCubemapName = propValue.Get()->AsObject()->GetStringField("ObjectName").Replace(TEXT("TextureCube "), TEXT(""));
				FString assetPath = "/UianaCPP/CubeMaps/" + newCubemapName + "." + newCubemapName;
				// TODO: Convert all static_cast with UObjects to Cast<>()
				UTextureCube* newCube = Cast<UTextureCube, UObject>(UEditorAssetLibrary::LoadAsset(assetPath));
				FObjectEditorUtils::SetPropertyValue(mat, propName, newCube);
			}
			else if (prop.Key.Equals("DecalMaterial"))
			{
				// TODO: Implement remaining object imports. Line 303 in main.py
				FString decalPath = FPaths::GetPath(propValue.Get()->AsObject()->GetStringField("ObjectPath"));
				UMaterialInstanceConstant* decalMat = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + decalPath + "." + decalPath));
				UDecalComponent decalComponent;
				decalComponent.SetMaterial(0, mat);
				decalComponent.SetDecalMaterial(decalMat);
			}
			else if (prop.Key.Equals("DecalSize"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				FObjectEditorUtils::SetPropertyValue(mat, propName, vec);
			}
			else if (prop.Key.Equals("StaticMesh"))
			{
				FString meshName;
				if (propValue->AsObject()->TryGetStringField("ObjectName", meshName))
				{
					FString name = meshName.Replace(TEXT("StaticMesh "), TEXT(""), ESearchCase::CaseSensitive);
					UStaticMesh* mesh = static_cast<UStaticMesh*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Meshes/" + name));
					FObjectEditorUtils::SetPropertyValue(mat, propName, mesh);
				}
			}
			else if (prop.Key.Equals("BoxExtent"))
			{
				FVector vec;
				const TSharedPtr<FJsonObject> obj = propValue.Get()->AsObject();
				vec.X = obj->GetNumberField("X");
				vec.Y = obj->GetNumberField("Y");
				vec.Z = obj->GetNumberField("Z");
				FObjectEditorUtils::SetPropertyValue(mat, "BoxExtent", vec);
			}
			else if (prop.Key.Equals("LightmassSettings"))
			{
				// TODO: Investigate why lightmass settings do not seem to be getting applied although no error!
				FLightmassMaterialInterfaceSettings lightmassSettings;
				FJsonObjectConverter::JsonObjectToUStruct(propValue.Get()->AsObject().ToSharedRef(), &lightmassSettings);
				if (!FObjectEditorUtils::SetPropertyValue(mat, "LightmassSettings", lightmassSettings)) UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to set lightmass settings!"));
			}
		}
		else if (propType == EJson::Array && prop.Key.Equals("OverrideMaterials"))
		{
			TArray<UMaterialInstanceConstant*> overrideMats;
			UMaterialInstanceConstant* loaded = nullptr;
			for (TSharedPtr<FJsonValue, ESPMode::ThreadSafe> ovrMat : propValue.Get()->AsArray())
			{
				if (!ovrMat.IsValid() || ovrMat.Get()->IsNull())
				{
					overrideMats.Add(nullptr);
					continue;
				}
				const TSharedPtr<FJsonObject> matData = ovrMat.Get()->AsObject();
				FString temp, objName;
				matData.Get()->GetStringField("ObjectName").Split(" ", &temp, &objName, ESearchCase::Type::IgnoreCase, ESearchDir::FromEnd);
				if (matData->HasTypedField<EJson::Object>("ObjectName")) UE_LOG(LogTemp, Error, TEXT("Uiana: Material Override Material ObjectName is not a string!"));
				if (objName.Contains("MaterialInstanceDynamic")) continue;
				loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/UianaCPP/Materials/" + objName));
				if (loaded == nullptr) loaded = static_cast<UMaterialInstanceConstant*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Materials/" + objName));
				overrideMats.Add(loaded);
			}
			FObjectEditorUtils::SetPropertyValue(mat, propName, overrideMats);
		}
		else
		{
			if (propType == EJson::Array)
			{
				const TArray<TSharedPtr<FJsonValue>> parameterArray = propValue.Get()->AsArray();
				if (prop.Key.Equals("TextureStreamingData"))
				{
					TArray<FMaterialTextureInfo> textures;
					for (const TSharedPtr<FJsonValue> texture : parameterArray)
					{
						FMaterialTextureInfo textureInfo;
						const TSharedPtr<FJsonObject> textureObj = texture.Get()->AsObject();
						FJsonObjectConverter::JsonObjectToUStruct(textureObj.ToSharedRef(), &textureInfo);
						textures.Add(textureInfo);
					}
					mat->SetTextureStreamingData(textures);
				}
				else
				{
					if (prop.Key.Equals("ScalarParameterValues") || prop.Key.Equals("VectorParameterValues") || prop.Key.Equals("TextureParameterValues")) continue;
					for (const TSharedPtr<FJsonValue> parameter : parameterArray)
					{
						const TSharedPtr<FJsonObject> paramObj = parameter.Get()->AsObject();
						// const TSharedPtr<FJsonObject> paramInfo = paramObj.Get()->GetObjectField("ParameterInfo");
						// FMaterialParameterInfo info;
						// FJsonObjectConverter::JsonObjectToUStruct(paramInfo.ToSharedRef(), &info);
						// if (prop.Key.Equals("ScalarParameterValues"))
						// {
						// 	double paramValue = paramObj.Get()->GetNumberField("ParameterValue");
						// 	mat->SetScalarParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("VectorParameterValues"))
						// {
						// 	FLinearColor paramValue;
						// 	const TSharedPtr<FJsonObject> vectorParams = paramObj.Get()->GetObjectField("ParameterValue");
						// 	paramValue.R = vectorParams.Get()->GetNumberField("R");
						// 	paramValue.G = vectorParams.Get()->GetNumberField("G");
						// 	paramValue.B = vectorParams.Get()->GetNumberField("B");
						// 	paramValue.A = vectorParams.Get()->GetNumberField("A");
						// 	mat->SetVectorParameterValueEditorOnly(info, paramValue);
						// }
						// else if (prop.Key.Equals("TextureParameterValues"))
						// {
						// 	FString temp, textureName;
						// 	paramObj.Get()->GetObjectField("ParameterValue").Get()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &textureName);
						// 	UTexture* paramValue = static_cast<UTexture*>(UEditorAssetLibrary::LoadAsset("/Game/ValorantContent/Textures/" + textureName));
						// 	mat->SetTextureParameterValueEditorOnly(info, paramValue);
						// }
						// else
						// {
						FString OutputString;
						TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
						FJsonSerializer::Serialize(paramObj.ToSharedRef(), Writer);
						UE_LOG(LogTemp, Warning, TEXT("Uiana: Array Material Property unaccounted for with value: %s"), *OutputString);	
						// }
					}	
				}
			}
			else UE_LOG(LogTemp, Warning, TEXT("Uiana: Need to set Material Property %s of unknown type!"), *prop.Key);
		}
	}
}

TArray<UMaterialInterface*> MaterialImporter::CreateOverrideMaterials(const TSharedPtr<FJsonObject> obj)
{
	TArray<UMaterialInterface*> mats = {};
	for (const TSharedPtr<FJsonValue> mat : obj->GetObjectField("Properties")->GetArrayField("OverrideMaterials"))
	{
		if (mat.Get()->Type != EJson::Object)
		{
			FString OutputString;
			TSharedRef< TJsonWriter<> > Writer = TJsonWriterFactory<>::Create(&OutputString);
			TArray<TSharedPtr<FJsonValue>> TempArr = {mat};
			FJsonSerializer::Serialize(TempArr, Writer);
			UE_LOG(LogTemp, Error, TEXT("Uiana: Override Material in Array is not Object and instead is:\n %s!"), *OutputString);
			continue;
		}
		FString objName, temp;
		mat->AsObject()->GetStringField("ObjectName").Split(TEXT(" "), &temp, &objName, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
		if (objName.Equals("Stone_M2_Steps_MI1")) objName = "Stone_M2_Steps_MI";
		if (objName.Contains("MaterialInstanceDynamic")) continue;
		mats.Add(Cast<UMaterialInterface>(UEditorAssetLibrary::LoadAsset(FPaths::Combine("/Game/ValorantContent/Materials/", objName))));
	}
	return mats;
}