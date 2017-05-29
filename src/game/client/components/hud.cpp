#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/storage.h>
#include <engine/shared/config.h>

#include <game/generated/protocol.h>
#include <game/generated/client_data.h>
#include <game/layers.h>
#include <game/client/gameclient.h>
#include <game/client/render.h>
#include <game/client/customstuff.h>

#include <game/weapons.h>
#include <game/buildables.h>

#include "controls.h"
#include "camera.h"
#include "hud.h"
#include "voting.h"
#include "binds.h"
#include "scoreboard.h"

#if defined(__ANDROID__)
#include <SDL_joystick.h>
#endif

#define RAD 0.017453292519943295769236907684886f

CHud::CHud()
{
	// won't work if zero
	m_AverageFPS = 1.0f;
}

int gs_DpadTexture = -1;
int gs_JumpButtonTexture = -1;
int gs_FireButtonTexture = -1;

void CHud::OnInit()
{
#if defined(__ANDROID__)
	if (gs_DpadTexture == -1)
		gs_DpadTexture = Graphics()->LoadTexture("dpad_button.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	if (gs_JumpButtonTexture == -1)
		gs_JumpButtonTexture = Graphics()->LoadTexture("jump_button.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
	if (gs_FireButtonTexture == -1)
		gs_FireButtonTexture = Graphics()->LoadTexture("fire_button.png", IStorage::TYPE_ALL, CImageInfo::FORMAT_AUTO, 0);
#endif
}

void CHud::OnReset()
{
}

void CScoreboard::RenderGameTimer()
{
	int Width = 300.0f*Graphics()->ScreenAspect();
	int Height = 300.0f;
	Graphics()->MapScreen(0.0f, 0.0f, Width, Height);

	float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;

	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH))
	{
		char Buf[32];
		int Time = 0;
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			Time = m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit*60 - ((Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed());

			if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER)
				Time = 0;
		}
		else
			Time = (Client()->GameTick()-m_pClient->m_Snap.m_pGameInfoObj->m_RoundStartTick)/Client()->GameTickSpeed();

		str_format(Buf, sizeof(Buf), "%d:%02d", Time/60, Time%60);
		float FontSize = 10.0f;
		float w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		// last 60 sec red, last 10 sec blink
		if(m_pClient->m_Snap.m_pGameInfoObj->m_TimeLimit && Time <= 60 && !m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
		{
			float Alpha = Time <= 10 && (2*time_get()/time_freq()) % 2 ? 0.5f : 1.0f;
			TextRender()->TextColor(1.0f, 0.25f, 0.25f, Alpha);
		}
		TextRender()->Text(0, Half-w/2, 2, FontSize, Buf, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	
	// survival mode text
	if (m_pClient->Survival())
	{
		TextRender()->TextColor(1.0f, 1.0f, 0.0f, 1.0f);
		const char *pText = Localize("Survival mode");
		float FontSize = 7.0f;
		float w = TextRender()->TextWidth(0, FontSize,pText, -1);
		TextRender()->Text(0, 150.0f*Graphics()->ScreenAspect()+-w/2.0f, 12.0f, FontSize, pText, -1);
		TextRender()->TextColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void CHud::RenderPauseNotification()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED &&
		!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		const char *pText = Localize("Game paused");
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize,pText, -1);
		TextRender()->Text(0, 150.0f*Graphics()->ScreenAspect()+-w/2.0f, 50.0f, FontSize, pText, -1);
	}
}

void CHud::RenderSuddenDeath()
{
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_SUDDENDEATH)
	{
		float Half = 300.0f*Graphics()->ScreenAspect()/2.0f;
		const char *pText = Localize("Sudden Death");
		float FontSize = 12.0f;
		float w = TextRender()->TextWidth(0, FontSize, pText, -1);
		TextRender()->Text(0, Half-w/2, 2, FontSize, pText, -1);
	}
}

void CHud::RenderScoreHud()
{
	// render small score hud
	if(!(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
	{
		int GameFlags = m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags;
		float Whole = 300*Graphics()->ScreenAspect();
		float StartY = 229.0f;

		if (GameFlags&GAMEFLAG_TEAMS && !(GameFlags&GAMEFLAG_INFECTION) && m_pClient->m_Snap.m_pGameDataObj)
		{
			char aScoreTeam[2][32];
			str_format(aScoreTeam[TEAM_RED], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreRed);
			str_format(aScoreTeam[TEAM_BLUE], sizeof(aScoreTeam)/2, "%d", m_pClient->m_Snap.m_pGameDataObj->m_TeamscoreBlue);
			float aScoreTeamWidth[2] = { TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_RED], -1), TextRender()->TextWidth(0, 14.0f, aScoreTeam[TEAM_BLUE], -1) };
			int FlagCarrier[2] = { m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierRed, m_pClient->m_Snap.m_pGameDataObj->m_FlagCarrierBlue };
			float ScoreWidthMax = max(max(aScoreTeamWidth[TEAM_RED], aScoreTeamWidth[TEAM_BLUE]), TextRender()->TextWidth(0, 14.0f, "100", -1));
			float Split = 3.0f;
			float ImageSize = GameFlags&GAMEFLAG_FLAGS ? 16.0f : Split;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				
				if (GameFlags&GAMEFLAG_INFECTION)
				{
					if(t == 0)
						Graphics()->SetColor(1.0f, 0.7f, 0.7f, 0.3f);
					else
						Graphics()->SetColor(0.1f, 0.1f, 0.1f, 0.3f);
				}
				else
				{
					if(t == 0)
						Graphics()->SetColor(1.0f, 0.0f, 0.0f, 0.25f);
					else
						Graphics()->SetColor(0.0f, 0.0f, 1.0f, 0.25f);
				}
				
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split, StartY+t*20, ScoreWidthMax+ImageSize+2*Split, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreTeamWidth[t])/2-Split, StartY+t*20, 14.0f, aScoreTeam[t], -1);

				if(GameFlags&GAMEFLAG_FLAGS)
				{
					int BlinkTimer = (m_pClient->m_FlagDropTick[t] != 0 &&
										(Client()->GameTick()-m_pClient->m_FlagDropTick[t])/Client()->GameTickSpeed() >= 25) ? 10 : 20;
					if(FlagCarrier[t] == FLAG_ATSTAND || (FlagCarrier[t] == FLAG_TAKEN && ((Client()->GameTick()/BlinkTimer)&1)))
					{
						// draw flag
						Graphics()->BlendNormal();
						Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
						Graphics()->QuadsBegin();
						RenderTools()->SelectSprite(t==0?SPRITE_FLAG_RED:SPRITE_FLAG_BLUE);
						IGraphics::CQuadItem QuadItem(Whole-ScoreWidthMax-ImageSize, StartY+1.0f+t*20, ImageSize/2, ImageSize);
						Graphics()->QuadsDrawTL(&QuadItem, 1);
						Graphics()->QuadsEnd();
					}
					else if(FlagCarrier[t] >= 0)
					{
						// draw name of the flag holder
						int ID = FlagCarrier[t]%MAX_CLIENTS;
						const char *pName = m_pClient->m_aClients[ID].m_aName;
						float w = TextRender()->TextWidth(0, 8.0f, pName, -1);
						TextRender()->Text(0, min(Whole-w-1.0f, Whole-ScoreWidthMax-ImageSize-2*Split), StartY+(t+1)*20.0f-3.0f, 8.0f, pName, -1);

						// draw tee of the flag holder
						CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
						Info.m_Size = 18.0f;
						//RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
						//	vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20));
					}
				}
				StartY += 8.0f;
			}
		}
		// dm, infection, co-op
		else
		{
			int Local = -1;
			int aPos[2] = { 1, 2 };
			const CNetObj_PlayerInfo *apPlayerInfo[2] = { 0, 0 };
			int i = 0;
			for(int t = 0; t < 2 && i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
			{
				if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
				{
					//if (!CustomStuff()->IsBot(m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID) || !(GameFlags&GAMEFLAG_COOP))
					if (!m_pClient->m_aClients[m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID].m_IsBot || !(GameFlags&GAMEFLAG_COOP))
					{
						apPlayerInfo[t] = m_pClient->m_Snap.m_paInfoByScore[i];
						if(apPlayerInfo[t]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
							Local = t;
						++t;
					}
				}
			}
			// search local player info if not a spectator, nor within top2 scores
			if(Local == -1 && m_pClient->m_Snap.m_pLocalInfo && m_pClient->m_Snap.m_pLocalInfo->m_Team != TEAM_SPECTATORS)
			{
				for(; i < MAX_CLIENTS && m_pClient->m_Snap.m_paInfoByScore[i]; ++i)
				{
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_Team != TEAM_SPECTATORS)
						++aPos[1];
					if(m_pClient->m_Snap.m_paInfoByScore[i]->m_ClientID == m_pClient->m_Snap.m_LocalClientID)
					{
						apPlayerInfo[1] = m_pClient->m_Snap.m_paInfoByScore[i];
						Local = 1;
						break;
					}
				}
			}
			char aScore[2][32];
			for(int t = 0; t < 2; ++t)
			{
				if(apPlayerInfo[t])
					str_format(aScore[t], sizeof(aScore)/2, "%d", apPlayerInfo[t]->m_Score);
				else
					aScore[t][0] = 0;
			}
			float aScoreWidth[2] = {TextRender()->TextWidth(0, 14.0f, aScore[0], -1), TextRender()->TextWidth(0, 14.0f, aScore[1], -1)};
			float ScoreWidthMax = max(max(aScoreWidth[0], aScoreWidth[1]), TextRender()->TextWidth(0, 14.0f, "10", -1));
			float Split = 3.0f, ImageSize = 16.0f, PosSize = 16.0f;

			for(int t = 0; t < 2; t++)
			{
				// draw box
				Graphics()->BlendNormal();
				Graphics()->TextureSet(-1);
				Graphics()->QuadsBegin();
				if(t == Local)
					Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.25f);
				else
					Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.25f);
				RenderTools()->DrawRoundRectExt(Whole-ScoreWidthMax-ImageSize-2*Split-PosSize, StartY+t*20, ScoreWidthMax+ImageSize+2*Split+PosSize, 18.0f, 5.0f, CUI::CORNER_L);
				Graphics()->QuadsEnd();

				// draw score
				TextRender()->Text(0, Whole-ScoreWidthMax+(ScoreWidthMax-aScoreWidth[t])/2-Split, StartY+t*20, 14.0f, aScore[t], -1);

				if(apPlayerInfo[t])
 				{
					// draw name
					int ID = apPlayerInfo[t]->m_ClientID;
					const char *pName = m_pClient->m_aClients[ID].m_aName;
					float w = TextRender()->TextWidth(0, 8.0f, pName, -1);
					TextRender()->Text(0, min(Whole-w-1.0f, Whole-ScoreWidthMax-ImageSize-2*Split-PosSize), StartY+(t+1)*20.0f-3.0f, 8.0f, pName, -1);

					// draw tee
					CTeeRenderInfo Info = m_pClient->m_aClients[ID].m_RenderInfo;
 					Info.m_Size = 18.0f;
 					//RenderTools()->RenderTee(CAnimState::GetIdle(), &Info, EMOTE_NORMAL, vec2(1,0),
					RenderTools()->RenderPortrait(&Info,
						vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20 + 16), 0);
						
 					//	vec2(Whole-ScoreWidthMax-Info.m_Size/2-Split, StartY+1.0f+Info.m_Size/2+t*20));
					
				}

				// draw position
				char aBuf[32];
				str_format(aBuf, sizeof(aBuf), "%d.", aPos[t]);
				TextRender()->Text(0, Whole-ScoreWidthMax-ImageSize-Split-PosSize, StartY+2.0f+t*20, 10.0f, aBuf, -1);

				StartY += 8.0f;
			}
		}
	}
}

void CHud::RenderWarmupTimer()
{
	// render warmup timer
	if(m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer)
	{
		char Buf[256];
		float FontSize = 20.0f;
		float w = TextRender()->TextWidth(0, FontSize, Localize("Warmup"), -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 50, FontSize, Localize("Warmup"), -1);

		int Seconds = m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer/SERVER_TICK_SPEED;
		if(Seconds < 5)
			str_format(Buf, sizeof(Buf), "%d.%d", Seconds, (m_pClient->m_Snap.m_pGameInfoObj->m_WarmupTimer*10/SERVER_TICK_SPEED)%10);
		else
			str_format(Buf, sizeof(Buf), "%d", Seconds);
		w = TextRender()->TextWidth(0, FontSize, Buf, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()+-w/2, 75, FontSize, Buf, -1);
	}
}

void CHud::MapscreenToGroup(float CenterX, float CenterY, CMapItemGroup *pGroup)
{
	float Points[4];
	RenderTools()->MapscreenToWorld(CenterX, CenterY, pGroup->m_ParallaxX/100.0f, pGroup->m_ParallaxY/100.0f,
		pGroup->m_OffsetX, pGroup->m_OffsetY, Graphics()->ScreenAspect(), 1.0f, Points);
	Graphics()->MapScreen(Points[0], Points[1], Points[2], Points[3]);
}

void CHud::RenderFps()
{
	if(g_Config.m_ClShowfps)
	{
		// calculate avg. fps
		float FPS = 1.0f / Client()->RenderFrameTime();
		m_AverageFPS = (m_AverageFPS*(1.0f-(1.0f/m_AverageFPS))) + (FPS*(1.0f/m_AverageFPS));
		char Buf[512];
		str_format(Buf, sizeof(Buf), "%d", (int)m_AverageFPS);
		TextRender()->Text(0, m_Width-10-TextRender()->TextWidth(0,12,Buf,-1), 5, 12, Buf, -1);
	}
}

void CHud::RenderConnectionWarning()
{
	if(Client()->ConnectionProblems())
	{
		const char *pText = Localize("Connection Problems...");
		float w = TextRender()->TextWidth(0, 24, pText, -1);
		TextRender()->Text(0, 150*Graphics()->ScreenAspect()-w/2, 50, 24, pText, -1);
	}
}

void CHud::RenderTeambalanceWarning()
{
	if (m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_INFECTION)
		return;
	
	// render prompt about team-balance
	bool Flash = time_get()/(time_freq()/2)%2 == 0;
	if(m_pClient->m_Snap.m_pGameInfoObj->m_GameFlags&GAMEFLAG_TEAMS)
	{
		int TeamDiff = m_pClient->m_Snap.m_aTeamSize[TEAM_RED]-m_pClient->m_Snap.m_aTeamSize[TEAM_BLUE];
		if (g_Config.m_ClWarningTeambalance && (TeamDiff >= 2 || TeamDiff <= -2))
		{
			const char *pText = Localize("Please balance teams!");
			if(Flash)
				TextRender()->TextColor(1,1,0.5f,1);
			else
				TextRender()->TextColor(0.7f,0.7f,0.2f,1.0f);
			TextRender()->Text(0x0, 5, 50, 6, pText, -1);
			TextRender()->TextColor(1,1,1,1);
		}
	}
}


void CHud::RenderVoting()
{
	if(!m_pClient->m_pVoting->IsVoting() || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	static const float TextX = m_Width / 16;
	static const float TextY = 1;
	static const float TextW = m_Width / 2 - m_Width / 8;
	static const float TextH = 42;

	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0,0,0,0.40f);
	RenderTools()->DrawRoundRect(TextX-5, TextY, TextW+15, TextH, 5.0f);
#if defined(__ANDROID__)
	RenderTools()->DrawRoundRect(TextX-5, TextY+TextH+2, TextW/2-10, 30, 5.0f);
	RenderTools()->DrawRoundRect(TextX+TextW/2+20, TextY+TextH+2, TextW/2-10, 30, 5.0f);
#endif
	Graphics()->QuadsEnd();

	TextRender()->TextColor(1,1,1,1);

	CTextCursor Cursor;
	char aBuf[512];
	str_format(aBuf, sizeof(aBuf), Localize("%ds left"), m_pClient->m_pVoting->SecondsLeft());
	float tw = TextRender()->TextWidth(0x0, 10, aBuf, -1);
	TextRender()->SetCursor(&Cursor, TextX+TextW-tw, 0.0f, 10.0f, TEXTFLAG_RENDER);
	TextRender()->TextEx(&Cursor, aBuf, -1);

	TextRender()->SetCursor(&Cursor, TextX, 0.0f, 10.0f, TEXTFLAG_RENDER);
	Cursor.m_LineWidth = TextW-tw;
	Cursor.m_MaxLines = 3;
	TextRender()->TextEx(&Cursor, m_pClient->m_pVoting->VoteDescription(), -1);

	// reason
	str_format(aBuf, sizeof(aBuf), "%s %s", Localize("Reason:"), m_pClient->m_pVoting->VoteReason());
	TextRender()->SetCursor(&Cursor, TextX, 23.0f, 10.0f, TEXTFLAG_RENDER|TEXTFLAG_STOP_AT_END);
	Cursor.m_LineWidth = TextW;
	TextRender()->TextEx(&Cursor, aBuf, -1);

	CUIRect Base = {TextX, TextH - 8, TextW, 4};
	m_pClient->m_pVoting->RenderBars(Base, false);

#if !defined(__ANDROID__)
	const char *pYesKey = m_pClient->m_pBinds->GetKey("vote yes");
	const char *pNoKey = m_pClient->m_pBinds->GetKey("vote no");
	str_format(aBuf, sizeof(aBuf), "%s - %s", pYesKey, Localize("Vote yes"));
	Base.y += Base.h+1;
	UI()->DoLabel(&Base, aBuf, 6.0f, -1);

	str_format(aBuf, sizeof(aBuf), "%s - %s", Localize("Vote no"), pNoKey);
	UI()->DoLabel(&Base, aBuf, 6.0f, 1);
#else
	Base.y += Base.h+15;
	UI()->DoLabel(&Base, Localize("Vote yes"), 16.0f, -1);
	UI()->DoLabel(&Base, Localize("Vote no"), 16.0f, 1);
	if( Input()->KeyDown(KEY_MOUSE_1) )
	{
		float mx, my;
		Input()->GetMousePosition(&mx, &my);
		mx = mx * m_Width / Graphics()->ScreenWidth();
		my = my * m_Height / Graphics()->ScreenHeight();
		if( my > TextY+TextH-10 && my < TextY+TextH+30 ) {
			if( mx > TextX-5 && mx < TextX-5+TextW/2-10 )
				m_pClient->m_pVoting->Vote(1);
			if( mx > TextX+TextW/2+20 && mx < TextX+TextW/2+20+TextW/2-10 )
				m_pClient->m_pVoting->Vote(-1);
		}
	}

	enum { LEFT_JOYSTICK_X = 0, LEFT_JOYSTICK_Y = 1 };
	int TouchX = SDL_JoystickGetAxis(m_pClient->m_pControls->m_TouchJoy, LEFT_JOYSTICK_X);
	int TouchY = SDL_JoystickGetAxis(m_pClient->m_pControls->m_TouchJoy, LEFT_JOYSTICK_Y);

	if( TouchY < -20000 )
	{
		m_pClient->m_pVoting->Vote(TouchX < 0);
	}
#endif
}

void CHud::RenderCursor()
{
	if(!m_pClient->m_Snap.m_pLocalCharacter || Client()->State() == IClient::STATE_DEMOPLAYBACK)
		return;

	MapscreenToGroup(m_pClient->m_pCamera->m_Center.x, m_pClient->m_pCamera->m_Center.y, Layers()->GameGroup());
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_GAME].m_Id);
	Graphics()->QuadsBegin();

	// render cursor
	RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS].m_pSpriteCursor);
	float CursorSize = 64;
	RenderTools()->DrawSprite(m_pClient->m_pControls->m_TargetPos.x, m_pClient->m_pControls->m_TargetPos.y, CursorSize);
	Graphics()->QuadsEnd();
}


void CHud::DrawCircular(float x, float y, float r, int Segments, int FillAmount, int Max, bool Flip)
{
	float AOff = -pi/2;
	if (Flip)
		AOff = pi/2;
	
	IGraphics::CFreeformItem Array[32];
	int NumItems = 0;
	float FSegments = (float)Segments;
	for(int i = 0; i < Segments; i+=2)
	{
		if ((i*Max)/FSegments < FillAmount)
			continue;
		
		float a1 = i/FSegments * 1*pi +AOff;
		float a2 = (i+1)/FSegments * 1*pi +AOff;
		float a3 = (i+2)/FSegments * 1*pi +AOff;
		float Ca1 = cosf(a1);
		float Ca2 = cosf(a2);
		float Ca3 = cosf(a3);
		float Sa1 = sinf(a1);
		float Sa2 = sinf(a2);
		float Sa3 = sinf(a3);

		if (!Flip)
		{
			Array[NumItems++] = IGraphics::CFreeformItem(
				x, y,
				x+Ca1*r, y+Sa1*r,
				x+Ca3*r, y+Sa3*r,
				x+Ca2*r, y+Sa2*r);
		}
		else
		{
			Array[NumItems++] = IGraphics::CFreeformItem(
				x, y,
				x+Ca1*r, y-Sa1*r,
				x+Ca3*r, y-Sa3*r,
				x+Ca2*r, y-Sa2*r);
		}
		
		if(NumItems == 32)
		{
			m_pClient->Graphics()->QuadsDrawFreeform(Array, 32);
			NumItems = 0;
		}
	}
	if(NumItems)
		m_pClient->Graphics()->QuadsDrawFreeform(Array, NumItems);
}



void CHud::RenderHealthAndAmmo(const CNetObj_Character *pCharacter)
{
	if(!pCharacter)
		return;

	//mapscreen_to_group(gacenter_x, center_y, layers_game_group());

	
	vec2 Area1Pos = vec2(0, 0);
	vec2 Area2Pos = vec2(38, 5);
	
	float x = Area2Pos.x; // 16
	float y = 5;

	
	CustomStuff()->m_SelectedGroup = pCharacter->m_SelectedGroup+1;
	
	// render ammo count
	// render gui stuff
	if (aCustomWeapon[pCharacter->m_Weapon%NUM_WEAPONS].m_MaxAmmo > 0)
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_PICKUPS].m_Id);

		Graphics()->QuadsBegin();

		// if weaponstage is active, put a "glow" around the stage ammo
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[pCharacter->m_Weapon%NUM_WEAPONS].m_pSpritePickup);
		IGraphics::CQuadItem Array[30];
		int i;
		
		float d = 30.0f / aCustomWeapon[pCharacter->m_Weapon%NUM_WEAPONS].m_MaxAmmo;
		
		for (i = 0; i < min(pCharacter->m_AmmoCount, 30); i++)
			Array[i] = IGraphics::CQuadItem(x+i*4*d,y+12, 6,12);
		Graphics()->QuadsDrawTL(Array, i);
		Graphics()->QuadsEnd();
	}

	
	// new health bar
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_HP].m_Id);
	Graphics()->QuadsBegin();
	
	vec2 HpSize = vec2(120, 12);
	
	{ // hp fill
		float f = min(pCharacter->m_Health, 100) / 100.0f;
		Graphics()->SetColor(1, 0, 0, 1);
		Graphics()->QuadsSetSubsetFree(0, 0.5f, 1*f, 0.5f, 0, 1, 1*f, 1);

		IGraphics::CFreeformItem FreeFormItem(
			x, y,
			x+f*HpSize.x, y,
			x, y+HpSize.y,
			x+f*HpSize.x, y+HpSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	{ // hp frame
		Graphics()->SetColor(1, 1, 1, 1);
		Graphics()->QuadsSetSubsetFree(0, 0, 1, 0, 0, 0.5f, 1, 0.5f); // nice way to pick a sprite

		IGraphics::CFreeformItem FreeFormItem(
			x, y,
			x+HpSize.x, y,
			x, y+HpSize.y,
			x+HpSize.x, y+HpSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	
	Graphics()->QuadsEnd();
	
	
	// new jetpack meter
	x = -1;
	y = -1;
	
	vec2 FrameSize = vec2(38, 38);
	int Fuel = pCharacter->m_JetpackPower/2;
	
	
	
	// buff duration
	x = -1;
	y = -1;
	
	
	int BuffTime = 100 - (Client()->GameTick() - CustomStuff()->m_Local.m_BuffStartTick)*5.0f / Client()->GameTickSpeed();
	
	if (CustomStuff()->m_Local.m_Buff < 0)
		BuffTime = -1;
	

	
	
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(1, Fuel*0.01f, 0, 1);
	DrawCircular(x+19, y+19, 12.5f, 64, 100-Fuel, 100);
	
	if (BuffTime < 0)
		DrawCircular(x+19, y+19, 12.5f, 64, 100-Fuel, 100, true);
	else
	{
		Graphics()->SetColor(0.15f, 0.5f+BuffTime*0.005f, 0.3f, 1);
		DrawCircular(x+19, y+19, 12.5f, 64, 100-BuffTime, 100, true);
	}
	Graphics()->QuadsEnd();
	
	
	// frame
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_FUEL].m_Id);
	Graphics()->QuadsBegin();
	{
		Graphics()->SetColor(1, 1, 1, 1);
		Graphics()->QuadsSetSubsetFree(0, 0, 1, 0, 0, 1, 1, 1);

		IGraphics::CFreeformItem FreeFormItem(
			x, y,
			x+FrameSize.x, y,
			x, y+FrameSize.y,
			x+FrameSize.x, y+FrameSize.y);

		Graphics()->QuadsDrawFreeform(&FreeFormItem, 1);
	}
	Graphics()->QuadsEnd();

	
	if (BuffTime > 0)
	{
		x = Area1Pos.x+18.5f; // 16
		y = Area1Pos.y+18.5f;
		
		// buff
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_ITEMS].m_Id);
		Graphics()->QuadsBegin();
		
		Graphics()->SetColor(1, 1, 1, 1);

		RenderTools()->SelectSprite(SPRITE_ITEM1+CustomStuff()->m_Local.m_Buff);
		RenderTools()->DrawSprite(x, y, 18);

		Graphics()->QuadsEnd();
		
		/*
		char aBuf[32];
		str_format(aBuf, sizeof(aBuf), "%d", BuffTime);
		TextRender()->TextColor(0, 1, 1, 1);
		TextRender()->Text(0, x+6, y+2, 6, aBuf, -1);
		*/
	}

	
	x = Area2Pos.x; // 16
	y = Area2Pos.y;

	
	// buildings
	if (m_pClient->m_pControls->m_BuildMode && m_pClient->BuildingEnabled())
	{
		float Size = 0.2f;
		y += 20;
		x += 12;
		
		for (int i = 0; i < NUM_KITS; i++)
		{
			if (i != 0)
				x += 180*Size;
			
			int Cost = BuildableCost[i];
			
			// draw building
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_BUILDINGS].m_Id);
			Graphics()->QuadsBegin();
	
			Graphics()->SetColor(1, 1, 1, 1);
			
			RenderTools()->SelectSprite(SPRITE_KIT_BARREL+i);
			RenderTools()->DrawSprite(x, y, 20);

			Graphics()->QuadsEnd();
			
			
			if (m_pClient->m_pControls->m_SelectedBuilding == i+1)
			{
				Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
				Graphics()->QuadsBegin();
				
				if (Cost > CustomStuff()->m_LocalKits)
					Graphics()->SetColor(1, 0, 0, 0.7f);
				else
					Graphics()->SetColor(0, 1, 0, 0.7f);
				
				RenderTools()->SelectSprite(SPRITE_KIT_BARREL+i);
				RenderTools()->DrawSprite(x, y, 20);
				
				Graphics()->QuadsEnd();
				Graphics()->ShaderEnd();
			}
			
			// draw cost
			char aBuf[8];
			str_format(aBuf, sizeof(aBuf), "%d", Cost);
			
			if (Cost > CustomStuff()->m_LocalKits)
				TextRender()->TextColor(1, 0, 0, 1);
			else
				TextRender()->TextColor(0, 1, 0, 1);
			
			TextRender()->Text(0, x+6, y+2, 6, aBuf, -1);
			TextRender()->TextColor(1, 1, 1, 1);
			
			Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1, 1, 1, 1.0f);
			RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
			RenderTools()->DrawSprite(x+14, y+6.5f, 8);
			Graphics()->QuadsEnd();
		}
		
		x = Area2Pos.x + 134;
		y = 16;
		
		// back to weapon helper
		int iw = clamp(CustomStuff()->m_LatestWeapon, 0, NUM_WEAPONS-1);
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();

		Graphics()->SetColor(1, 1, 1, 1);

		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[iw].m_pSpriteBody);
		RenderTools()->DrawSprite(x, y, 24);
		Graphics()->QuadsEnd();
		

		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+9, y+1, 8, m_pClient->m_pBinds->GetKey("+build"), -1);
		TextRender()->TextColor(1, 1, 1, 1);
		
		x = Area2Pos.x + 164;
		y = 16;
		
		// number of contruction kits
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1, 1, 1, 1.0f);
		RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
		RenderTools()->DrawSprite(x, y, 20);
		Graphics()->QuadsEnd();
		
		char aBuf[8];
		str_format(aBuf, sizeof(aBuf), "%d", clamp(CustomStuff()->m_LocalKits ,0, 9));
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+7, y+2, 8, aBuf, -1);
		TextRender()->TextColor(1, 1, 1, 1);

		return;
	}
	
	
	y += 31;
	
	
	// weapons
	float Size = 0.2f;
	int iw = pCharacter->m_WeaponGroup1;

	if (m_pClient->m_pControls->m_SignalWeapon >= 0)
	{
		CustomStuff()->m_WeaponSignalTimer = 1.0f;
		CustomStuff()->m_WeaponSignal = m_pClient->m_pControls->m_SignalWeapon;
		m_pClient->m_pControls->m_SignalWeapon = -1;
	}
	
#if !defined(__ANDROID__)
	
	Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
	
	for (int i = 1; i < NUM_WEAPONS; i++)
	{
		if (i != 1)
			x += 40*Size;
		
		bool Got = true;
		bool Upgraded = true;
		bool Upgraded2 = true;
		
		int w = CustomStuff()->m_LocalWeapons;
		if (!(w & (1<<i)))
			Got = false;
		
		int u = CustomStuff()->m_LocalUpgrades;
		if (!(u & (1<<i)))
			Upgraded = false;
		
		u = CustomStuff()->m_LocalUpgrades2;
		if (!(u & (1<<i)))
			Upgraded2 = false;
		
		if (CustomStuff()->m_WeaponpickTimer > 0.0f)
		{
			int pw = clamp(CustomStuff()->m_WeaponpickWeapon, 0, NUM_WEAPONS-1);
			if (i == pw)
			{
				Graphics()->QuadsBegin();
				float a = sin(CustomStuff()->m_WeaponpickTimer*pi)*sin(CustomStuff()->m_WeaponpickTimer*pi);
				
				Graphics()->SetColor(1, 1, 1, a);
				
				RenderTools()->SelectSprite(SPRITE_WEAPON_PICKUP);
				RenderTools()->DrawSprite(x, y, 32);
				Graphics()->QuadsEnd();
			}
		}

		if (Got && Upgraded)
		{
			Graphics()->QuadsBegin();
			Graphics()->SetColor(1, 1, 1, 0.5f);
			
			if (Upgraded2)
				RenderTools()->SelectSprite(SPRITE_WEAPON_UPGRADED2);
			else
				RenderTools()->SelectSprite(SPRITE_WEAPON_UPGRADED1);
			
			RenderTools()->DrawSprite(x, y, 18);
			Graphics()->QuadsEnd();
		}
		
		if (!Got)
			Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.5f);
		
		Graphics()->QuadsBegin();
		
		if (!Got)
			Graphics()->SetColor(1, 1, 1, 0.5f);
		else
			Graphics()->SetColor(1, 1, 1, 1);
		
		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
		RenderTools()->DrawSprite(x, y, g_pData->m_Weapons.m_aId[i].m_VisualSize * Size);

		Graphics()->QuadsEnd();
		Graphics()->ShaderEnd();

		// can't pick a weapon
		if (CustomStuff()->m_WeaponSignalTimer > 0.0f)
		{
			int pw = CustomStuff()->m_WeaponSignal;
			if (i == pw)
			{
				Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
				Graphics()->QuadsBegin();
				
				Graphics()->SetColor(1.0f, 0.0f, 0.0f, CustomStuff()->m_WeaponSignalTimer * 0.75f);
				
				RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
				RenderTools()->DrawSprite(x, y, g_pData->m_Weapons.m_aId[i].m_VisualSize * Size);
				
				Graphics()->QuadsEnd();
				Graphics()->ShaderEnd();
			}
		}
		
		// green layer for selected weapon
		if (iw == i)
		{
			Graphics()->ShaderBegin(SHADER_GRAYSCALE, 0.0f);
			Graphics()->QuadsBegin();
			
			Graphics()->SetColor(0.0f, 1.0f, 0.0f, 0.5f);
			
			RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[i].m_pSpriteBody);
			RenderTools()->DrawSprite(x, y, g_pData->m_Weapons.m_aId[i].m_VisualSize * Size);
			
			Graphics()->QuadsEnd();
			Graphics()->ShaderEnd();
			
			CustomStuff()->m_LatestWeapon = i;
		}
		
		x += 40*Size;
	}
#endif

	x = Area2Pos.x + 134; y = 16;
	
	// tool / build helper
#if !defined(__ANDROID__)
	if (!m_pClient->IsLocalUndead() && m_pClient->BuildingEnabled())
	{
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();

		Graphics()->SetColor(1, 1, 1, 1);

		RenderTools()->SelectSprite(g_pData->m_Weapons.m_aId[WEAPON_TOOL].m_pSpriteBody);
		RenderTools()->DrawSprite(x, y, 24);
		Graphics()->QuadsEnd();
		

		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+9, y+1, 8, m_pClient->m_pBinds->GetKey("+build"), -1);
	
		x = Area2Pos.x + 164;
		y = 16;
			
		// number of contruction kits
		Graphics()->TextureSet(g_pData->m_aImages[IMAGE_WEAPONS].m_Id);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1, 1, 1, 1.0f);
		RenderTools()->SelectSprite(SPRITE_PICKUP_KIT);
		RenderTools()->DrawSprite(x, y, 20);
		Graphics()->QuadsEnd();


		char aBuf[8];
		str_format(aBuf, sizeof(aBuf), "%d", clamp(CustomStuff()->m_LocalKits ,0, 9));
		TextRender()->TextColor(0.2f, 0.7f, 0.2f, 1);
		TextRender()->Text(0, x+7, y+2, 8, aBuf, -1);
		TextRender()->TextColor(1, 1, 1, 1);
	}
#endif
}

void CHud::RenderSpectatorHud()
{
	// draw the box
	Graphics()->TextureSet(-1);
	Graphics()->QuadsBegin();
	Graphics()->SetColor(0.0f, 0.0f, 0.0f, 0.4f);
	RenderTools()->DrawRoundRectExt(m_Width-180.0f, m_Height-15.0f, 180.0f, 15.0f, 5.0f, CUI::CORNER_TL);
	Graphics()->QuadsEnd();

	// draw the text
	char aBuf[128];
	str_format(aBuf, sizeof(aBuf), "%s: %s", Localize("Spectate"), m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW ?
		m_pClient->m_aClients[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_aName : Localize("Free-View"));
	TextRender()->Text(0, m_Width-174.0f, m_Height-13.0f, 8.0f, aBuf, -1);
}

void CHud::RenderTouchscreenButtons()
{
#if defined(__ANDROID__)
	if (!m_pClient->m_pControls->m_TouchJoyAimPressed || g_Config.m_ClTouchscreenJumpButton)
	{
		vec2 Screen = vec2(Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
		Graphics()->MapScreen(0, 0, Screen.x, Screen.y);
		Graphics()->TextureSet(gs_JumpButtonTexture);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.4f);
		IGraphics::CQuadItem QuadItem(Screen.x * (0.5f + 0.5f * (m_pClient->m_pControls->m_TouchJoyJumpButtonPos.x + 32768) / 65536.0f),
										Screen.y * (0.2f + 0.8f * (m_pClient->m_pControls->m_TouchJoyJumpButtonPos.y + 32768) / 65536.0f),
										Screen.y * 0.1f, Screen.y * 0.1f);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	if (g_Config.m_ClTouchscreenFireButton)
	{
		vec2 Screen = vec2(Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
		Graphics()->MapScreen(0, 0, Screen.x, Screen.y);
		Graphics()->TextureSet(gs_FireButtonTexture);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.4f);
		IGraphics::CQuadItem QuadItem(Screen.x * (0.5f + 0.5f * (m_pClient->m_pControls->m_TouchJoyFireButtonPos.x + 32768) / 65536.0f),
										Screen.y * (0.2f + 0.8f * (m_pClient->m_pControls->m_TouchJoyFireButtonPos.y + 32768) / 65536.0f),
										Screen.y * 0.2f, Screen.y * 0.2f);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}

	if (g_Config.m_ClTouchscreenFixedDpad)
	{
		vec2 Screen = vec2(Graphics()->ScreenWidth(), Graphics()->ScreenHeight());
		Graphics()->MapScreen(0, 0, Screen.x, Screen.y);
		Graphics()->TextureSet(gs_DpadTexture);
		Graphics()->QuadsBegin();
		Graphics()->SetColor(1.0f, 1.0f, 1.0f, 0.4f);
		IGraphics::CQuadItem QuadItem(Screen.x * (2.0f / 16.0f), Screen.y * (1.0f - 0.8f / 6.0f),
										Screen.y * 0.15f, Screen.y * 0.15f);
		Graphics()->QuadsDraw(&QuadItem, 1);
		Graphics()->QuadsEnd();
	}
#endif
}

void CHud::OnRender()
{
	if(!m_pClient->m_Snap.m_pGameInfoObj)
		return;

	m_Width = 300.0f*Graphics()->ScreenAspect();
	m_Height = 300.0f;
	Graphics()->MapScreen(0.0f, 0.0f, m_Width, m_Height);

	if(g_Config.m_ClShowhud)
	{
		if(m_pClient->m_Snap.m_pLocalCharacter && !(m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_GAMEOVER))
			RenderHealthAndAmmo(m_pClient->m_Snap.m_pLocalCharacter);
		else if(m_pClient->m_Snap.m_SpecInfo.m_Active)
		{
			if(m_pClient->m_Snap.m_SpecInfo.m_SpectatorID != SPEC_FREEVIEW)
				RenderHealthAndAmmo(&m_pClient->m_Snap.m_aCharacters[m_pClient->m_Snap.m_SpecInfo.m_SpectatorID].m_Cur);
			RenderSpectatorHud();
		}

		//RenderGameTimer();
		RenderPauseNotification();
		RenderSuddenDeath();
		RenderScoreHud();
		RenderWarmupTimer();
		RenderFps();
		if(Client()->State() != IClient::STATE_DEMOPLAYBACK)
			RenderConnectionWarning();
		RenderTeambalanceWarning();
		RenderVoting();
	}
	RenderCursor();
	RenderTouchscreenButtons();
}
