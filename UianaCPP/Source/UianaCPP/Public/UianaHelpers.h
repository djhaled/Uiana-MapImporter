#pragma once

class UianaHelpers
{
public:
	static void CreateFolder(FDirectoryPath &FolderPath, FString Root, FString Extension);
	static TSharedPtr<FJsonObject> ParseJson(FString InputStr);
};
