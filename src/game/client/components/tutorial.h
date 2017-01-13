#ifndef GAME_CLIENT_COMPONENTS_TUTORIAL_H
#define GAME_CLIENT_COMPONENTS_TUTORIAL_H
#include <base/vmath.h>
#include <game/client/component.h>
#include <game/client/customstuff.h>

class CTutorial : public CComponent
{
	bool m_Active;
	int64 m_Time;
	static float m_Duration;

	float Fade(float Seconds, float Middle);
	void DrawTextFade(float Seconds, float Tick, float x, float y, const char *text);

public:
	CTutorial();

	virtual void OnRender();
};

#endif
