
#include <base/math.h>
#include <base/system.h>

#include <engine/shared/config.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include "ui.h"

#if defined(__ANDROID__)
#include <SDL.h>
#include <SDL_screenkeyboard.h>
#endif

/********************************************************
 UI
*********************************************************/

CUI::CUI()
{
	m_pHotItem = 0;
	m_pActiveItem = 0;
	m_pLastActiveItem = 0;
	m_pBecommingHotItem = 0;

	m_MouseX = 0;
	m_MouseY = 0;
	m_MouseWorldX = 0;
	m_MouseWorldY = 0;
	m_MouseButtons = 0;
	m_LastMouseButtons = 0;

	m_Screen.x = 0;
	m_Screen.y = 0;
	m_Screen.w = 848.0f;
	m_Screen.h = 480.0f;
}

int CUI::Update(float Mx, float My, float Mwx, float Mwy, int Buttons)
{
	m_MouseX = Mx;
	m_MouseY = My;
	m_MouseWorldX = Mwx;
	m_MouseWorldY = Mwy;
	m_LastMouseButtons = m_MouseButtons;
	m_MouseButtons = Buttons;
	m_pHotItem = m_pBecommingHotItem;
	if(m_pActiveItem)
		m_pHotItem = m_pActiveItem;
	m_pBecommingHotItem = 0;
	return 0;
}

int CUI::MouseInside(const CUIRect *r)
{
	if(m_MouseX >= r->x && m_MouseX <= r->x+r->w && m_MouseY >= r->y && m_MouseY <= r->y+r->h)
		return 1;
	return 0;
}

void CUI::ConvertMouseMove(float *x, float *y)
{
#if !defined(__ANDROID__)
	float Fac = (float)(g_Config.m_UiMousesens)/g_Config.m_InpMousesens;
	*x = *x*Fac;
	*y = *y*Fac;
#endif
}

#if defined(__ANDROID__)
static void AndroidScreenKeysTwoJoysticks(SDL_Rect Buttons[], int ScreenW, int ScreenH)
{
	// First joystick to the left
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD].w = ScreenW / 2;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD].h = ScreenH * 0.8;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD].x = 0;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD].y =
		ScreenH - Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD].h;

	// Second joystick to the right
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].w = ScreenW / 2;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].h = ScreenH * 0.8;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].x = ScreenW / 2;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].y =
		ScreenH - Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].h;

	// Third joystick is a weapon selection bar, plus the scores/chat button
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].w = ScreenW;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].h = ScreenH * 0.2;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].x = 0;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].y = 0;

	// Drop mine
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0].h = ScreenH * 0.10;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0].w = ScreenH * 0.15;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0].x = ScreenW - Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0].w;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0].y = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].h;
	// Build turret
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1].h = ScreenH * 0.10;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1].w = ScreenH * 0.15;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1].x = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_0].x - Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1].w;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1].y = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].h;
	// Build flamer
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2].h = ScreenH * 0.10;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2].w = ScreenH * 0.15;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2].x = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_1].x - Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2].w;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2].y = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].h;
	// Build barrel
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_3].h = ScreenH * 0.10;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_3].w = ScreenH * 0.15;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_3].x = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_2].x - Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_3].w;
	Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_3].y = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD3].h;

	// Block events to joystick when a button is pressed
	SDL_ANDROID_SetScreenKeyboardPreventButtonOverlap(1);
}
#endif

void CUI::AndroidShowScreenKeys(bool shown)
{
#if defined(__ANDROID__)
	static bool ScreenKeyboardInitialized = false;
	static bool ScreenKeyboardShown = true;
	static SDL_Rect Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM];
	static SDL_Rect ButtonsInit[SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM];
	static SDL_Rect ButtonHidden = { 0, 0, 0, 0 };

	if( !ScreenKeyboardInitialized )
	{
		ScreenKeyboardInitialized = true;

		for( int i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++ )
			SDL_ANDROID_GetScreenKeyboardButtonPos( i, &ButtonsInit[i] );

		for( int i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++ )
			Buttons[i] = ButtonsInit[i];

		int ScreenW = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].x +
						Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].w;
		int ScreenH = Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].y +
						Buttons[SDL_ANDROID_SCREENKEYBOARD_BUTTON_DPAD2].h;

		AndroidScreenKeysTwoJoysticks(Buttons, ScreenW, ScreenH);
	}

	if( ScreenKeyboardShown == shown )
		return;
	ScreenKeyboardShown = shown;

	for( int i = 0; i < SDL_ANDROID_SCREENKEYBOARD_BUTTON_NUM; i++ )
		SDL_ANDROID_SetScreenKeyboardButtonPos( i, shown ? &Buttons[i] : &ButtonHidden );
#endif
}

void CUI::AndroidShowTextInput(const char *text)
{
#if defined(__ANDROID__)
	SDL_ANDROID_ToggleScreenKeyboardTextInput(text);
#endif
}

void CUI::AndroidTextInputHintMessage(const char *hintText)
{
#if defined(__ANDROID__)
	SDL_ANDROID_SetScreenKeyboardHintMesage(hintText);
#endif
}

void CUI::AndroidBlockAndGetTextInput(char *text, int textLength, const char *hintText)
{
#if defined(__ANDROID__)
	SDL_ANDROID_SetScreenKeyboardHintMesage(hintText);
	SDL_ANDROID_GetScreenKeyboardTextInput(text, textLength);
#endif
}

bool CUI::AndroidGetTextInput(char *text, int textLength)
{
#if defined(__ANDROID__)
	static char textBuf[1024];
	if (!SDL_IsScreenKeyboardShown(NULL))
	{
		str_copy(textBuf, text, sizeof(textBuf));
	}
	if (SDL_ANDROID_GetScreenKeyboardTextInputAsync(textBuf, sizeof(textBuf)) == SDL_ANDROID_TEXTINPUT_ASYNC_FINISHED)
	{
		str_copy(text, textBuf, textLength);
		return true;
	}
#endif
	return false;
}

bool CUI::AndroidTextInputShown()
{
#if defined(__ANDROID__)
	return SDL_IsScreenKeyboardShown(NULL);
#else
	return false;
#endif
}


CUIRect *CUI::Screen()
{
	float Aspect = Graphics()->ScreenAspect();
	float w, h;

	h = 600;
	w = Aspect*h;

	m_Screen.w = w;
	m_Screen.h = h;

	return &m_Screen;
}

float CUI::PixelSize()
{
	return Screen()->w/Graphics()->ScreenWidth();
}

void CUI::SetScale(float s)
{
	g_Config.m_UiScale = (int)(s*100.0f);
}

float CUI::Scale()
{
	return g_Config.m_UiScale/100.0f;
}

float CUIRect::Scale() const
{
	return g_Config.m_UiScale/100.0f;
}

void CUI::ClipEnable(const CUIRect *r)
{
	float XScale = Graphics()->ScreenWidth()/Screen()->w;
	float YScale = Graphics()->ScreenHeight()/Screen()->h;
	Graphics()->ClipEnable((int)(r->x*XScale), (int)(r->y*YScale), (int)(r->w*XScale), (int)(r->h*YScale));
}

void CUI::ClipDisable()
{
	Graphics()->ClipDisable();
}

void CUIRect::HSplitMid(CUIRect *pTop, CUIRect *pBottom) const
{
	CUIRect r = *this;
	float Cut = r.h/2;

	if(pTop)
	{
		pTop->x = r.x;
		pTop->y = r.y;
		pTop->w = r.w;
		pTop->h = Cut;
	}

	if(pBottom)
	{
		pBottom->x = r.x;
		pBottom->y = r.y + Cut;
		pBottom->w = r.w;
		pBottom->h = r.h - Cut;
	}
}

void CUIRect::HSplitTop(float Cut, CUIRect *pTop, CUIRect *pBottom) const
{
	CUIRect r = *this;
	Cut *= Scale();

	if (pTop)
	{
		pTop->x = r.x;
		pTop->y = r.y;
		pTop->w = r.w;
		pTop->h = Cut;
	}

	if (pBottom)
	{
		pBottom->x = r.x;
		pBottom->y = r.y + Cut;
		pBottom->w = r.w;
		pBottom->h = r.h - Cut;
	}
}

void CUIRect::HSplitBottom(float Cut, CUIRect *pTop, CUIRect *pBottom) const
{
	CUIRect r = *this;
	Cut *= Scale();

	if (pTop)
	{
		pTop->x = r.x;
		pTop->y = r.y;
		pTop->w = r.w;
		pTop->h = r.h - Cut;
	}

	if (pBottom)
	{
		pBottom->x = r.x;
		pBottom->y = r.y + r.h - Cut;
		pBottom->w = r.w;
		pBottom->h = Cut;
	}
}


void CUIRect::VSplitMid(CUIRect *pLeft, CUIRect *pRight) const
{
	CUIRect r = *this;
	float Cut = r.w/2;
//	Cut *= Scale();

	if (pLeft)
	{
		pLeft->x = r.x;
		pLeft->y = r.y;
		pLeft->w = Cut;
		pLeft->h = r.h;
	}

	if (pRight)
	{
		pRight->x = r.x + Cut;
		pRight->y = r.y;
		pRight->w = r.w - Cut;
		pRight->h = r.h;
	}
}

void CUIRect::VSplitLeft(float Cut, CUIRect *pLeft, CUIRect *pRight) const
{
	CUIRect r = *this;
	Cut *= Scale();

	if (pLeft)
	{
		pLeft->x = r.x;
		pLeft->y = r.y;
		pLeft->w = Cut;
		pLeft->h = r.h;
	}

	if (pRight)
	{
		pRight->x = r.x + Cut;
		pRight->y = r.y;
		pRight->w = r.w - Cut;
		pRight->h = r.h;
	}
}

void CUIRect::VSplitRight(float Cut, CUIRect *pLeft, CUIRect *pRight) const
{
	CUIRect r = *this;
	Cut *= Scale();

	if (pLeft)
	{
		pLeft->x = r.x;
		pLeft->y = r.y;
		pLeft->w = r.w - Cut;
		pLeft->h = r.h;
	}

	if (pRight)
	{
		pRight->x = r.x + r.w - Cut;
		pRight->y = r.y;
		pRight->w = Cut;
		pRight->h = r.h;
	}
}

void CUIRect::Margin(float Cut, CUIRect *pOtherRect) const
{
	CUIRect r = *this;
	Cut *= Scale();

	pOtherRect->x = r.x + Cut;
	pOtherRect->y = r.y + Cut;
	pOtherRect->w = r.w - 2*Cut;
	pOtherRect->h = r.h - 2*Cut;
}

void CUIRect::VMargin(float Cut, CUIRect *pOtherRect) const
{
	CUIRect r = *this;
	Cut *= Scale();

	pOtherRect->x = r.x + Cut;
	pOtherRect->y = r.y;
	pOtherRect->w = r.w - 2*Cut;
	pOtherRect->h = r.h;
}

void CUIRect::HMargin(float Cut, CUIRect *pOtherRect) const
{
	CUIRect r = *this;
	Cut *= Scale();

	pOtherRect->x = r.x;
	pOtherRect->y = r.y + Cut;
	pOtherRect->w = r.w;
	pOtherRect->h = r.h - 2*Cut;
}

int CUI::DoButtonLogic(const void *pID, const char *pText, int Checked, const CUIRect *pRect)
{
	// logic
	int ReturnValue = 0;
	int Inside = MouseInside(pRect);
	static int ButtonUsed = 0;

	if(ActiveItem() == pID)
	{
		if(!MouseButton(ButtonUsed))
		{
			if(Inside && Checked >= 0)
				ReturnValue = 1+ButtonUsed;
			SetActiveItem(0);
		}
	}
	else if(HotItem() == pID)
	{
		if(MouseButton(0))
		{
			SetActiveItem(pID);
			ButtonUsed = 0;
		}

		if(MouseButton(1))
		{
			SetActiveItem(pID);
			ButtonUsed = 1;
		}
	}

	if(Inside)
		SetHotItem(pID);

	return ReturnValue;
}
/*
int CUI::DoButton(const void *id, const char *text, int checked, const CUIRect *r, ui_draw_button_func draw_func, const void *extra)
{
	// logic
	int ret = 0;
	int inside = ui_MouseInside(r);
	static int button_used = 0;

	if(ui_ActiveItem() == id)
	{
		if(!ui_MouseButton(button_used))
		{
			if(inside && checked >= 0)
				ret = 1+button_used;
			ui_SetActiveItem(0);
		}
	}
	else if(ui_HotItem() == id)
	{
		if(ui_MouseButton(0))
		{
			ui_SetActiveItem(id);
			button_used = 0;
		}

		if(ui_MouseButton(1))
		{
			ui_SetActiveItem(id);
			button_used = 1;
		}
	}

	if(inside)
		ui_SetHotItem(id);

	if(draw_func)
		draw_func(id, text, checked, r, extra);
	return ret;
}*/

void CUI::DoLabel(const CUIRect *r, const char *pText, float Size, int Align, int MaxWidth)
{
	// TODO: FIX ME!!!!
	//Graphics()->BlendNormal();
	if(Align == 0)
	{
		float tw = TextRender()->TextWidth(0, Size, pText, MaxWidth);
		TextRender()->Text(0, r->x + r->w/2-tw/2, r->y - Size/10, Size, pText, MaxWidth);
	}
	else if(Align < 0)
		TextRender()->Text(0, r->x, r->y - Size/10, Size, pText, MaxWidth);
	else if(Align > 0)
	{
		float tw = TextRender()->TextWidth(0, Size, pText, MaxWidth);
		TextRender()->Text(0, r->x + r->w-tw, r->y - Size/10, Size, pText, MaxWidth);
	}
}

void CUI::DoLabelScaled(const CUIRect *r, const char *pText, float Size, int Align, int MaxWidth)
{
	DoLabel(r, pText, Size*Scale(), Align, MaxWidth);
}
