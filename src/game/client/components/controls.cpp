

#include <base/math.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/components/picker.h>
#include <game/client/components/weaponbar.h>
#include <game/client/components/chat.h>
#include <game/client/components/menus.h>
#include <game/client/components/scoreboard.h>

#include <game/client/customstuff.h>

#if defined(__ANDROID__)
#include <SDL.h>
#include <SDL_screenkeyboard.h>
#endif

#include "controls.h"

#if defined(__ANDROID__)
// Android constants
enum {	LEFT_JOYSTICK_X = 0, LEFT_JOYSTICK_Y = 1,
		RIGHT_JOYSTICK_X = 2, RIGHT_JOYSTICK_Y = 3,
		TOP_JOYSTICK_X = 20, TOP_JOYSTICK_Y = 21,
		ORIENTATION_X = 8, ORIENTATION_Y = 9, ORIENTATION_Z = 10,
		ACCELEROMETER_X = 0, ACCELEROMETER_Y = 1,
		NUM_JOYSTICK_AXES = 22 };
#endif

CControls::CControls()
{
	mem_zero(&m_LastData, sizeof(m_LastData));

#if defined(__ANDROID__)
	SDL_Init(SDL_INIT_JOYSTICK);
	m_TouchJoy = SDL_JoystickOpen(0);
	if( m_TouchJoy && SDL_JoystickNumAxes(m_TouchJoy) < NUM_JOYSTICK_AXES )
	{
		SDL_JoystickClose(m_TouchJoy);
		m_TouchJoy = NULL;
	}

	m_Gamepad = SDL_JoystickOpen(2);

	m_Accelerometer = NULL;

	SDL_JoystickEventState(SDL_QUERY);

	m_UsingGamepad = false;
	if( getenv("OUYA") )
		m_UsingGamepad = true;
#endif
}

void CControls::OnReset()
{
	m_LastData.m_Direction = 0;
	m_LastData.m_Hook = 0;
	// simulate releasing the fire button
	if((m_LastData.m_Fire&1) != 0)
		m_LastData.m_Fire++;
	m_LastData.m_Fire &= INPUT_STATE_MASK;
	m_LastData.m_Jump = 0;
	m_InputData = m_LastData;

	m_InputDirectionLeft = 0;
	m_InputDirectionRight = 0;
	
	m_PickedWeapon = -1;
	m_SignalWeapon = -1;
	
	m_Build = 0;
	m_BuildReleased = true;
	m_BuildMode = false;
	m_LastWeapon = 1;
	m_SelectedBuilding = -1;

	m_TouchJoyRunPressed = false;
	m_TouchJoyRunTapTime = 0;
	m_TouchJoyRunAnchor = ivec2(0,0);
	//m_TouchJoyRunLastPos = ivec2(0,0);
	m_TouchJoyAimPressed = false;
	m_TouchJoyAimAnchor = ivec2(0,0);
	m_TouchJoyAimLastPos = ivec2(20000,20000);
	m_TouchJoyAimTapTime = 0;
	m_TouchJoyFirePressed = false;
	m_TouchJoyRightJumpPressed = false;
	m_TouchJoyWeaponbarPressed = false;
	m_WeaponIdxOutOfAmmo = -1;
	m_OldMouseX = m_OldMouseY = 0.0f;
}

void CControls::OnRelease()
{
	OnReset();
}

void CControls::OnPlayerDeath()
{
	m_LastData.m_WantedWeapon = m_InputData.m_WantedWeapon = 0;
	m_WeaponIdxOutOfAmmo = -1;
}

static void ConKeyInputState(IConsole::IResult *pResult, void *pUserData)
{
	((int *)pUserData)[0] = pResult->GetInteger(0);
}

static void ConKeyInputCounter(IConsole::IResult *pResult, void *pUserData)
{
	int *v = (int *)pUserData;
	if(((*v)&1) != pResult->GetInteger(0))
		(*v)++;
	*v &= INPUT_STATE_MASK;
}

struct CInputSet
{
	CControls *m_pControls;
	int *m_pVariable;
	int *m_pVariable2; // for building
	int m_Value;
};

static void ConKeyInputSet(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	if(pResult->GetInteger(0))
	{
		*pSet->m_pVariable = pSet->m_Value;
		*pSet->m_pVariable2 = pSet->m_Value-1; // ugly!
	}
}

static void ConKeyInputNextPrevWeapon(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	ConKeyInputCounter(pResult, pSet->m_pVariable);
	pSet->m_pControls->m_InputData.m_WantedWeapon = 0;
	pSet->m_pControls->m_WeaponIdxOutOfAmmo = -1;
}

void CControls::OnConsoleInit()
{
	// game commands
	Console()->Register("+left", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+right", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+down", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Down, "Slide / down");
	Console()->Register("+jump", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+turbo", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Turbo");
	Console()->Register("+fire", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_InputData.m_Fire, "Fire");
	Console()->Register("+build", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_Build, "Build");

	// gamepad
	Console()->Register("+gamepadleft", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+gamepadright", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+gamepadjump", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+gamepadturbo", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Turbo");
	//Console()->Register("+gamepadpicker", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Weapon picker");
	Console()->Register("+gamepadfire", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_InputData.m_Fire, "Fire");

	/*
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 1}; Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to primary weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 2}; Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to secondary weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 3}; Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to tool"); }
	*/
	

	// can't pick tool except with build key
	//{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 1}; Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon1"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 2}; Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon2"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 3}; Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon3"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 4}; Console()->Register("+weapon4", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon4"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 5}; Console()->Register("+weapon5", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon5"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 6}; Console()->Register("+weapon6", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon6"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 7}; Console()->Register("+weapon7", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon7"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 8}; Console()->Register("+weapon8", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon8"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, &m_SelectedBuilding, 9}; Console()->Register("+weapon9", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to weapon9"); }

	/*
	{ static CInputSet s_Set = {this, &m_SelectedBuilding, 1}; Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Build1"); }
	{ static CInputSet s_Set = {this, &m_SelectedBuilding, 2}; Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Build2"); }
	{ static CInputSet s_Set = {this, &m_SelectedBuilding, 3}; Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Build3"); }
	*/
	
	//{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 1}; Console()->Register("+gamepadweapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to primary weapon"); }
	//{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 2}; Console()->Register("+gamepadweapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to secondary weapon"); }

	{ static CInputSet s_Set = {this, &m_InputData.m_NextWeapon, 0}; Console()->Register("+nextweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to next weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_PrevWeapon, 0}; Console()->Register("+prevweapon", "", CFGFLAG_CLIENT, ConKeyInputNextPrevWeapon, (void *)&s_Set, "Switch to previous weapon"); }
}

void CControls::OnMessage(int Msg, void *pRawMsg)
{
	/*
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		if(g_Config.m_ClAutoswitchWeapons)
			m_InputData.m_WantedWeapon = pMsg->m_Weapon+1;
		// We don't really know ammo count, until we'll switch to that weapon, but any non-zero count will suffice here
	}
	*/
	
	if(Msg == NETMSGTYPE_SV_WEAPONPICKUP)
	{
		CNetMsg_Sv_WeaponPickup *pMsg = (CNetMsg_Sv_WeaponPickup *)pRawMsg;
		bool HaveOtherWeapons = (CustomStuff()->m_LocalWeapons & ~((1 << WEAPON_HAMMER) | (1 << WEAPON_TOOL)));
		CustomStuff()->m_LocalWeapons |= 1 << (pMsg->m_Weapon);
		CustomStuff()->m_WeaponpickTimer = 1.0f;
		CustomStuff()->m_WeaponpickWeapon = pMsg->m_Weapon;
		CustomStuff()->m_LastWeaponPicked = false;
		m_WeaponIdxOutOfAmmo = -1;
		if(g_Config.m_ClAutoswitchWeapons || (g_Config.m_ClAutoswitchWeaponsOutOfAmmo && !HaveOtherWeapons))
		{
			/* old way using weapon groups
			char aBuf[32];
			str_format(aBuf, sizeof(aBuf), "weaponpick %d", pMsg->m_Weapon-1);
			Console()->ExecuteLine(aBuf);
			*/

			// not working yet
			m_InputData.m_WantedWeapon = pMsg->m_Weapon+1;
		}
	}
}


//input count
struct CInputCount
{
	int m_Presses;
	int m_Releases;
};

CInputCount CountInput(int Prev, int Cur)
{
	CInputCount c = {0, 0};
	Prev &= INPUT_STATE_MASK;
	Cur &= INPUT_STATE_MASK;
	int i = Prev;

	while(i != Cur)
	{
		i = (i+1)&INPUT_STATE_MASK;
		if(i&1)
			c.m_Presses++;
		else
			c.m_Releases++;
	}

	return c;
}


int CControls::SnapInput(int *pData)
{
	static int64 LastSendTime = 0;
	bool Send = false;

	// update player state
	if(m_pClient->m_pChat->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_CHATTING;
	else if(m_pClient->m_pMenus->IsActive())
		m_InputData.m_PlayerFlags = PLAYERFLAG_IN_MENU;
	else
		m_InputData.m_PlayerFlags = PLAYERFLAG_PLAYING;

	if(m_pClient->m_pScoreboard->Active())
		m_InputData.m_PlayerFlags |= PLAYERFLAG_SCOREBOARD;

	if(m_LastData.m_PlayerFlags != m_InputData.m_PlayerFlags)
		Send = true;

	m_LastData.m_PlayerFlags = m_InputData.m_PlayerFlags;

	// we freeze the input if chat or menu is activated
	if(!(m_InputData.m_PlayerFlags&PLAYERFLAG_PLAYING))
	{
		OnReset();

		mem_copy(pData, &m_InputData, sizeof(m_InputData));

		// send once a second just to be sure
		if(time_get() > LastSendTime + time_freq())
			Send = true;
	}
	else
	{

		m_InputData.m_TargetX = (int)m_MousePos.x;
		m_InputData.m_TargetY = (int)m_MousePos.y;
		if(!m_InputData.m_TargetX && !m_InputData.m_TargetY)
		{
			m_InputData.m_TargetX = 1;
			m_MousePos.x = 1;
		}

		// set direction
		m_InputData.m_Direction = 0;
		if(m_InputDirectionLeft && !m_InputDirectionRight)
			m_InputData.m_Direction = -1;
		if(!m_InputDirectionLeft && m_InputDirectionRight)
			m_InputData.m_Direction = 1;

		// stress testing
		if(g_Config.m_DbgStress)
		{
			float t = Client()->LocalTime();
			mem_zero(&m_InputData, sizeof(m_InputData));

			m_InputData.m_Direction = ((int)t/2)&1;
			m_InputData.m_Jump = ((int)t);
			m_InputData.m_Fire = ((int)(t*10));
			m_InputData.m_Hook = ((int)(t*2))&1;
			m_InputData.m_WantedWeapon = ((int)t)%NUM_WEAPONS;
			m_InputData.m_TargetX = (int)(sinf(t*3)*100.0f);
			m_InputData.m_TargetY = (int)(cosf(t*3)*100.0f);
		}

		// get wanted weapon from picker
		if (m_PickedWeapon >= 0 && !m_BuildMode)
			m_InputData.m_WantedWeapon = m_PickedWeapon;
		
		m_PickedWeapon = -1;
		
		// can't want a weapon you don't have to prevent weapon change on picking wanted weapon
		int w = CustomStuff()->m_LocalWeapons;
		if (!m_BuildMode && !(w & (1<<(m_InputData.m_WantedWeapon-1))) && m_InputData.m_WantedWeapon > 0)
		{
			m_SignalWeapon = m_InputData.m_WantedWeapon-1;
			m_InputData.m_WantedWeapon = CustomStuff()->m_LocalWeapon+1;
		}
		
		if (m_Build == 0)
			m_BuildReleased = true;
		
		if (m_BuildReleased && m_Build && m_pClient->BuildingEnabled())
		{
			m_BuildMode = !m_BuildMode;
			m_BuildReleased = false;
			
			m_SelectedBuilding = -1;
			
			if (!m_BuildMode)
				m_InputData.m_WantedWeapon = m_LastWeapon;
			else
				m_LastWeapon = CustomStuff()->m_LocalWeapon+1;
		}
		
		if (m_BuildMode)
		{
			//if (m_InputData.m_WantedWeapon > 1)
			//	m_SelectedBuilding = m_InputData.m_WantedWeapon;
			
			m_InputData.m_WantedWeapon = 1;
			m_InputData.m_NextWeapon = 0;
			m_InputData.m_PrevWeapon = 0;
		}
		
		// place buildings
		if (m_InputData.m_Fire&1)
		{
			if (m_BuildMode && m_SelectedBuilding >= 0 && CustomStuff()->m_BuildPosValid &&
				CountInput(m_InputData.m_Fire, m_LastData.m_Fire).m_Presses)
			{
				CNetMsg_Cl_UseKit Msg;
				Msg.m_Kit = m_SelectedBuilding-1;
				Msg.m_X = CustomStuff()->m_BuildPos.x;
				Msg.m_Y = CustomStuff()->m_BuildPos.y+18;
				Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
			}
		}
		
		
		// check if we need to send input
		if(m_InputData.m_Direction != m_LastData.m_Direction) Send = true;
		else if(m_InputData.m_Jump != m_LastData.m_Jump) Send = true;
		else if(m_InputData.m_Fire != m_LastData.m_Fire) Send = true;
		else if(m_InputData.m_Hook != m_LastData.m_Hook) Send = true;
		else if(m_InputData.m_WantedWeapon != m_LastData.m_WantedWeapon) Send = true;
		else if(m_InputData.m_NextWeapon != m_LastData.m_NextWeapon) Send = true;
		else if(m_InputData.m_PrevWeapon != m_LastData.m_PrevWeapon) Send = true;

		// send at at least 10hz
		if(time_get() > LastSendTime + time_freq()/25)
			Send = true;
		
		m_Build = 0;
	}
	
	// copy and return size
	m_LastData = m_InputData;

	if(!Send)
		return 0;

	LastSendTime = time_get();
	mem_copy(pData, &m_InputData, sizeof(m_InputData));
	return sizeof(m_InputData);
}

void CControls::OnRender()
{
#if defined(__ANDROID__)
	if( m_TouchJoy && !m_UsingGamepad )
		TouchscreenInput();

	if( m_Gamepad )
		GamepadInput();
#endif

	if( g_Config.m_ClAutoswitchWeaponsOutOfAmmo )
		AutoswitchWeaponsOutOfAmmo();

	// update target pos
	if(m_pClient->m_Snap.m_pGameInfoObj && !m_pClient->m_Snap.m_SpecInfo.m_Active)
		m_TargetPos = m_pClient->m_LocalCharacterPos + m_MousePos;
	else if(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
		m_TargetPos = m_pClient->m_Snap.m_SpecInfo.m_Position + m_MousePos;
	else
		m_TargetPos = m_MousePos;
}

bool CControls::OnMouseMove(float x, float y)
{
	if((m_pClient->m_Snap.m_pGameInfoObj && m_pClient->m_Snap.m_pGameInfoObj->m_GameStateFlags&GAMESTATEFLAG_PAUSED) ||
		(m_pClient->m_Snap.m_SpecInfo.m_Active && m_pClient->m_pChat->IsActive()))
		return false;

#if defined(__ANDROID__)
	// We're using joystick on Android, mouse is disabled
	if( m_UsingGamepad && (m_OldMouseX != x || m_OldMouseY != y) )
	{
		m_OldMouseX = x;
		m_OldMouseY = y;
		m_MousePos = vec2((x - g_Config.m_GfxScreenWidth/2), (y - g_Config.m_GfxScreenHeight/2));
		ClampMousePos();
	}
#else
	Input()->SetMouseModes(IInput::MOUSE_MODE_WARP_CENTER);
	Input()->ShowCursor(false);

	Input()->GetRelativePosition(&x, &y);

	if (Input()->UsingGamepad())
	{
		if (m_pClient->m_Snap.m_SpecInfo.m_Active)
			m_MousePos += vec2(x, y) * ((200.0f + g_Config.m_InpMousesens) / 1500.0f);
		else
			m_MousePos += vec2(x, y) * ((200.0f + g_Config.m_InpMousesens) / 150.0f);
	}
	else
		m_MousePos += vec2(x, y) * ((200.0f + g_Config.m_InpMousesens) / 150.0f);
	ClampMousePos();
#endif

	return true;
}

void CControls::ClampMousePos()
{
	if(m_pClient->m_Snap.m_SpecInfo.m_Active && !m_pClient->m_Snap.m_SpecInfo.m_UsePosition)
	{
		m_MousePos.x = clamp(m_MousePos.x, 200.0f, Collision()->GetWidth()*32-200.0f);
		m_MousePos.y = clamp(m_MousePos.y, 200.0f, Collision()->GetHeight()*32-200.0f);
	}
	else
	{
		float CameraMaxDistance = 200.0f;
		float FollowFactor = g_Config.m_ClMouseFollowfactor/100.0f;
		float MouseMax = min(CameraMaxDistance/FollowFactor + g_Config.m_ClMouseDeadzone, (float)g_Config.m_ClMouseMaxDistance);

		if(length(m_MousePos) > MouseMax)
			m_MousePos = normalize(m_MousePos)*MouseMax;
	}
}

void CControls::AutoswitchWeaponsOutOfAmmo()
{
	if( ! m_pClient->m_Snap.m_pLocalCharacter )
		return;

	if( m_InputData.m_Fire % 2 != 0 &&
		m_pClient->m_Snap.m_pLocalCharacter->m_AmmoCount == 0 &&
		m_pClient->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_HAMMER &&
		m_pClient->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_TOOL )
	{
		if (m_WeaponIdxOutOfAmmo == -1)
			m_WeaponIdxOutOfAmmo = NUM_WEAPONS - 1;
		int w;
		for( w = m_WeaponIdxOutOfAmmo; w > WEAPON_HAMMER; w-- )
		{
			if( w == m_pClient->m_Snap.m_pLocalCharacter->m_Weapon )
				continue;
			if( CustomStuff()->m_LocalWeapons & (1 << w) )
				break;
		}
		m_WeaponIdxOutOfAmmo = w;
		//dbg_msg("controls", "Out of ammo - selected weapon %d current %d mask %x", w, m_pClient->m_Snap.m_pLocalCharacter->m_Weapon, CustomStuff()->m_LocalWeapons);
		if( w != m_pClient->m_Snap.m_pLocalCharacter->m_Weapon )
		{
			char aBuf[32];
			str_format(aBuf, sizeof(aBuf), "weaponpick %d", w - 1);
			Console()->ExecuteLine(aBuf);
		}
	}
}

#if defined(__ANDROID__)
void CControls::GamepadInput()
{
	enum {
		GAMEPAD_DEAD_ZONE = 65536 / 8,
	};

	// Get input from left joystick
	int RunX = SDL_JoystickGetAxis(m_Gamepad, LEFT_JOYSTICK_X);
	int RunY = SDL_JoystickGetAxis(m_Gamepad, LEFT_JOYSTICK_Y);
	if( m_UsingGamepad )
	{
		//m_InputDirectionLeft = (RunX < -GAMEPAD_DEAD_ZONE);
		//m_InputDirectionRight = (RunX > GAMEPAD_DEAD_ZONE);
		static int OldRunX = 0, OldRunY = 0;
		if( RunX < -GAMEPAD_DEAD_ZONE && OldRunX >= -GAMEPAD_DEAD_ZONE )
			m_InputDirectionLeft = 1;
		if( RunX >= -GAMEPAD_DEAD_ZONE && OldRunX < -GAMEPAD_DEAD_ZONE )
			m_InputDirectionLeft = 0;
		if( RunX > GAMEPAD_DEAD_ZONE && OldRunX <= GAMEPAD_DEAD_ZONE )
			m_InputDirectionRight = 1;
		if( RunX <= GAMEPAD_DEAD_ZONE && OldRunX > GAMEPAD_DEAD_ZONE )
			m_InputDirectionRight = 0;
		OldRunX = RunX;
		OldRunY = RunY;
	}

	// Get input from right joystick
	int AimX = SDL_JoystickGetAxis(m_Gamepad, RIGHT_JOYSTICK_X);
	int AimY = SDL_JoystickGetAxis(m_Gamepad, RIGHT_JOYSTICK_Y);
	if( abs(AimX) > GAMEPAD_DEAD_ZONE || abs(AimY) > GAMEPAD_DEAD_ZONE )
	{
		m_MousePos = vec2(AimX / 30, AimY / 30);
		ClampMousePos();
	}

	if( !m_UsingGamepad && (abs(AimX) > GAMEPAD_DEAD_ZONE || abs(AimY) > GAMEPAD_DEAD_ZONE ||
		abs(RunX) > GAMEPAD_DEAD_ZONE || abs(RunY) > GAMEPAD_DEAD_ZONE || (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))) )
	{
		UI()->AndroidShowScreenKeys(false);
		m_UsingGamepad = true;
	}
}

void CControls::WeaponBarInput()
{
	// Get input from the top joystick
	int TopX = SDL_JoystickGetAxis(m_TouchJoy, TOP_JOYSTICK_X);
	int TopY = SDL_JoystickGetAxis(m_TouchJoy, TOP_JOYSTICK_Y);
	bool TopPressed = (TopX != 0 || TopY != 0);

	if (TopPressed)
	{
		m_pClient->Weaponbar()->OnFingerTouch(vec2((TopX + 32768) / 65536.0f, (TopY + 32768) / 65536.0f));
	}

	if (TopPressed != m_TouchJoyWeaponbarPressed)
	{
		m_TouchJoyWeaponbarPressed = TopPressed;
		if (!TopPressed)
		{
			m_pClient->Weaponbar()->OnFingerRelease();
			m_WeaponIdxOutOfAmmo = -1;
		}
	}
}

void CControls::TouchscreenInput()
{
	WeaponBarInput();

	enum {
		TOUCHJOY_DEAD_ZONE = 65536 / 20,
		TOUCHJOY_AIM_DEAD_ZONE = 65536 / 10,
	};

	int64 CurTime = time_get();

	// Get input from the left joystick
	int RunX = SDL_JoystickGetAxis(m_TouchJoy, LEFT_JOYSTICK_X);
	int RunY = SDL_JoystickGetAxis(m_TouchJoy, LEFT_JOYSTICK_Y);
	bool RunPressed = (RunX != 0 || RunY != 0);
	// Get input from the right joystick
	ivec2 AimPos = ivec2(SDL_JoystickGetAxis(m_TouchJoy, RIGHT_JOYSTICK_X), SDL_JoystickGetAxis(m_TouchJoy, RIGHT_JOYSTICK_Y));
	bool AimPressed = (AimPos.x != 0 || AimPos.y != 0);
	bool oldTouchJoyRightJump = m_TouchJoyRightJumpPressed;

	// Process left joystick
	if( m_TouchJoyRunPressed != RunPressed )
	{
		if( RunPressed )
		{
			// Tap to jetpack, and do not reset the anchor coordinates, if tapped under 500ms
			m_TouchJoyRunAnchor = ivec2(RunX, RunY);
		}
		else
		{
			m_InputData.m_Hook = 0;
		}
		m_TouchJoyRunTapTime = CurTime;
		m_TouchJoyRunPressed = RunPressed;
	}

	if( RunPressed )
	{
		m_InputDirectionLeft = (RunX - m_TouchJoyRunAnchor.x < -TOUCHJOY_DEAD_ZONE);
		m_InputDirectionRight = (RunX - m_TouchJoyRunAnchor.x > TOUCHJOY_DEAD_ZONE);
		m_InputData.m_Down = (RunY - m_TouchJoyRunAnchor.y > TOUCHJOY_DEAD_ZONE * 3);
		// Activate run-assist jetpack if we slide finger up
		/*
		if( RunY - m_TouchJoyRunAnchor.y < -TOUCHJOY_DEAD_ZONE * 3 )
			m_InputData.m_Hook = 1;
		else
			m_InputData.m_Hook = 0;
		*/
		// Activate run-assist jetpack if we slide finger even more
		if( m_TouchJoyRunAnchor.x - RunX < -TOUCHJOY_DEAD_ZONE * 4 || m_TouchJoyRunAnchor.x - RunX > TOUCHJOY_DEAD_ZONE * 4 )
			m_InputData.m_Hook = 1;
		else
			m_InputData.m_Hook = 0;
		// Change your facing direction if not aiming
		if( !AimPressed )
		{
			if( m_InputDirectionRight && (m_MousePos.x <= 0 || m_InputData.m_Hook) )
			{
				m_MousePos.x = 100;
				m_MousePos.y = -11;
			}
			if( m_InputDirectionLeft && (m_MousePos.x >= 0 || m_InputData.m_Hook) )
			{
				m_MousePos.x = -100;
				m_MousePos.y = -11;
			}
		}
		// Move the anchor if we move the finger too much
		if( m_TouchJoyRunAnchor.x - RunX < -TOUCHJOY_DEAD_ZONE * 5 )
			m_TouchJoyRunAnchor.x = RunX - TOUCHJOY_DEAD_ZONE * 5;
		if( m_TouchJoyRunAnchor.x - RunX > TOUCHJOY_DEAD_ZONE * 5 )
			m_TouchJoyRunAnchor.x = RunX + TOUCHJOY_DEAD_ZONE * 5;
	}

	//dbg_msg("controls", "");

	if( !RunPressed )
	{
		m_InputDirectionLeft = 0;
		m_InputDirectionRight = 0;
		m_InputData.m_Down = 0;
	}

	// Process right joystick
	if( !AimPressed )
	{
		AimPos = m_TouchJoyAimLastPos;
	}

	if( AimPressed != m_TouchJoyAimPressed )
	{
		SDL_Rect joypos;
		SDL_ANDROID_GetScreenKeyboardButtonPos( SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2, &joypos );
		if( AimPressed )
		{
			if( distance(AimPos, m_TouchJoyAimLastPos) < TOUCHJOY_DEAD_ZONE * 2 )
			{
				m_TouchJoyRightJumpPressed = true;
			}
			m_TouchJoyAimAnchor = AimPos;
		}
		else
		{
			m_TouchJoyRightJumpPressed = false;
			if( m_TouchJoyAimTapTime + time_freq() / 4 > CurTime && distance(AimPos, m_TouchJoyAimAnchor) < TOUCHJOY_DEAD_ZONE )
				m_TouchJoyRightJumpPressed = true;
		}
		m_TouchJoyAimPressed = AimPressed;
		m_TouchJoyAimTapTime = CurTime;
	}

	if( AimPressed )
	{
		if (distance(AimPos, m_TouchJoyAimAnchor) > TOUCHJOY_DEAD_ZONE / 3)
			m_MousePos = vec2(AimPos.x - m_TouchJoyAimAnchor.x, AimPos.y - m_TouchJoyAimAnchor.y) / 30;
		ClampMousePos();
		if( m_TouchJoyAimTapTime + time_freq() * 11 / 10 < CurTime )
			m_TouchJoyRightJumpPressed = false; // Disengage jetpack in 1 second after use
	}
	else
	{
		if( m_TouchJoyAimTapTime != CurTime )
			m_TouchJoyRightJumpPressed = false;
	}

	if( m_TouchJoyRightJumpPressed != oldTouchJoyRightJump )
		m_InputData.m_Jump = m_TouchJoyRightJumpPressed;

	bool FirePressed = AimPressed && distance(AimPos, m_TouchJoyAimAnchor) > TOUCHJOY_AIM_DEAD_ZONE;

	if( FirePressed != m_TouchJoyFirePressed )
	{
		if( m_InputData.m_Fire % 2 != FirePressed )
			m_InputData.m_Fire ++;
		m_TouchJoyFirePressed = FirePressed;
	}

	if (AimPressed)
		m_TouchJoyAimLastPos = AimPos;
}
#endif
