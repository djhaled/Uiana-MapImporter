#pragma once
#include "JsonObjectConverter.h"
#include "UObject/PropertyAccessUtil.h"
#include "UianaSettings.h"
#include "Materials/MaterialInstanceConstant.h"

template<class ObjType>
class TBaseImporter
{
public:
	virtual ~TBaseImporter() = default;
	TBaseImporter();
	explicit TBaseImporter(const UianaSettings* UianaSettings);
	void SetSettingsFromJsonProperties(const TSharedPtr<FJsonObject> JsonProps, ObjType* BaseObj);
	
	virtual bool OverrideNumericProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, ObjType* BaseObj);
	virtual bool OverrideObjectProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, ObjType* BaseObj);
	virtual bool OverrideArrayProp(const FString JsonPropName, const TSharedPtr<FJsonValue> JsonPropValue, const FProperty* ObjectProp, ObjType* BaseObj);
protected:
	const UianaSettings* Settings;
};