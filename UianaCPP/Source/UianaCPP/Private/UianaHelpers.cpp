#include "UianaHelpers.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UianaHelpers::CreateFolder(FDirectoryPath& FolderPath, FString Root, FString Extension)
{
	FolderPath.Path = FPaths::Combine(Root, Extension).Replace(TEXT("\\"), TEXT("/"));
	IPlatformFile& FileManager = FPlatformFileManager::Get().GetPlatformFile();
	if (FileManager.DirectoryExists(*(FolderPath.Path)))
	{
		UE_LOG(LogTemp, Warning, TEXT("Uiana: Not creating directory %s since it already exists."), *(FolderPath.Path));
	}
	else if (!FileManager.CreateDirectory(*(FolderPath.Path)))
	{
		UE_LOG(LogTemp, Error, TEXT("Uiana: Failed to create folder %s"), *(FolderPath.Path));
	}
}

TSharedPtr<FJsonObject> UianaHelpers::ParseJson(FString InputStr)
{
	TSharedPtr<FJsonObject> JsonParsed = MakeShareable(new FJsonObject());
	const TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(InputStr);
	if (FJsonSerializer::Deserialize(JsonReader, JsonParsed) && JsonParsed.IsValid())
	{
		return JsonParsed;
	}
	return nullptr;
}