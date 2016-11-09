

#ifndef GAME_CLIENT_COMPONENTS_CONTROLS_H
#define GAME_CLIENT_COMPONENTS_CONTROLS_H
#include <base/vmath.h>
#include <base/system.h>
#include <game/client/component.h>

typedef struct _SDL_Joystick SDL_Joystick;

class CControls : public CComponent
{
	void TouchscreenInput();
	void WeaponBarInput();
	void GamepadInput();
	void AutoswitchWeaponsOutOfAmmo();

public:
	vec2 m_MousePos;
	vec2 m_TargetPos;

	float m_OldMouseX;
	float m_OldMouseY;

	SDL_Joystick *m_TouchJoy;

	bool m_TouchJoyRunPressed;
	ivec2 m_TouchJoyRunAnchor;
	//ivec2 m_TouchJoyRunLastPos;
	int64 m_TouchJoyRunTapTime;

	bool m_TouchJoyAimPressed;
	ivec2 m_TouchJoyAimAnchor;
	ivec2 m_TouchJoyAimLastPos;
	int64 m_TouchJoyAimTapTime;
	bool m_TouchJoyFirePressed;

	bool m_TouchJoyRightJumpPressed;

	bool m_TouchJoyWeaponbarPressed;

	int m_WeaponIdxOutOfAmmo;

	SDL_Joystick *m_Gamepad;
	bool m_UsingGamepad;

	SDL_Joystick *m_Accelerometer;

	int m_AmmoCount[NUM_WEAPONS];

	CNetObj_PlayerInput m_InputData;
	CNetObj_PlayerInput m_LastData;
	int m_InputDirectionLeft;
	int m_InputDirectionRight;
	
	// weapon change from picker
	int m_PickedWeapon;
	
	// signal wanted weapon you don't have to hud
	int m_SignalWeapon;
	
	// input
	int m_Build;
	bool m_BuildReleased;
	
	bool m_BuildMode;
	int m_SelectedBuilding;
	
	// switch back to shooting
	int m_LastWeapon;

	CControls();

	virtual void OnReset();
	virtual void OnRelease();
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void *pRawMsg);
	virtual bool OnMouseMove(float x, float y);
	virtual void OnConsoleInit();
	virtual void OnPlayerDeath();

	int SnapInput(int *pData);
	void ClampMousePos();
};
#endif
