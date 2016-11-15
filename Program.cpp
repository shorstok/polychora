#include <d3d10.h>
#include <windows.h>
#include "resource.h"

#include "Viewpoint.h"
#include "Polychora.h"

#include "ScreenSpaceFxProcessor.h"

#include "DxManager.h"
#include "ScreenSaverModeHandler.h"

#include "PolychoraSettings.h"

#include <memory>

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL CALLBACK ConfigDialogProc(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam);

std::unique_ptr<CDxManager> dxMgr = nullptr;
std::unique_ptr<CScreenSaverModeHandler> screenSaverMode = nullptr;

POINT originalMousePos;

const int mouseMoveThrehold = 32;


//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
    UNREFERENCED_PARAMETER( hPrevInstance );
    UNREFERENCED_PARAMETER( lpCmdLine );
	DWORD tLast = 0;

	srand(timeGetTime());
	
	MSG msg = {0};

	screenSaverMode = std::unique_ptr<CScreenSaverModeHandler>(CScreenSaverModeHandler::FromCommandLine(lpCmdLine));

	if(!screenSaverMode->IsValid())
	{
			MessageBox(nullptr, (std::wstring(L"Invalid screensaver mode : ")+lpCmdLine).c_str(),L"X",MB_OK);
			return 0;
	}

	switch (screenSaverMode->CurrentMode())
	{
	case CScreenSaverModeHandler::Configure:
		DialogBox(hInstance, MAKEINTRESOURCE(DLG_CONFIG), screenSaverMode->parentWindowHandle, ConfigDialogProc);
		break;

	case  CScreenSaverModeHandler::Screensaver:
	case  CScreenSaverModeHandler::Preview:

		
		// Tell windows that a screen saver is running.
		UINT oldval;
		SystemParametersInfo(SPI_SCREENSAVERRUNNING,1,&oldval,0);

		if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
			return 0;

		if( FAILED( dxMgr->InitDevice() ) )
		{
			dxMgr->CleanupDevice();
			return 0;
		}

		GetCursorPos(&originalMousePos);

		// Main message loop

		while( WM_QUIT != msg.message )
		{
			if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
			else
			{
				//Don't slow down Windows UI on preview
				if((timeGetTime() - tLast) < 50 && screenSaverMode->CurrentMode() == CScreenSaverModeHandler::Preview)
					continue;

				tLast = timeGetTime();
				dxMgr->Render();
			}
		}

		dxMgr->CleanupDevice();

		SystemParametersInfo(SPI_SCREENSAVERRUNNING,0,&oldval,0);
		
		return ( int )msg.wParam;

	default:

		

		break;
	}


    return 0;
}

// Callback function for the dialog box.
BOOL CALLBACK ConfigDialogProc(HWND hwnd,UINT msg, WPARAM wParam, LPARAM lParam)
{
	// Get the application's instance.
	switch (msg)
	{
	case WM_INITDIALOG:
		{
			//Center dialog
			RECT rcDesk,rcClient;
			GetWindowRect(GetDesktopWindow(), &rcDesk);
			GetWindowRect(hwnd, &rcClient);

			auto w =rcClient.right - rcClient.left;
			auto h = rcClient.bottom - rcClient.top;

			MoveWindow(hwnd,rcDesk.right/2-w/2,rcDesk.bottom/2-h/2,w,h,false);

			CPolychoraSettings settings;

			// Get the values and propagate them
			// into the dialog box controls.

			CheckDlgButton(hwnd,IDC_USE_FXAA,settings.UseFXAA?1:0);
			CheckDlgButton(hwnd,IDC_USE_COMPLEX_ROT,settings.UseComplexRotations?1:0);
			CheckDlgButton(hwnd,IDC_SHOW_TEXT,settings.ShowText?1:0);

			for (auto & p : CPolychora::PolychoraGenerators)
				SendMessage(GetDlgItem(hwnd,IDC_POLYCHORON_SELECTOR),CB_ADDSTRING,0, reinterpret_cast<LPARAM>(p.first.c_str()));

			auto f = CPolychora::PolychoraGenerators.find(settings.Polychoron);

			if(f == CPolychora::PolychoraGenerators.end())
				f = CPolychora::PolychoraGenerators.begin();

			SendMessage(GetDlgItem(hwnd,IDC_POLYCHORON_SELECTOR),CB_SELECTSTRING,0, reinterpret_cast<LPARAM>(f->first.c_str()));
			
			return true;
		}
	case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			if( id==IDC_OK )
			{
				CPolychoraSettings settings;
				
				settings.UseFXAA = IsDlgButtonChecked(hwnd,IDC_USE_FXAA)!=0;
				settings.UseComplexRotations = IsDlgButtonChecked(hwnd,IDC_USE_COMPLEX_ROT)!=0;
				settings.ShowText = IsDlgButtonChecked(hwnd,IDC_SHOW_TEXT)!=0;

				auto csel = SendMessage(GetDlgItem(hwnd,IDC_POLYCHORON_SELECTOR),CB_GETCURSEL,0, 0);

				wchar_t buf[1024];

				SendMessage(GetDlgItem(hwnd,IDC_POLYCHORON_SELECTOR),CB_GETLBTEXT,csel, reinterpret_cast<LPARAM>(buf));

				settings.Polychoron = buf;

				settings.Save();
			}

			if( id==IDC_OK || id==IDCANCEL )	
				EndDialog(hwnd,id);
		} break;
	}
	return false;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{
    // Register class
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof( WNDCLASSEX );
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon( hInstance, ( LPCTSTR )IDI_TUTORIAL1 );
    wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
    wcex.hbrBackground = ( HBRUSH )( COLOR_WINDOW + 1 );
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = L"PolychoraWindowClass";
    wcex.hIconSm = LoadIcon( wcex.hInstance, ( LPCTSTR )IDI_POLYCHORA );
    if( !RegisterClassEx( &wcex ) )
    {
		return E_FAIL;
	}

    // Create window
    RECT rc;
		
//#define HDVIDEO_WINDOWSIZE
	
	HWND hwnd;
	DWORD style = WS_POPUP;
	
	if(screenSaverMode->parentWindowHandle == nullptr)
	{
		auto desk = GetDesktopWindow();

		GetWindowRect(desk,&rc);

		AdjustWindowRect( &rc, WS_POPUP, FALSE );
	}
	else
	{
		style=WS_CHILD | WS_VISIBLE;
		GetWindowRect(screenSaverMode->parentWindowHandle,&rc);
	}

//#define HDVIDEO_WINDOWSIZE

#ifdef HDVIDEO_WINDOWSIZE

	rc.left = rc.top = 10;

	rc.right = rc.left+1080;
	rc.bottom = rc.top+720;

#endif

	
	hwnd = CreateWindow( L"PolychoraWindowClass", L"Polychora", style,CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, screenSaverMode->parentWindowHandle, NULL, hInstance,NULL);

	if( !hwnd )
		return E_FAIL;

	dxMgr = std::unique_ptr<CDxManager>(new CDxManager(hwnd,screenSaverMode.get()));

    ShowWindow( hwnd, nCmdShow );

    return S_OK;
}



//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
    PAINTSTRUCT ps;
    HDC hdc;

    switch( message )
    {
        case WM_PAINT:
            hdc = BeginPaint( hWnd, &ps );
            EndPaint( hWnd, &ps );
            break;

        case WM_DESTROY:
            PostQuitMessage( 0 );
            break;

		case WM_MOUSEMOVE:
			{
				#ifndef _DEBUG
				if( screenSaverMode->CurrentMode() == CScreenSaverModeHandler::Screensaver )
				{
					POINT pt;
					GetCursorPos(&pt);

					if( abs(pt.x - originalMousePos.x) > mouseMoveThrehold ||
						abs(pt.y - originalMousePos.y) > mouseMoveThrehold)
					{
						PostQuitMessage( 0 );
					}
				}
				#endif
			} 
			break;

		case WM_SETCURSOR:
			{
				if( screenSaverMode->CurrentMode() == CScreenSaverModeHandler::Screensaver )
					SetCursor(NULL);
				else
					SetCursor(LoadCursor(NULL,IDC_ARROW));
			} 
			break;

		case WM_KEYDOWN:

			if(wParam == VkKeyScan('f'))
				dxMgr->ssfxp.UseFXAA = !dxMgr->ssfxp.UseFXAA;
			else if(wParam == VkKeyScan('b'))
				dxMgr->ssfxp.debugShowBrightpassOnly = !dxMgr->ssfxp.debugShowBrightpassOnly;
			else if(wParam == VkKeyScan('g'))
				dxMgr->ssfxp.debugShowBloomOnly= !dxMgr->ssfxp.debugShowBloomOnly;
			else if(wParam == VK_ESCAPE)
				PostQuitMessage(0);
#ifndef _DEBUG



			else if( screenSaverMode->CurrentMode() == CScreenSaverModeHandler::Screensaver )
				PostQuitMessage(0);

#endif
		
			break;

        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }

    return 0;
}





