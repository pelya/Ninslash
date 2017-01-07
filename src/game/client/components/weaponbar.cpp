#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h>
#include <game/weapons.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>


#include <game/client/components/sounds.h>
#include <game/client/components/scoreboard.h>
#include <game/client/components/picker.h>
#include "weaponbar.h"

/*
const int CWeaponbar::WeaponOrder[] =
{
	0, //WEAPON_TOOL=0,
	1, //WEAPON_HAMMER,
	3, //WEAPON_SHOTGUN,
	4, //WEAPON_RIFLE,
	5, //WEAPON_LASER,
	6, //WEAPON_ELECTRIC,
	7, //WEAPON_GRENADE,
	8, //WEAPON_FLAMER,
	2, //WEAPON_CHAINSAW,
};
*/

const int CWeaponbar::WeaponOrder[] =
{
	WEAPON_TOOL,
	WEAPON_HAMMER,
	WEAPON_CHAINSAW,
	WEAPON_SHOTGUN,
	WEAPON_RIFLE,
	WEAPON_LASER,
	WEAPON_ELECTRIC,
	WEAPON_GRENADE,
	WEAPON_FLAMER,
};

CWeaponbar::CWeaponbar()
{
	OnReset();
	dbg_assert(sizeof(WeaponOrder) / sizeof(WeaponOrder[0]) == NUM_WEAPONS, "WeaponOrder array misses some weapons");
}

void CWeaponbar::OnReset()
{
	m_Touching = false;
	m_Pos = vec2(0,0);
	m_InitialPos = vec2(0,0);
	m_CanDrop = true;
	m_LastPicked = -1;
	m_ScoreboardShown = false;
	m_BlockTouchEvents = false;
}

void CWeaponbar::OnRelease()
{
	OnReset();
}

void CWeaponbar::OnFingerTouch(vec2 posNormalized)
{
	CUIRect Screen = *UI()->Screen();
	posNormalized.x *= Screen.w;
	if (!m_Touching)
		m_InitialPos = posNormalized;
	m_Touching = true;
	m_Pos = posNormalized;
}

void CWeaponbar::OnFingerRelease()
{
	// Drop by sliding weapon bar up or down, disabled since we have a separate button
	//if (m_CanDrop && absolute(m_InitialPos.y - m_Pos.y) > 0.4f)
	//	Console()->ExecuteLine("+dropweapon");
	m_Touching = false;
	m_CanDrop = true;
	m_LastPicked = -1;
}

void CWeaponbar::OnRender()
{
	if(m_pClient->m_Snap.m_SpecInfo.m_Active)
	{
		m_Touching = false;
		return;
	}

	CUIRect Screen = *UI()->Screen();

	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();

	// Draw drop button
	TextRender()->TextColor(0.3f, 0.3f, 0.3f, 1);
	TextRender()->Text(0, Screen.w * 0.94f, Screen.h * 0.023f, 20, Localize("drop"), -1);
	TextRender()->TextColor(1, 1, 1, 1);

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);

	// Draw weapons
	int selected = m_pClient->m_Snap.m_pLocalCharacter ? m_pClient->m_Snap.m_pLocalCharacter->m_Weapon % NUM_WEAPONS : -1;
	int counter = 0;
	bool changed = false;
	for (int ii = WEAPON_HAMMER, i = WeaponOrder[ii]; ii < NUM_WEAPONS; ii++, i = WeaponOrder[ii])
	{
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<i)))
			continue;

		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);

		float size = 0.6f;

		vec2 pos = vec2(Screen.w * 0.50f * (NUM_WEAPONS - 2 - counter) / (NUM_WEAPONS - 2), Screen.h * 0.05f);
		pos.x += Screen.w * 0.50f;
		pos.x -= Screen.w / (NUM_WEAPONS - 2) * 0.7f;
		if (selected == i)
		{
			pos.y *= 1.3f;
			size = 0.7f;
		}
		if (m_Touching && counter == 0 && m_Pos.x > pos.x + Screen.w / (NUM_WEAPONS - 2) * 0.22f)
		{
			if (!changed && m_CanDrop)
			{
				Console()->ExecuteLine("+dropweapon");
			}
			m_CanDrop = false;
			changed = true;
		}
		if (m_Touching && m_Pos.x > pos.x - Screen.w / (NUM_WEAPONS - 2) / 4 && !changed)
		{
			changed = true;
			m_CanDrop = false;
			if (selected != i && m_LastPicked != i)
			{
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "weaponpick %d", i - 1);
				Console()->ExecuteLine(aBuf);
				m_LastPicked = i;
			}
		}

		RenderTools()->DrawSprite(pos.x, pos.y, g_pData->m_Weapons.m_aId[i].m_VisualSize * size);

		counter++;
	}

	Graphics()->QuadsEnd();

	if (m_Touching && m_Pos.x < Screen.w * 0.25f)
	{
		if (!m_ScoreboardShown)
			m_pClient->m_pScoreboard->Show(true); //Console()->ExecuteLine("+scoreboard");
		m_ScoreboardShown = true;
	}
	else
	{
		if (m_ScoreboardShown)
			m_pClient->m_pScoreboard->Show(false);  //Console()->ExecuteLine("-scoreboard");
		m_ScoreboardShown = false;
		TextRender()->TextColor(0.3f, 0.3f, 0.3f, 1);
		TextRender()->Text(0, Screen.w * 0.05f, Screen.h * 0.1f, 16, Localize("scores"), -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}

	// Draw mines
	if (CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE] > 0 || CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE] > 0)
	{
		int Item = PLAYERITEM_LANDMINE;
		if (CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE] > CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE])
			Item = PLAYERITEM_ELECTROMINE;

		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1,1,1,1);

		RenderTools()->SelectSprite(SPRITE_ITEM1 + Item);
		RenderTools()->DrawSprite(Screen.w - 64 + 16, Screen.h * 0.2f, 64);
		Graphics()->QuadsEnd();
		
		char aBuf[8];
		str_format(aBuf, sizeof(aBuf), "%d", clamp(CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE] + CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE], 0, 9));
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, Screen.w - 64 + 16 + 20, Screen.h * 0.2f + 12, 16, aBuf, -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}

	// TODO: build tools
}

void CWeaponbar::OnConsoleInit()
{
	Console()->Register("dropmine", "", CFGFLAG_CLIENT, ConDropMine, this, "Drop mine");
	Console()->Register("+buildturret", "", CFGFLAG_CLIENT, ConBuildTurret, this, "Build turret");
	Console()->Register("+buildflamer", "", CFGFLAG_CLIENT, ConBuildFlamer, this, "Build flamer");
	Console()->Register("+buildbarrel", "", CFGFLAG_CLIENT, ConBuildBarrel, this, "Build barrel");
}

void CWeaponbar::ConDropMine(IConsole::IResult *pResult, void *pUserData)
{
	CWeaponbar * bar = (CWeaponbar *)pUserData;
	if (bar->CustomStuff()->m_aLocalItems[PLAYERITEM_ELECTROMINE] > bar->CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE])
	{
		bar->m_pClient->Picker()->Itempick(PLAYERITEM_ELECTROMINE);
	}
	else if (bar->CustomStuff()->m_aLocalItems[PLAYERITEM_LANDMINE] > 0)
	{
		bar->m_pClient->Picker()->Itempick(PLAYERITEM_LANDMINE);
	}
}

void CWeaponbar::ConBuildTurret(IConsole::IResult *pResult, void *pUserData)
{
	CWeaponbar * bar = (CWeaponbar *)pUserData;
}

void CWeaponbar::ConBuildFlamer(IConsole::IResult *pResult, void *pUserData)
{
	CWeaponbar * bar = (CWeaponbar *)pUserData;
}

void CWeaponbar::ConBuildBarrel(IConsole::IResult *pResult, void *pUserData)
{
	CWeaponbar * bar = (CWeaponbar *)pUserData;
}
