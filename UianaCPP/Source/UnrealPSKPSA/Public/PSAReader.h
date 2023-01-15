#pragma once
#include "PSKReader.h"
#if ENGINE_MAJOR_VERSION == 4
#define FVector3f FVector
#define FQuat4f FQuat
#endif

struct VAnimInfoBinary
{
	char Name[64];
	char Group[64];
	int TotalBones;
	int RootInclude;
	int KeyCompressionStyle;
	int KeyQuotum;
	float KeyReduction;
	float TrackTime;
	float AnimRate;
	int StartBone;
	int FirstRawFrame;
	int NumRawFrames;
};

struct VQuatAnimKey
{
	FQuat4f Orientation;
	FVector3f Position;
	float Time;
};


struct VAnimScaleKey
{
	FVector3f ScaleVector;
	float Time;
};

class PSAReader
{
public:
	PSAReader(const FString Filename);
	bool Read();

	// Switches
	bool bHasScaleKeys;
	
	// PSA
	VAnimInfoBinary AnimInfo;
	TArray<VNamedBoneBinary> Bones;
	TArray<VQuatAnimKey> AnimKeys;
	TArray<VAnimScaleKey> ScaleKeys;

private:
	bool CheckHeader(const VChunkHeader Header) const;
	const char* HeaderBytes = "ANIMHEAD" + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00 + 0x00;
	std::ifstream Ar;
	
};
