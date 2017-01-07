#ifndef GAME_CLIENT_COMPONENTS_WEAPONBAR_H
#define GAME_CLIENT_COMPONENTS_WEAPONBAR_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/customstuff.h>

class CWeaponbar : public CComponent
{
	bool m_Touching;
	vec2 m_InitialPos;
	vec2 m_Pos;
	int m_LastPicked;
	bool m_CanDrop;
	bool m_ScoreboardShown;
	bool m_BlockTouchEvents;

	static void ConDropMine(IConsole::IResult *pResult, void *pUserData);
	static void ConBuildTurret(IConsole::IResult *pResult, void *pUserData);
	static void ConBuildFlamer(IConsole::IResult *pResult, void *pUserData);
	static void ConBuildBarrel(IConsole::IResult *pResult, void *pUserData);

	void BuildProcess(int keypress);

public:
	CWeaponbar();

	virtual void OnReset();
	virtual void OnRender();
	virtual void OnRelease();
	virtual void OnConsoleInit();

	void OnFingerTouch(vec2 posNormalized);
	void OnFingerRelease();

	int ScreenToWeapon(float pos, float width) const;

	static const int WeaponOrder[];
};

#endif
