#include "UianaHelpers.h"

#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

void UianaHelpers::CreateFolder(FDirectoryPath& FolderPath, FString Root, FString Extension)
{
	FolderPath.Path = FPaths::Combine(Root, Extension);
	std::filesystem::create_directory(*(FolderPath.Path));
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