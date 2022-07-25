#pragma once
#include "PSKReader.h"

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
	FVector3f Position;
	FQuat4f Orientation;
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
