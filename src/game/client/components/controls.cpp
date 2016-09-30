

#include <base/math.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>

#include <game/collision.h>
#include <game/client/gameclient.h>
#include <game/client/component.h>
#include <game/client/components/picker.h>
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
		SECOND_RIGHT_JOYSTICK_X = 20, SECOND_RIGHT_JOYSTICK_Y = 21,
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

	m_TouchJoyRunPressed = false;
	m_TouchJoyRunTapTime = 0;
	m_TouchJoyRunAnchor = ivec2(0,0);
	m_TouchJoyAimPressed = false;
	m_TouchJoyAimAnchor = ivec2(0,0);
	m_TouchJoyAimPrev = ivec2(0,0);
	m_TouchJoyAimTapTime = 0;
	m_TouchJoyFirePressed = false;
	m_TouchJoyWeaponSelected = false;
	for( int i = 0; i < NUM_WEAPONS; i++ )
		m_AmmoCount[i] = 0;
	m_OldMouseX = m_OldMouseY = 0.0f;
}

void CControls::OnRelease()
{
	OnReset();
}

void CControls::OnPlayerDeath()
{
	m_LastData.m_WantedWeapon = m_InputData.m_WantedWeapon = 0;
	for( int i = 0; i < NUM_WEAPONS; i++ )
		m_AmmoCount[i] = 0;
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
	int m_Value;
};

static void ConKeyInputSet(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	if(pResult->GetInteger(0))
		*pSet->m_pVariable = pSet->m_Value;
}

static void ConKeyInputNextPrevWeapon(IConsole::IResult *pResult, void *pUserData)
{
	CInputSet *pSet = (CInputSet *)pUserData;
	ConKeyInputCounter(pResult, pSet->m_pVariable);
	pSet->m_pControls->m_InputData.m_WantedWeapon = 0;
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

	// gamepad
	Console()->Register("+gamepadleft", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionLeft, "Move left");
	Console()->Register("+gamepadright", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputDirectionRight, "Move right");
	Console()->Register("+gamepadjump", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Jump, "Jump");
	Console()->Register("+gamepadturbo", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Turbo");
	//Console()->Register("+gamepadpicker", "", CFGFLAG_CLIENT, ConKeyInputState, &m_InputData.m_Hook, "Weapon picker");
	Console()->Register("+gamepadfire", "", CFGFLAG_CLIENT, ConKeyInputCounter, &m_InputData.m_Fire, "Fire");

	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 1}; Console()->Register("+weapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to primary weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 2}; Console()->Register("+weapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to secondary weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 3}; Console()->Register("+weapon3", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to tool"); }

	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 1}; Console()->Register("+gamepadweapon1", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to primary weapon"); }
	{ static CInputSet s_Set = {this, &m_InputData.m_WantedWeapon, 2}; Console()->Register("+gamepadweapon2", "", CFGFLAG_CLIENT, ConKeyInputSet, (void *)&s_Set, "Switch to secondary weapon"); }

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
		
		CustomStuff()->m_LocalWeapons |= 1 << (pMsg->m_Weapon);
		CustomStuff()->m_WeaponpickTimer = 1.0f;
		CustomStuff()->m_WeaponpickWeapon = pMsg->m_Weapon;
		CustomStuff()->m_LastWeaponPicked = false;
		m_AmmoCount[pMsg->m_Weapon%NUM_WEAPONS] = 10; // TODO: move ammo count for inactive weapons into the network protocol
		// Does not quite work yet
		/*
		if(g_Config.m_ClAutoswitchWeapons)
		{
			char aBuf[32];
			str_format(aBuf, sizeof(aBuf), "weaponpick %d", pMsg->m_Weapon-1);
			Console()->ExecuteLine(aBuf);
		}
		*/
	}
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
	bool FireWasPressed = false;

	if( m_TouchJoy && !m_UsingGamepad )
		TouchscreenInput(&FireWasPressed);

	if( m_Gamepad )
		GamepadInput();

	if( g_Config.m_ClAutoswitchWeaponsOutOfAmmo )
		AutoswitchWeaponsOutOfAmmo(FireWasPressed);
#endif

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

#if defined(__ANDROID__)
void CControls::TouchscreenInput(bool *FireWasPressed)
{
	enum {
		TOUCHJOY_DEAD_ZONE = 65536 / 40,
		TOUCHJOY_AIM_DEAD_ZONE = 65536 / 10,
	};

	int64 CurTime = time_get();

	// Get input from the left joystick
	int RunX = SDL_JoystickGetAxis(m_TouchJoy, LEFT_JOYSTICK_X);
	int RunY = SDL_JoystickGetAxis(m_TouchJoy, LEFT_JOYSTICK_Y);
	bool RunPressed = (RunX != 0 || RunY != 0);
	// Get input from the right joystick
	int AimX = SDL_JoystickGetAxis(m_TouchJoy, RIGHT_JOYSTICK_X);
	int AimY = SDL_JoystickGetAxis(m_TouchJoy, RIGHT_JOYSTICK_Y);
	bool AimPressed = (AimX != 0 || AimY != 0);

	// Process left joystick
	if( m_TouchJoyRunPressed != RunPressed )
	{
		if( RunPressed )
		{
			// Tap to jetpack, and do not reset the anchor coordinates, if tapped under 300ms
			if( m_TouchJoyRunTapTime + time_freq() / 3 > CurTime && distance(ivec2(RunX, RunY), m_TouchJoyRunAnchor) < TOUCHJOY_DEAD_ZONE )
				m_InputData.m_Hook = 1;
			else
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
		// Change your facing direction if not aiming
		if( !AimPressed )
		{
			if( m_MousePos.x == 0 )
				m_MousePos.x = 1;
			if( m_MousePos.x < 0 && m_InputDirectionRight || m_MousePos.x > 0 && m_InputDirectionLeft )
				m_MousePos.x = -m_MousePos.x;
		}
		// Move the anchor if we move the finger too much
		if( m_TouchJoyRunAnchor.x - RunX < -TOUCHJOY_DEAD_ZONE * 3 )
			m_TouchJoyRunAnchor.x = RunX - TOUCHJOY_DEAD_ZONE * 3;
		if( m_TouchJoyRunAnchor.x - RunX > TOUCHJOY_DEAD_ZONE * 3 )
			m_TouchJoyRunAnchor.x = RunX + TOUCHJOY_DEAD_ZONE * 3;
		if( m_TouchJoyRunTapTime + time_freq() * 1.1f < CurTime )
			m_InputData.m_Hook = 0; // Disengage jetpack in 1 second after use
	}

	// Move 100ms in the same direction, to prevent speed drop when tapping
	if( !RunPressed && m_TouchJoyRunTapTime + time_freq() / 10 < CurTime )
	{
		m_InputDirectionLeft = 0;
		m_InputDirectionRight = 0;
	}

	// Process right joystick
	if( !AimPressed )
	{
		AimX = m_TouchJoyAimPrev.x;
		AimY = m_TouchJoyAimPrev.y;
	}

	if( AimPressed != m_TouchJoyAimPressed )
	{
		if( !AimPressed )
		{
			SDL_Rect joypos;
			SDL_ANDROID_GetScreenKeyboardButtonPos( SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2, &joypos );
			m_InputData.m_Jump = 0;
			if ( !(m_TouchJoyWeaponSelected && m_TouchJoyAimTapTime + time_freq() / 3 > CurTime) )
			{
				this->Picker()->SetDrawPos(vec2(joypos.x + (AimX + 32767) * joypos.w / 65536, joypos.y + (AimY + 32767) * joypos.h / 65536));
				this->Picker()->OpenPicker();
			}
			m_TouchJoyWeaponSelected = false;
		}
		else
		{
			if( m_TouchJoyAimTapTime + time_freq() / 2 >= CurTime )
			{
				if( distance(ivec2(AimX, AimY), m_TouchJoyAimAnchor) < TOUCHJOY_AIM_DEAD_ZONE / 2 )
				{
					m_InputData.m_Jump = 1;
					this->Picker()->ClosePicker();
				}
				else
				{
					this->Picker()->OnMouseMove((AimX - m_TouchJoyAimAnchor.x) * joypos.w / 32767, (AimY - m_TouchJoyAimAnchor.y) * joypos.h / 32767);
					m_TouchJoyWeaponSelected = true;
				}
			}
		}
		m_TouchJoyAimPressed = AimPressed;
		m_TouchJoyAimAnchor = ivec2(AimX, AimY);
		m_TouchJoyAimTapTime = CurTime;
	}

	if( AimPressed )
	{
		m_MousePos = vec2(AimX - m_TouchJoyAimAnchor.x, AimY - m_TouchJoyAimAnchor.y) / 30;
		ClampMousePos();
		if( m_TouchJoyAimTapTime + time_freq() * 1.1f < CurTime )
			m_InputData.m_Jump = 0; // Disengage jetpack in 1 second after use
		if( m_TouchJoyAimTapTime != CurTime )
			this->Picker()->ClosePicker(); // We need to call onRender() after setting picker coordinates, so call it in another frame
	}

	if( !AimPressed && m_TouchJoyAimTapTime + time_freq() / 2 < CurTime )
		this->Picker()->ClosePicker();

	bool FirePressed = distance(ivec2(AimX, AimY), m_TouchJoyAimAnchor) > TOUCHJOY_AIM_DEAD_ZONE;

	if( FirePressed != m_TouchJoyFirePressed )
	{
		if( m_InputData.m_Fire % 2 != FirePressed )
			m_InputData.m_Fire ++;
		if( !FirePressed )
			*FireWasPressed = true;
		m_TouchJoyFirePressed = FirePressed;
	}

	m_TouchJoyAimPrev = ivec2(AimX, AimY);
}

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

void CControls::AutoswitchWeaponsOutOfAmmo(bool FireWasPressed)
{
	if( ! m_pClient->m_Snap.m_pLocalCharacter )
		return;

	// Keep track of ammo count, we know weapon ammo only when we switch to that weapon, this is tracked on server and protocol does not track that
	m_AmmoCount[m_pClient->m_Snap.m_pLocalCharacter->m_Weapon%NUM_WEAPONS] = m_pClient->m_Snap.m_pLocalCharacter->m_AmmoCount;
	// Autoswitch weapon if we're out of ammo
	
	if( (m_InputData.m_Fire % 2 != 0 || FireWasPressed) &&
		m_pClient->m_Snap.m_pLocalCharacter->m_AmmoCount == 0 &&
		m_pClient->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_HAMMER &&
		m_pClient->m_Snap.m_pLocalCharacter->m_Weapon != WEAPON_TOOL )
	{
		int w;
		for( w = NUM_WEAPONS - 1; w > WEAPON_HAMMER; w-- )
		{
			if( w == m_pClient->m_Snap.m_pLocalCharacter->m_Weapon )
				continue;
			if( m_AmmoCount[w] > 0 )
				break;
		}
		//dbg_msg("controls", "Out of ammo - selected weapon %d ammo %d", w, m_AmmoCount[w]);
		if( w != m_pClient->m_Snap.m_pLocalCharacter->m_Weapon )
		{
			//m_InputData.m_WantedWeapon = w; // This changes weapon group instead
			CNetMsg_Cl_SelectWeapon Msg;
			Msg.m_Weapon = w+1;
			Msg.m_Group = 0;
			Client()->SendPackMsg(&Msg, MSGFLAG_VITAL);
		}
	}
}

#endif
