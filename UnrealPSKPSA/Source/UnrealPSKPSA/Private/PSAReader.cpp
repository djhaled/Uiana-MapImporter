#include "PSAReader.h"

PSAReader::PSAReader(const FString Filename)
{
	Ar.open(ToCStr(Filename), std::ios::binary);
}

bool PSAReader::Read()
{
	VChunkHeader Header;
	Ar.read(reinterpret_cast<char*>(&Header), sizeof(VChunkHeader));

	if (!CheckHeader(Header))
		return false;

	VChunkHeader Chunk;
	while (!Ar.eof())
	{
		Ar.read(reinterpret_cast<char*>(&Chunk), sizeof(VChunkHeader));
		const auto DataCount = Chunk.DataCount;

		if (CHUNK("ANIMINFO"))
		{
			Ar.read(reinterpret_cast<char*>(&AnimInfo), sizeof(VAnimInfoBinary));
		}
		else if (CHUNK("BONENAMES"))
		{
			Bones.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Bones[i].Name), sizeof(Bones[i].Name));
				Ar.read(reinterpret_cast<char*>(&Bones[i].Flags), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Bones[i].NumChildren), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Bones[i].ParentIndex), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Orientation), sizeof(FQuat4f));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Position), sizeof(FVector3f));
	            	
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Length), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.XSize), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.YSize), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.ZSize), sizeof(float));
			}
		}
		else if (CHUNK("ANIMKEYS"))
		{
			AnimKeys.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&AnimKeys[i].Position), sizeof(FVector3f));
				Ar.read(reinterpret_cast<char*>(&AnimKeys[i].Orientation), sizeof(FQuat4f));
				Ar.read(reinterpret_cast<char*>(&AnimKeys[i].Time), sizeof(float));
			}
		}
		else if (CHUNK("SCALEKEYS"))
		{
			ScaleKeys.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&ScaleKeys[i].ScaleVector), sizeof(FVector3f));
				Ar.read(reinterpret_cast<char*>(&ScaleKeys[i].Time), sizeof(float));
			}
		}
		else
		{
			Ar.ignore(Chunk.DataSize*DataCount); 
		}
		
	}

	bHasScaleKeys = ScaleKeys.Num() > 0;

	return true;
}

bool PSAReader::CheckHeader(const VChunkHeader Header) const
{
	return std::strcmp(Header.ChunkID, HeaderBytes) == 0;
}

