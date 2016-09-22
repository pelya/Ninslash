

#include <base/math.h>

#include <engine/demo.h>
#include <engine/keys.h>
#include <engine/graphics.h>
#include <engine/textrender.h>
#include <engine/storage.h>

#include <game/client/render.h>
#include <game/client/gameclient.h>
#include <game/localization.h>

#include <game/client/ui.h>

#include <game/generated/client_data.h>
#include <engine/shared/config.h>

#include "menus.h"

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


void CMenus::ServerCreatorInit()
{
}

void CMenus::ServerCreatorProcess(CUIRect MainView)
{
	// background
	RenderTools()->DrawUIRect(&MainView, ms_ColorTabbarActive, CUI::CORNER_ALL, 20.0f);
	MainView.Margin(20.0f, &MainView);

	MainView.HSplitTop(10, 0, &MainView);
	CUIRect MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, Localize("Local server"), 20.0f, 0);

	static int64 LastUpdateTime = 0;
	static bool ServerRunning = false;
	static bool ServerStarting = false;
	static bool AddingBot = false;
	if (time_get() / time_freq() > LastUpdateTime + 3)
	{
		LastUpdateTime = time_get() / time_freq();
		int status = system("$SECURE_STORAGE_DIR/busybox sh -c 'ps | grep ninslash_srv'");
		ServerRunning = WEXITSTATUS(status) == 0;
		ServerStarting = false;
		AddingBot = false;
	}

	MainView.HSplitTop(30, 0, &MainView);
	MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, AddingBot ? Localize("Adding a bot to server") :
								 ServerStarting ? Localize("Server is starting") :
								 ServerRunning ? Localize("Server is running") :
								 Localize("Server stopped"), 20.0f, 0);

	MainView.HSplitTop(30, 0, &MainView);

	CUIRect Button;

	MainView.VSplitLeft(50, 0, &Button);
	Button.h = 50;
	Button.w = 300;
	static int s_StartServerButton = 0;
	if(DoButton_Menu(&s_StartServerButton, Localize("Start server"), 0, &Button))
	{
		system("$SECURE_STORAGE_DIR/ninslash_srv -f \"$UNSECURE_STORAGE_DIR/example configs/dm-autoexec.cfg\" >/dev/null 2>&1 &");
		LastUpdateTime = time_get() / time_freq(); // We do not actually ping the server, just wait 3 seconds
		ServerStarting = true;
	}

	MainView.VSplitRight(350, 0, &Button);
	Button.h = 50;
	Button.w = 300;
	static int s_StopServerButton = 0;
	if(DoButton_Menu(&s_StopServerButton, Localize("Stop server"), 0, &Button))
	{
		system("$SECURE_STORAGE_DIR/busybox killall ninslash_srv");
		LastUpdateTime = time_get() / time_freq() - 2;
	}

	MainView.HSplitTop(60, 0, &MainView);

	MainView.VSplitLeft(50, 0, &Button);
	Button.h = 50;
	Button.w = 300;
	static int s_EditServerConfigButton = 0;
	if(DoButton_Menu(&s_EditServerConfigButton, Localize("Edit server config"), 0, &Button))
	{
		system("am start --user -3 -a android.intent.action.VIEW -t 'text/*' -d \"file://$UNSECURE_STORAGE_DIR/example configs/dm-autoexec.cfg\"");
	}

	if(ServerRunning)
	{
		MainView.VSplitRight(350, 0, &Button);
		Button.h = 50;
		Button.w = 300;
		static int s_JoinServerButton = 0;
		if(DoButton_Menu(&s_JoinServerButton, Localize("Join server"), 0, &Button))
		{
			strcpy(g_Config.m_UiServerAddress, "127.0.0.1");
			Client()->Connect(g_Config.m_UiServerAddress);
		}
	}

	MainView.HSplitTop(60, 0, &MainView);
	MsgBox = MainView;
#if 0
	if(ServerRunning)
	{
		UI()->DoLabelScaled(&MsgBox, Localize("Add bots to server"), 20.0f, 0);
	}

	MainView.HSplitTop(30, 0, &MainView);

	if(ServerRunning)
	{
		MainView.VSplitRight(50, 0, &Button);
		int width = Button.x;
		MainView.VSplitLeft(50, 0, &Button);
		width -= Button.x;
		width /= 14;

		Button.h = 50;
		Button.w = width * 2;
		static int s_AddBot1 = 0;
		if(DoButton_Menu(&s_AddBot1, Localize("Harmless"), 0, &Button))
		{
			system("$SECURE_STORAGE_DIR/teebot 'bot_skill 0' >/dev/null 2>&1 &");
			AddingBot = true;
			LastUpdateTime = time_get() / time_freq();
		}

		MainView.VSplitLeft(50 + width * 3, 0, &Button);
		Button.h = 50;
		Button.w = width * 2;
		static int s_AddBot2 = 0;
		if(DoButton_Menu(&s_AddBot2, Localize("Clumsy"), 0, &Button))
		{
			system("$SECURE_STORAGE_DIR/teebot 'bot_skill 1' >/dev/null 2>&1 &");
			AddingBot = true;
			LastUpdateTime = time_get() / time_freq();
		}

		MainView.VSplitLeft(50 + width * 6, 0, &Button);
		Button.h = 50;
		Button.w = width * 2;
		static int s_AddBot3 = 0;
		if(DoButton_Menu(&s_AddBot3, Localize("Skilled"), 0, &Button))
		{
			system("$SECURE_STORAGE_DIR/teebot 'bot_skill 2' >/dev/null 2>&1 &");
			AddingBot = true;
			LastUpdateTime = time_get() / time_freq();
		}

		MainView.VSplitLeft(50 + width * 9, 0, &Button);
		Button.h = 50;
		Button.w = width * 2;
		static int s_AddBot4 = 0;
		if(DoButton_Menu(&s_AddBot4, Localize("Dominating"), 0, &Button))
		{
			system("$SECURE_STORAGE_DIR/teebot 'bot_skill 3' >/dev/null 2>&1 &");
			AddingBot = true;
			LastUpdateTime = time_get() / time_freq();
		}

		MainView.VSplitLeft(50 + width * 12, 0, &Button);
		Button.h = 50;
		Button.w = width * 2;
		static int s_AddBot5 = 0;
		if(DoButton_Menu(&s_AddBot5, Localize("Impossible"), 0, &Button))
		{
			system("$SECURE_STORAGE_DIR/teebot 'bot_skill 4' >/dev/null 2>&1 &");
			AddingBot = true;
			LastUpdateTime = time_get() / time_freq();
		}
	}
#endif
	MainView.HSplitTop(60, 0, &MainView);
	MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, Localize("If you don't have Wi-Fi connection, go to Android Settings, and enable Wi-Fi hotspot"), 20.0f, 0);
	MainView.HSplitTop(30, 0, &MainView);
	MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, Localize("in Tethering & Hotspot menu, then ask other players to connect to your Wi-Fi network,"), 20.0f, 0);
	MainView.HSplitTop(30, 0, &MainView);
	MsgBox = MainView;
	UI()->DoLabelScaled(&MsgBox, Localize("open Ninslash, select LAN menu, and connect to your server."), 20.0f, 0);

	MainView.HSplitTop(30, 0, &MainView);
	MainView.VMargin(50.0f, &Button);
	Button.h = 50;
	static int s_ShareAppButton = 0;
	if(DoButton_Menu(&s_ShareAppButton, Localize("Share Ninslash over Bluetooth to other devices"), 0, &Button))
	{
		system("$SECURE_STORAGE_DIR/busybox cp -f $ANDROID_MY_OWN_APP_FILE $UNSECURE_STORAGE_DIR/Ninslash.apk");
		system("am start --user -3 -a android.intent.action.SEND -t application/vnd.android.package-archive "
				"--eu android.intent.extra.STREAM file://$UNSECURE_STORAGE_DIR/Ninslash.apk");
	}
}
