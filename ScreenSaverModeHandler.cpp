#include "ScreenSaverModeHandler.h"

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <vector>


CScreenSaverModeHandler::CScreenSaverModeHandler(void) : isValid(false),parentWindowHandle(nullptr)
{
}

CScreenSaverModeHandler::CScreenSaverModeHandler( Mode _mode, std::wstring token ) : mode(_mode), isValid(true), parentWindowHandle(nullptr)
{
	auto idx = token.find(L":");

	if(token.length()<1)
		parentWindowHandle = nullptr;
	else if (std::wstring::npos == idx)
		parentWindowHandle = GetForegroundWindow();
	else
	{
		auto handleString = token.substr(idx+1,std::wstring::npos);
		parentWindowHandle = (HWND)_wtoi(handleString.c_str());
	}
}


CScreenSaverModeHandler::~CScreenSaverModeHandler(void)
{
}

CScreenSaverModeHandler * CScreenSaverModeHandler::FromCommandLine( LPWSTR cmdLine )
{
	std::vector<std::wstring> elems;

	std::wstringstream ss(cmdLine);
	std::wstring item;
	while (std::getline(ss, item, L' ')) 
		elems.push_back(item);

	if(elems.empty() || elems[0].length() < 2)
		return InvalidMode();

	CScreenSaverModeHandler * ret = new CScreenSaverModeHandler();

	switch(elems[0][1])
	{
		case L'p':	case L'P':
		case L'l':	case L'L':
			return new CScreenSaverModeHandler(Preview,elems.size()>1?elems[0]+L":"+elems[1]:elems[0]);

		case L's':	case L'S':
			return new CScreenSaverModeHandler(Screensaver,L"");

		case L'c':	case L'C':
			return new CScreenSaverModeHandler(Configure,elems[0]);

		case L'a':	case L'A':
			return new CScreenSaverModeHandler(Password,elems[0]);
	}

	return InvalidMode();
}

bool CScreenSaverModeHandler::IsValid()
{
	return isValid;
}

CScreenSaverModeHandler * CScreenSaverModeHandler::InvalidMode()
{
	return new CScreenSaverModeHandler();
}

CScreenSaverModeHandler::Mode CScreenSaverModeHandler::CurrentMode()
{
	return mode;
}
