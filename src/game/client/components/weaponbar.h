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

public:
	CWeaponbar();

	virtual void OnReset();
	virtual void OnRender();
	virtual void OnRelease();

	void OnFingerTouch(vec2 posNormalized);
	void OnFingerRelease();

	int ScreenToWeapon(float pos, float width) const;
};

#endif
