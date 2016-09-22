

#ifndef GAME_CLIENT_COMPONENTS_CONTROLS_H
#define GAME_CLIENT_COMPONENTS_CONTROLS_H
#include <base/vmath.h>
#include <base/system.h>
#include <game/client/component.h>

typedef struct _SDL_Joystick SDL_Joystick;

class CControls : public CComponent
{
	void TouchscreenInput(int64 CurTime, bool *FireWasPressed);

public:
	vec2 m_MousePos;
	vec2 m_TargetPos;
	float m_OldMouseX;
	float m_OldMouseY;
	SDL_Joystick *m_Joystick;
	bool m_JoystickFirePressed;
	bool m_JoystickRunPressed;
	int64 m_JoystickTapTime;

	SDL_Joystick *m_Gamepad;
	bool m_UsingGamepad;

	SDL_Joystick *m_Accelerometer;

	int m_AmmoCount[NUM_WEAPONS];

	CNetObj_PlayerInput m_InputData;
	CNetObj_PlayerInput m_LastData;
	int m_InputDirectionLeft;
	int m_InputDirectionRight;

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
