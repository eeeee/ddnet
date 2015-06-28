/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SKINS_H
#define GAME_CLIENT_COMPONENTS_SKINS_H
#include <base/vmath.h>
#include <base/tl/sorted_array.h>
#include <game/client/component.h>

enum
{
	SKIN_STATE_NOT_LOADED,
	SKIN_STATE_LOADED,
	SKIN_STATE_ERROR,
};

class CSkins : public CComponent
{
public:
	// do this better and nicer
	struct CSkin
	{
		mutable int m_LastUsed;
		mutable int m_OrgTexture;
		mutable int m_ColorTexture;
		char m_aName[24];
		mutable vec3 m_BloodColor;
		mutable int m_State;

		bool operator<(const CSkin &Other) { return str_comp(m_aName, Other.m_aName) < 0; }
	};

	void OnInit();

	vec3 GetColorV3(int v);
	vec4 GetColorV4(int v);
	int Num();
	const CSkin *Get(int Index, bool CallUse = true);
	int Find(const char *pName);

	// Use marks the skin as being used (e.g. for rendering the current frame).
	// Component would try to load this skin if it's not currently loaded,
	// potentially phasing out another least recently used skin.
	void Use(const CSkin *pSkin);

private:
	array<CSkin> m_aSkins;
	int m_UsageCounter;

	static int SkinScan(const char *pName, int IsDir, int DirType, void *pUser);
	void LoadSkin(const CSkin &Skin);
	void UnloadSkin(CSkin &Skin);
};
#endif
