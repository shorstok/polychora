#pragma once

#include <d3d10.h>
#include <windows.h>
#include "resource.h"

#include <string>

class CScreenSaverModeHandler
{
public:

	enum Mode
	{
		Preview,
		Screensaver,
		Configure,
		Password
	};

	CScreenSaverModeHandler(void);
	~CScreenSaverModeHandler(void);

	bool IsValid();

	Mode CurrentMode();

public:

	static CScreenSaverModeHandler * FromCommandLine(LPWSTR cmdLine);

	HWND parentWindowHandle;

protected:

	CScreenSaverModeHandler(Mode mode,std::wstring token);

	static CScreenSaverModeHandler * InvalidMode();

	bool isValid;
	Mode mode;
};

