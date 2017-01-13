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
#include "tutorial.h"

float CTutorial::m_Duration = 1.0f;

CTutorial::CTutorial()
{
	m_Active = true;
	m_Time = 0;
}

float CTutorial::Fade(float Seconds, float Middle)
{
	return (m_Duration - fabsf( Seconds - Middle )) / m_Duration;
}

void CTutorial::DrawTextFade(float Seconds, float Tick, float x, float y, const char *text)
{
	if ( Seconds > m_Duration * (Tick - 1) && Seconds < m_Duration * (Tick + 1) )
	{
		float Width = TextRender()->TextWidth(0, 25, text, -1);
		TextRender()->TextColor(0.7f, 0.7f, 0.7f, Fade(Seconds, m_Duration * Tick));
		TextRender()->Text(0, x - Width / 2, y, 25, text, -1);
	}
}

void CTutorial::OnRender()
{
	if(!m_pClient->m_Snap.m_pGameInfoObj || !m_pClient->m_Snap.m_pLocalCharacter || !m_Active)
	{
		return;
	}
	printf("CTutorial::OnRender()\n");

	int64 CurTime = time_get();
	
	if (m_Time == 0)
	{
		m_Time = CurTime;
	}

	float Seconds = (float) (CurTime - m_Time) / time_freq();

	CUIRect Screen = *UI()->Screen();

	Graphics()->MapScreen(Screen.x, Screen.y, Screen.w, Screen.h);

	Graphics()->BlendNormal();

	DrawTextFade(Seconds, 2, Screen.w * 0.25f, Screen.h * 0.6f, Localize("Swipe to move"));
	DrawTextFade(Seconds, 4, Screen.w * 0.25f, Screen.h * 0.6f, Localize("Swipe down to slide"));

	DrawTextFade(Seconds, 6, Screen.w * 0.75f, Screen.h * 0.6f, Localize("Swipe to attack"));
	DrawTextFade(Seconds, 8, Screen.w * 0.75f, Screen.h * 0.6f, Localize("Tap to jump"));

	DrawTextFade(Seconds, 10, Screen.w * 0.8f, Screen.h * 0.1f, Localize("Switch weapons"));

	DrawTextFade(Seconds, 12, Screen.w * 0.15f, Screen.h * 0.1f, Localize("Show score board"));
	DrawTextFade(Seconds, 14, Screen.w * 0.25f, Screen.h * 0.1f, Localize("Show chat window"));

	if (Seconds > 20)
	{
		m_Active = false;
	}

	TextRender()->TextColor(1, 1, 1, 1);
}
