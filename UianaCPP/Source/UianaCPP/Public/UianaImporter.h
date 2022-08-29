#pragma once
#include "UianaCPPDataSettings.h"
#include <BPFL.h>

#include "UianaMap.h"

UCLASS()
class UIANACPP_API UUianaImporter : public UObject
{
public:
	UUianaImporter(FString MapName, UUianaCPPDataSettings Settings);
	static void ImportMap(UUianaCPPDataSettings Settings);
private:
	FDirectoryPath AssetsPath, ToolsPath;
	UUianaMap Map;
};
