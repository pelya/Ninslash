#ifndef GAME_SERVER_MAPGEN_GEN_LAYER_H
#define GAME_SERVER_MAPGEN_GEN_LAYER_H

#include <base/vmath.h>

#define GEN_MAX 999

class CGenLayer
{
private:
	int *m_pTiles;
	int *m_pBGTiles;
	int *m_pFlags;
	int *m_pBGFlags;
	int m_Width;
	int m_Height;
	int m_Size;

	ivec2 m_aPlatform[GEN_MAX];
	int m_NumPlatforms;
	
	// inner top corners
	ivec2 m_aTopCorner[GEN_MAX];
	int m_NumTopCorners;
	
	// sharp corners
	ivec2 m_aCorner[GEN_MAX];
	int m_NumCorners;
	
	// acid pools
	ivec4 m_aPit[GEN_MAX];
	int m_NumPits;
	
	ivec2 m_aWall[GEN_MAX];
	int m_NumWalls;
	
	ivec2 m_aCeiling[GEN_MAX];
	int m_NumCeilings;
	
	ivec2 m_aPlayerSpawn[GEN_MAX];
	int m_NumPlayerSpawns;
	
public:
	CGenLayer(int w, int h);
	~CGenLayer();
	
	enum Layer
	{
		FOREGROUND,
		BACKGROUND
	};
	
	void Set(int Tile, int x, int y, int Flags = 0, int Layer = FOREGROUND);
	int GetByIndex(int Index, int Layer = FOREGROUND);
	int Get(int x, int y, int Layer = FOREGROUND);
	int GetFlags(int x, int y, int Layer = FOREGROUND);
	bool Used(int x, int y);
	
	void Use(int x, int y);
	
	int Width() { return m_Width; }
	int Height() { return m_Height; }
	
	int NumPlatforms() { return m_NumPlatforms; }
	int NumTopCorners() { return m_NumTopCorners; }
	
	void GenerateBackground();
	void GenerateAirPlatforms(int Num);
	void Scan();
	int Size();
	
	ivec2 GetPlayerSpawn();
	ivec2 GetPlatform();
	ivec2 GetCeiling();
	ivec2 GetWall();
	ivec2 GetTopCorner();
	ivec2 GetSharpCorner();
	ivec4 GetPit();
};

#endif
