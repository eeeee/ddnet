/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <math.h>

#include <base/system.h>
#include <base/math.h>

#include <engine/graphics.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include "skins.h"

const char* vanillaSkins[] = {"bluekitty.png", "bluestripe.png", "brownbear.png",
	"cammo.png", "cammostripes.png", "coala.png", "default.png", "limekitty.png",
	"pinky.png", "redbopp.png", "redstripe.png", "saddo.png", "toptri.png",
	"twinbop.png", "twintri.png", "warpaint.png", "x_ninja.png"};

void CSkins::UnloadSkin(CSkin &Skin)
{
	if(g_Config.m_Debug)
	{
		char aBuf[512];
		str_format(aBuf, sizeof(aBuf), "unloading skin %s", Skin.m_aName);
		Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	Graphics()->UnloadTexture(Skin.m_OrgTexture);
	Graphics()->UnloadTexture(Skin.m_ColorTexture);
	Skin.m_OrgTexture = m_aSkins[0].m_OrgTexture;
	Skin.m_ColorTexture = m_aSkins[0].m_ColorTexture;
	Skin.m_BloodColor = m_aSkins[0].m_BloodColor;
	Skin.m_State = SKIN_STATE_NOT_LOADED;
}

void CSkins::LoadSkin(const CSkin &Skin)
{
	CSkins *pSelf = this;

	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), "skins/%s.png", Skin.m_aName);
	CImageInfo Info;
	if(!pSelf->Graphics()->LoadPNG(&Info, aBuf, IStorage::TYPE_ALL))
	{
		str_format(aBuf, sizeof(aBuf), "failed to load skin from %s.png", Skin.m_aName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
		Skin.m_State = SKIN_STATE_ERROR;
		int def = Find("default");
		if (def > -1)
		{
			Skin.m_OrgTexture = m_aSkins[def].m_OrgTexture;
			Skin.m_ColorTexture = m_aSkins[def].m_ColorTexture;
			Skin.m_BloodColor = m_aSkins[def].m_BloodColor;
		}
		return;
	}

	Skin.m_OrgTexture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);

	int BodySize = 96; // body size
	unsigned char *d = (unsigned char *)Info.m_pData;
	int Pitch = Info.m_Width*4;

	// dig out blood color
	{
		int aColors[3] = {0};
		for(int y = 0; y < BodySize; y++)
			for(int x = 0; x < BodySize; x++)
			{
				if(d[y*Pitch+x*4+3] > 128)
				{
					aColors[0] += d[y*Pitch+x*4+0];
					aColors[1] += d[y*Pitch+x*4+1];
					aColors[2] += d[y*Pitch+x*4+2];
				}
			}

		Skin.m_BloodColor = normalize(vec3(aColors[0], aColors[1], aColors[2]));
	}

	// create colorless version
	int Step = Info.m_Format == CImageInfo::FORMAT_RGBA ? 4 : 3;

	// make the texture gray scale
	for(int i = 0; i < Info.m_Width*Info.m_Height; i++)
	{
		int v = (d[i*Step]+d[i*Step+1]+d[i*Step+2])/3;
		d[i*Step] = v;
		d[i*Step+1] = v;
		d[i*Step+2] = v;
	}


	int Freq[256] = {0};
	int OrgWeight = 0;
	int NewWeight = 192;

	// find most common frequence
	for(int y = 0; y < BodySize; y++)
		for(int x = 0; x < BodySize; x++)
		{
			if(d[y*Pitch+x*4+3] > 128)
				Freq[d[y*Pitch+x*4]]++;
		}

	for(int i = 1; i < 256; i++)
	{
		if(Freq[OrgWeight] < Freq[i])
			OrgWeight = i;
	}

	// reorder
	int InvOrgWeight = 255-OrgWeight;
	int InvNewWeight = 255-NewWeight;
	for(int y = 0; y < BodySize; y++)
		for(int x = 0; x < BodySize; x++)
		{
			int v = d[y*Pitch+x*4];
			if(v <= OrgWeight)
				v = (int)(((v/(float)OrgWeight) * NewWeight));
			else
				v = (int)(((v-OrgWeight)/(float)InvOrgWeight)*InvNewWeight + NewWeight);
			d[y*Pitch+x*4] = v;
			d[y*Pitch+x*4+1] = v;
			d[y*Pitch+x*4+2] = v;
		}

	Skin.m_ColorTexture = pSelf->Graphics()->LoadTextureRaw(Info.m_Width, Info.m_Height, Info.m_Format, Info.m_pData, Info.m_Format, 0);
	mem_free(Info.m_pData);

	// set skin data
	if(g_Config.m_Debug)
	{
		str_format(aBuf, sizeof(aBuf), "load skin %s", Skin.m_aName);
		pSelf->Console()->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "game", aBuf);
	}
	Skin.m_State = SKIN_STATE_LOADED;
}

int CSkins::SkinScan(const char *pName, int IsDir, int DirType, void *pUser)
{
	bool vanilla = false;
	for(unsigned int i = 0; i < sizeof(vanillaSkins) / sizeof(vanillaSkins[0]); i++)
	{
		if(str_comp(pName, vanillaSkins[i]) == 0)
		{
			vanilla = true;
			break;
		}
	}
	if(!g_Config.m_ClShowNewSkins && !vanilla)
		return 0;

	CSkins *pSelf = (CSkins *)pUser;

	int l = str_length(pName);
	if(l < 4 || IsDir || str_comp(pName+l-4, ".png") != 0)
		return 0;

	// Don't add duplicate skins (one from user's config directory, other from
	// client itself)
	for(int i = 0; i < pSelf->Num(); i++)
	{
		const char* pExName = pSelf->m_aSkins[i].m_aName;
		if(str_comp_num(pExName, pName, l-4) == 0 && str_length(pExName) == l-4)
			return 0;
	}

	CSkin Skin;
	str_copy(Skin.m_aName, pName, min((int)sizeof(Skin.m_aName),l-3));
	Skin.m_LastUsed = 0;
	Skin.m_OrgTexture = -1;
	Skin.m_ColorTexture = -1;
	Skin.m_BloodColor = vec3(1.0f, 1.0f, 1.0f);
	Skin.m_State = SKIN_STATE_NOT_LOADED;
	if (vanilla)
		pSelf->LoadSkin(Skin);
	pSelf->m_aSkins.add(Skin);

	return 0;
}


void CSkins::OnInit()
{
	// load skins
	m_aSkins.clear();
	m_UsageCounter = 0;
	Storage()->ListDirectory(IStorage::TYPE_ALL, "skins", SkinScan, this);
}

int CSkins::Num()
{
	return m_aSkins.size();
}

void CSkins::Use(const CSkin *pSkin) {
	pSkin->m_LastUsed = m_UsageCounter++;
	if (pSkin->m_State != SKIN_STATE_NOT_LOADED)
		return;
	int skinsLoaded;
	do
	{
		// check if we must unload some skins first to comply with cl_skins_cache_size
		int minLastUsed = m_UsageCounter;
		int minLastUsedIdx = -1;
		skinsLoaded = 0;
		for (int i = 0; i < Num(); i++)
		{
			if (m_aSkins[i].m_State == SKIN_STATE_LOADED)
			{
				skinsLoaded++;
				if (m_aSkins[i].m_LastUsed < minLastUsed && str_comp(m_aSkins[i].m_aName, "default") != 0)
				{
					minLastUsed = m_aSkins[i].m_LastUsed;
					minLastUsedIdx = i;
				}
			}
		}
		if (minLastUsedIdx == -1 || skinsLoaded <= g_Config.m_ClSkinsCacheSize)
			break;
		UnloadSkin(m_aSkins[minLastUsedIdx]);
	}
	while (true);
	LoadSkin(*pSkin);
}

const CSkins::CSkin *CSkins::Get(int Index, bool CallUse)
{
	CSkin *pSkin = &m_aSkins[max(0, Index%m_aSkins.size())];
	if (CallUse)
		Use(pSkin);
	return pSkin;
}

int CSkins::Find(const char *pName)
{
	for(int i = 0; i < m_aSkins.size(); i++)
	{
		if(str_comp(m_aSkins[i].m_aName, pName) == 0)
			return i;
	}
	return -1;
}

vec3 CSkins::GetColorV3(int v)
{
	return HslToRgb(vec3(((v>>16)&0xff)/255.0f, ((v>>8)&0xff)/255.0f, 0.5f+(v&0xff)/255.0f*0.5f));
}

vec4 CSkins::GetColorV4(int v)
{
	vec3 r = GetColorV3(v);
	return vec4(r.r, r.g, r.b, 1.0f);
}
