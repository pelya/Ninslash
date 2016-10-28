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
	m_CanDrop = true;
	m_LastPicked = -1;
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
	if (m_CanDrop && absolute(m_InitialPos.y - m_Pos.y) > 0.4f)
		Console()->ExecuteLine("+dropweapon");
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

	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1,1,1,1);

	int selected = m_pClient->m_Snap.m_pLocalCharacter ? m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS - 1 : -1;
	int counter = 0;
	bool changed = false;
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
		if (m_Touching && m_Pos.x > pos.x - Screen.w / (NUM_WEAPONS - 2) / 4 && !changed)
		{
			changed = true;
			if (selected != i && m_LastPicked != i)
			{
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "weaponpick %d", i);
				Console()->ExecuteLine(aBuf);
				m_CanDrop = false;
				m_LastPicked = i;
			}
		}

		RenderTools()->DrawSprite(pos.x, pos.y, g_pData->m_Weapons.m_aId[i+1].m_VisualSize * size);

		counter++;
	}

	Graphics()->QuadsEnd();

	// TODO: mines, build tools
}
