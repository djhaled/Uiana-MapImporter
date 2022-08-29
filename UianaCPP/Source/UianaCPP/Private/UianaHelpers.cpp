#include "UianaHelpers.h"

void UianaHelpers::CreateFolder(FDirectoryPath& FolderPath, FString Root, FString Extension)
{
	FolderPath.Path = FPaths::Combine(Root, Extension);
	std::filesystem::create_directory(*(FolderPath.Path));
}