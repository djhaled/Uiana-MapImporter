#include "PSKReader.h"

#include "ActorXUtils.h"


PSKReader::PSKReader(const FString Filename)
{
	Ar.open(ToCStr(Filename), std::ios::binary);
}

bool PSKReader::Read()
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

		if (CHUNK("PNTS0000"))
		{
			Vertices.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Vertices[i]), sizeof(FVector3f));
			}
		}
		else if (CHUNK("VTXW0000"))
		{
			Wedges.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Wedges[i]), sizeof(VVertex));
				if (DataCount <= 65536)
				{
					Wedges[i].PointIndex &= 0xFFFF;
				}
			}
		}
		else if (CHUNK("FACE0000"))
		{
			Faces.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				for (auto j = 0; j < 3; j++)
				{
					Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex[j]), sizeof(short));
					Faces[i].WedgeIndex[j] &= 0xFFFF;
				}
	                
				Ar.read(&Faces[i].MatIndex, sizeof(char));
				Ar.read(&Faces[i].AuxMatIndex, sizeof(char));
				Ar.read(reinterpret_cast<char*>(&Faces[i].SmoothingGroups), sizeof(unsigned));
			}
		}
		else if (CHUNK("FACE3200"))
		{
			Faces.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex), sizeof Faces[i].WedgeIndex);
				Ar.read(&Faces[i].MatIndex, sizeof(char));
                Ar.read(&Faces[i].AuxMatIndex, sizeof(char));
                Ar.read(reinterpret_cast<char*>(&Faces[i].SmoothingGroups), sizeof(unsigned));
			}
		}
		else if (CHUNK("MATT0000"))
		{
			Materials.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Materials[i]), sizeof(VMaterial));
			}
		}
		else if (CHUNK("VTXNORMS"))
		{
			Normals.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Normals[i]), sizeof(FVector3f));
			}
		}
		else if (CHUNK("VERTEXCOLOR"))
		{
			VertexColors.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&VertexColors[i]), sizeof(FColor));
			}
		}
		else if (CHUNK("EXTRAUVS"))
		{
			TArray<FVector2f> UVData;
			UVData.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&UVData[i]), sizeof(FVector2f));
			}

			ExtraUVs.Add(UVData);
		}
		else if (CHUNK("REFSKELT") || CHUNK("REFSKEL0"))
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
		else if (CHUNK("RAWWEIGHTS") || CHUNK("RAWW0000"))
		{
			Influences.SetNum(DataCount);
			for (auto i = 0; i < DataCount; i++)
			{
				Ar.read(reinterpret_cast<char*>(&Influences[i]), sizeof(VRawBoneInfluence));
			}
		}
		else
		{
			Ar.ignore(Chunk.DataSize*DataCount); 
		}
	}

	bHasVertexNormals = Normals.Num() > 0;
	bHasVertexColors = VertexColors.Num() > 0;
	bHasExtraUVs = ExtraUVs.Num() > 0;
	Ar.close();
	return true;
}

bool PSKReader::CheckHeader(const VChunkHeader Header) const
{
	return std::strcmp(Header.ChunkID, HeaderBytes) == 0;
}


