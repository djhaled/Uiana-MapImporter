#pragma once
#include<filesystem>

class UianaHelpers
{
public:
	static void CreateFolder(FDirectoryPath &FolderPath, FString Root, FString Extension);
	static TSharedPtr<FJsonObject> UianaHelpers::ParseJson(FString InputStr);
};
