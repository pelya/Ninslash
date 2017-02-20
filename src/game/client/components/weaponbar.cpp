#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h>
#include <game/weapons.h>
#include <game/buildables.h>
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>


#include <game/client/components/sounds.h>
#include <game/client/components/scoreboard.h>
#include <game/client/components/picker.h>
#include <game/client/components/controls.h>
#include <game/client/components/chat.h>
#include "weaponbar.h"

const int CWeaponbar::WeaponOrder[] =
{
	WEAPON_TOOL,
	WEAPON_HAMMER,
	WEAPON_CHAINSAW,
	WEAPON_RIFLE,
	WEAPON_SHOTGUN,
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
	if(m_pClient->m_Snap.m_pGameInfoObj)
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

	char ChatText[1024] = "";
	static bool s_AndroidGetTextActive = false;

	if (m_Touching && m_Pos.x < Screen.w * 0.15f)
	{
		if (!m_ScoreboardShown)
			m_pClient->m_pScoreboard->Show(true); //Console()->ExecuteLine("+scoreboard");
		m_ScoreboardShown = true;
	}
	else if (m_Touching && m_Pos.x < Screen.w * 0.25f)
	{
		UI()->AndroidGetTextInput(ChatText, sizeof(ChatText));
		s_AndroidGetTextActive = true;
		m_CanDrop = false;
	}
	else
	{
		if (m_ScoreboardShown)
			m_pClient->m_pScoreboard->Show(false);  //Console()->ExecuteLine("-scoreboard");
		m_ScoreboardShown = false;
		TextRender()->TextColor(0.3f, 0.3f, 0.3f, 1);
		TextRender()->Text(0, Screen.w * 0.08f, Screen.h * 0.1f, 16, Localize("scores"), -1);
		TextRender()->Text(0, Screen.w * 0.23f, Screen.h * 0.1f, 16, Localize("chat"), -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}

	if (s_AndroidGetTextActive && UI()->AndroidGetTextInput(ChatText, sizeof(ChatText)))
	{
		s_AndroidGetTextActive = false;
		if(ChatText[0] == '!')
		{
			if(m_pClient->Client()->RconAuthed())
			{
				if(str_comp_num(ChatText, "!help", str_length("!help")) == 0)
				{
					CChat::ListOfRemoteCommands = (char *)mem_alloc(10, 1);
					strcpy(CChat::ListOfRemoteCommands, "");
					m_pClient->Console()->PossibleCommands(str_length(ChatText) > str_length("!help") + 1 ? ChatText + str_length("!help") + 1 : "",
															CFGFLAG_SERVER, true, CChat::PrintRemoteCommands, m_pClient->m_pChat);
					if (CChat::ListOfRemoteCommands[0])
						m_pClient->m_pChat->AddLine(-1, 0, CChat::ListOfRemoteCommands);
					mem_free(CChat::ListOfRemoteCommands);
				}
				else
				{
					m_pClient->Client()->Rcon(ChatText + 1);
				}
			}
			else
			{
				int idx = 1;
				if(str_comp_num(ChatText, "!password ", str_length("!password ")) == 0)
					idx = str_length("!password ");
				m_pClient->Client()->RconAuth("", ChatText + idx);
			}
		}
		else if (ChatText[0] != 0)
		{
			m_pClient->m_pChat->Say(0, ChatText);
		}
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

	if (CustomStuff()->m_LocalKits >= 1)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
		Graphics()->QuadsBegin();

		RenderTools()->SelectSprite(SPRITE_KIT_BARREL);
		RenderTools()->DrawSprite(Screen.w - Screen.h * 0.15f * 4 + 32, Screen.h * 0.2f - 5, 64);

		if (CustomStuff()->m_LocalKits >= 2)
		{
			RenderTools()->SelectSprite(SPRITE_KIT_TURRET);
			RenderTools()->DrawSprite(Screen.w - Screen.h * 0.15f * 2 + 32, Screen.h * 0.2f, 64);
			RenderTools()->SelectSprite(SPRITE_KIT_FLAMETRAP);
			RenderTools()->DrawSprite(Screen.w - Screen.h * 0.15f * 3 + 32 + 7, Screen.h * 0.2f + 1, 64);
		}

		Graphics()->QuadsEnd();
	}
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
	bar->m_pClient->m_pControls->m_SelectedBuilding = BUILDABLE_TURRET + 1;
	bar->BuildProcess(pResult->GetInteger(0));
}

void CWeaponbar::ConBuildFlamer(IConsole::IResult *pResult, void *pUserData)
{
	CWeaponbar * bar = (CWeaponbar *)pUserData;
	bar->m_pClient->m_pControls->m_SelectedBuilding = BUILDABLE_FLAMETRAP + 1;
	bar->BuildProcess(pResult->GetInteger(0));
}

void CWeaponbar::ConBuildBarrel(IConsole::IResult *pResult, void *pUserData)
{
	CWeaponbar * bar = (CWeaponbar *)pUserData;
	bar->m_pClient->m_pControls->m_SelectedBuilding = BUILDABLE_BARREL + 1;
	bar->BuildProcess(pResult->GetInteger(0));
}

void CWeaponbar::BuildProcess(int keypress)
{
	if (!m_pClient->BuildingEnabled())
		return;

	if (keypress)
	{
		if (!m_pClient->m_pControls->m_BuildMode)
		{
			m_pClient->m_pControls->m_BuildReleased = true;
			m_pClient->m_pControls->m_Build = 1;
		}
	}
	else if (m_pClient->m_pControls->m_BuildMode)
	{
		m_pClient->m_pControls->m_BuildReleased = true;
		m_pClient->m_pControls->m_Build = 1;

		CNetMsg_Cl_UseKit Msg;
		Msg.m_Kit = m_pClient->m_pControls->m_SelectedBuilding - 1;
		Msg.m_X = CustomStuff()->m_BuildPos.x;
		Msg.m_Y = CustomStuff()->m_BuildPos.y+18;
		Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
	}
}
