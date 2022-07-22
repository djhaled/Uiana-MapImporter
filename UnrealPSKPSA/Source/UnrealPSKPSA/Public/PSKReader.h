#pragma once
#include <fstream>
#include <vector>
#include "CoreMinimal.h"

#define CHUNKHANDLER(chunkName) (strncmp(Chunk.ChunkID, chunkName, strlen(chunkName)) == 0)

struct VChunkHeader
{
	char ChunkID[20];
	int TypeFlag;
	int DataSize;
	int DataCount;
};
struct VVertex
{
	int PointIndex;
	float U, V;
	char MatIndex;
	char Reserved;
	short Pad;
};
struct VTriangle
{
	int WedgeIndex[3];
	char MatIndex;
	char AuxMatIndex;
	unsigned SmoothingGroups;
};
struct VMainSets
{
	int LightningResolution;
	float UVDensity;
	int32 CoordinateIndex;
};
struct VMaterial
{
	char MaterialName[64];
	int TextureIndex;
	unsigned PolyFlags;
	int AuxMaterial;
	unsigned AuxFlags;
	int LodBias;
	int LodStyle;
};
struct FJointPos
{
	FQuat Orientation;
	FVector3f Position;
	float Length;
	float XSize;
	float YSize;
	float ZSize;
};
struct VNamedBoneBinary
{
	char Name[64];
	int Flags;
	int NumChildren;
	int ParentIndex;
	FJointPos BonePos;
};
struct VRawBoneInfluence
{
	float Weight;
	int PointIdx;
	int BoneIdx;
};
struct VUVEntry
{
	char Name[20];
	std::vector<FVector2f> UVData;
};

class PskReader
{
public:
	
	PskReader(FString filepath)
	{
		Ar.open(ToCStr(filepath), std::ios::binary);
	}
	void Read()
	{
		VChunkHeader Header;
	    Ar.read(reinterpret_cast<char*>(&Header), sizeof(VChunkHeader));


	    VChunkHeader Chunk;
	    while (!Ar.eof())
	    {
	        Ar.read(reinterpret_cast<char*>(&Chunk), sizeof(VChunkHeader));
	    	
	        if (CHUNKHANDLER("PNTS0000"))
	        {
	            Vertices.resize(Chunk.DataCount);
	            for (auto i = 0; i < Vertices.size(); i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Vertices[i]), sizeof(FVector3f));
	            }
	        }
	        else if (CHUNKHANDLER("VTXW0000"))
	        {
	            Wedges.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Wedges[i]), sizeof(VVertex));
	                if (Chunk.DataCount <= 65536)
	                {
	                    Wedges[i].PointIndex &= 0xFFFF; // Padded to Int
	                }
	            }
	        }
	        else if (CHUNKHANDLER("FACE0000"))
	        {
	            Faces.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex[0]), sizeof(short));
	                Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex[1]), sizeof(short));
	                Ar.read(reinterpret_cast<char*>(&Faces[i].WedgeIndex[2]), sizeof(short));
	                
	                Ar.read(&Faces[i].MatIndex, sizeof(char));
	                Ar.read(&Faces[i].AuxMatIndex, sizeof(char));
	                Ar.read(reinterpret_cast<char*>(&Faces[i].SmoothingGroups), sizeof(unsigned));
	            }
	        }
			else if (CHUNKHANDLER("SMS0000"))
			{
				Sets.resize(Chunk.DataCount);
				Ar.read(reinterpret_cast<char*>(&Sets[0].LightningResolution), sizeof(int));
				Ar.read(reinterpret_cast<char*>(&Sets[0].UVDensity), sizeof(float));
				Ar.read(reinterpret_cast<char*>(&Sets[0].CoordinateIndex), sizeof(int32));
			}
	        else if (CHUNKHANDLER("MATT0000"))
	        {
	            Materials.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Materials[i]), sizeof(VMaterial));
	            }
	        }
	        else if (CHUNKHANDLER("REFSKELT") || CHUNKHANDLER("REFSKEL0"))
	        {
	            Bones.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Bones[i].Name), sizeof(Bones[i].Name));
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].Flags), sizeof(int));
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].NumChildren), sizeof(int));
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].ParentIndex), sizeof(int));
	            	
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Orientation), sizeof(FQuat));
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Position), sizeof(FVector3f));
	            	
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.Length), sizeof(float));
					Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.XSize), sizeof(float));
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.YSize), sizeof(float));
	            	Ar.read(reinterpret_cast<char*>(&Bones[i].BonePos.ZSize), sizeof(float));
	            	
	            }
	        }
	        else if (CHUNKHANDLER("RAWWEIGHTS") || CHUNKHANDLER("RAWW0000"))
	        {
	            Weights.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Weights[i]), sizeof(VRawBoneInfluence));
	            }
	        }
	        else if (CHUNKHANDLER("VTXNORMS"))
	        {
	            Normals.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Normals[i]), sizeof(FVector3f));
	            }
	        }
	        else if (CHUNKHANDLER("VERTEXCOLOR"))
	        {
	            VertexColors.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&VertexColors[i]), sizeof(FColor));
	            }
	        }
	        else if (CHUNKHANDLER("EXTRAUVS"))
	        {
	            VUVEntry Entry;
	            strcpy_s(Entry.Name, Chunk.ChunkID);
	            Entry.UVData.resize(Chunk.DataCount);
	            for (auto i = 0; i < Chunk.DataCount; i++)
	            {
	                Ar.read(reinterpret_cast<char*>(&Entry.UVData[i]), sizeof(FVector2f));
	            }

	            ExtraUVs.push_back(Entry);
	        }
	        else
	        {
	            Ar.ignore(Chunk.DataSize*Chunk.DataCount); 
	        }
	    }
	}

	std::vector<FVector3f> Vertices;
	std::vector<VVertex> Wedges;
	std::vector<VMainSets> Sets;
	std::vector<VTriangle> Faces;
	std::vector<VMaterial> Materials;
	std::vector<VNamedBoneBinary> Bones;
	std::vector<VRawBoneInfluence> Weights;
	std::vector<FVector3f> Normals;
	std::vector<FColor> VertexColors;
	std::vector<VUVEntry> ExtraUVs;

private:
	std::ifstream Ar;
};