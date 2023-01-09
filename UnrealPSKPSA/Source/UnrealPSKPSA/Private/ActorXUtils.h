#pragma once
#if ENGINE_MAJOR_VERSION == 5
#include "AssetRegistry/AssetRegistryModule.h"
#else
#include "AssetRegistryModule.h"
#endif

class FActorXUtils
{
public:
	
	template <typename T>
	static T* LocalFindOrCreate(UClass* StaticClass, UObject* FactoryParent, FString Filename, EObjectFlags Flags)
	{
		auto Package = CreatePackage(nullptr, *FPaths::Combine(FPaths::GetPath(FactoryParent->GetPathName()), Filename));
		
		auto Asset = LoadObject<T>(Package, *Filename);
		if (!Asset)
		{
			Asset = NewObject<T>(Package, StaticClass, FName(Filename), Flags);
			Asset->PostEditChange();
			FAssetRegistryModule::AssetCreated(Asset);
			Asset->MarkPackageDirty();
		}

		return Asset;
	}

	template <typename T>
	static T* LocalCreate(UClass* StaticClass, UObject* FactoryParent, FString Filename, EObjectFlags Flags)
	{
		auto Package = CreatePackage(nullptr, *FPaths::Combine(FPaths::GetPath(FactoryParent->GetPathName()), Filename));
		
		auto Asset = NewObject<T>(Package, StaticClass, FName(Filename), Flags);
		return Asset;
	}
};
