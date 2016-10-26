#include <engine/engine.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/shared/config.h>
#include <game/generated/protocol.h>
#include <game/generated/client_data.h>

#include <game/gamecore.h> // get_angle
#include <game/weapons.h> // get_angle
#include <game/client/ui.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>


#include <game/client/components/sounds.h>
#include "weaponbar.h"

CWeaponbar::CWeaponbar()
{
	OnReset();
}

void CWeaponbar::OnReset()
{
	m_Touching = false;
	m_Pos = vec2(0,0);
	m_InitialPos = vec2(0,0);
	m_LastPicked = -1;
	m_CanDrop = true;
}

void CWeaponbar::OnRelease()
{
	OnReset();
}

bool CWeaponbar::OnFingerTouch(vec2 pos)
{
	bool ret = false;
	if (!m_Touching)
		m_InitialPos = pos;
	m_Touching = true;
	m_Pos = pos;
	int weapon = ScreenToWeapon(pos.x, 1.0f);
	if (weapon >= 0 && weapon != m_LastPicked)
	{
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "weaponpick %d", weapon);
		Console()->ExecuteLine(aBuf);
		if (m_LastPicked > 0)
			m_CanDrop = false;
		m_LastPicked = weapon;
		ret = true;
	}
	return ret;
}

bool CWeaponbar::OnFingerRelease()
{
	bool ret = false;
	if (m_CanDrop && m_LastPicked > 0 && absolute(m_InitialPos.y - m_Pos.y) > 0.6f)
	{
		Console()->ExecuteLine("+dropweapon");
		ret = true;
	}
	m_Touching = false;
	m_LastPicked = -1;
	m_CanDrop = true;
	return ret;
}

int CWeaponbar::ScreenToWeapon(float pos, float width) const
{
	pos = width - pos;
	for (int i = 0, counter = 0; i < NUM_WEAPONS-1; i++)
	{
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<(i+1))))
			continue;
		counter++;
		float x = width * 0.55f * (NUM_WEAPONS - 2 - counter) / (NUM_WEAPONS - 2);
		x += width * 0.45f;
		x -= width / (NUM_WEAPONS - 2) / 4;
		if (x > pos)
			return i;
	}
	return -1;
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

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);

	int selected = m_pClient->m_Snap.m_pLocalCharacter ? m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS - 1 : -1;
	int counter = 0;
	for (int i = 0; i < NUM_WEAPONS-1; i++)
	{
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<(i+1))))
			continue;

		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i+1].m_pSpriteBody);

		float size = 0.6f;

		vec2 pos = vec2(Screen.w * 0.55f * (NUM_WEAPONS - 2 - counter) / (NUM_WEAPONS - 2), Screen.h * 0.05f);
		pos.x += Screen.w * 0.45f;
		pos.x -= Screen.w / (NUM_WEAPONS - 2) / 4;
		if (selected == i)
		{
			pos.y *= 1.3f;
			size = 0.7f;
		}

		RenderTools()->DrawSprite(pos.x, pos.y, g_pData->m_Weapons.m_aId[i+1].m_VisualSize * size);

		counter++;
	}

	Graphics()->QuadsEnd();

	// TODO: mines, build tools
}
