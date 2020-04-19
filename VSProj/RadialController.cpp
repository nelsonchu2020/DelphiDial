#include "stdafx.h"
#include <tchar.h>
#include <StrSafe.h>
//#include <wrl\implements.h>
//#include <wrl\module.h>
//#include <wrl\event.h>
//#include <roapi.h>
//#include <wrl.h>
#include "DeviceListener.h"
//#include <ShellScalingApi.h>

//#define CLASSNAME L"Radial Device Controller"
//#define BUFFER_SIZE 2000

//LRESULT CALLBACK WindowProc(
//    __in HWND hWindow,
//    __in UINT uMsg,
//    __in WPARAM wParam,
//    __in LPARAM lParam);
//
//HANDLE console;
//WCHAR windowClass[MAX_PATH];

void PrintMsg(WCHAR *msg)
{
    //size_t textLength;
    //DWORD charsWritten;
    //StringCchLength(msg, BUFFER_SIZE, &textLength);
    //WriteConsole(console, msg, (DWORD)textLength, &charsWritten, NULL);
}

void PrintStartupMessage()
{
    //wchar_t message[2000];

    //swprintf_s(message, 2000, L"Press F10 to begin...\n");
    //PrintMsg(message);
}

#define DllImport   extern "C" __declspec( dllimport )
#define DllExport   extern "C" __declspec( dllexport )    


DllExport int DoMath(int a, int b) {  // for testing if DLL export works only
	return a + b;
}

DeviceListener *listener = nullptr;

DllExport HRESULT Init(HWND hwnd, bool MultiThread)
{
	HRESULT hr;
	if (!listener)
	{
		listener = new DeviceListener(nullptr);
		hr = listener->Init(hwnd, MultiThread);
	}
	return hr;
}

DllExport HRESULT Finalize()
{
	return listener->Finalize();
}

DllExport void GetDialState(float* AccDelta, int* ClickCount, int* LastSelectedItem)
{
	listener->GetDialState(AccDelta, ClickCount, LastSelectedItem);
}

DllExport bool RegisterCallback(
	bool(*aCallBack)(UINT device_identifier, float * value),
	void(*aOnControlAcquired)(),
	void(*aOnControlLost)()
)
{
	float aFloat = 13.5; // test

	listener->_OnControlAcquired = aOnControlAcquired;
	listener->_OnControlLost = aOnControlLost;

	//aCallBack(12, &aFloat);
	//aFloat += 3.1f;
	//aCallBack(16, &aFloat);

	return true;
}

DllExport HRESULT SetRotationResolution(float res)
{
	return listener->SetRotationResolution(res);
}

//size_t Test(const wchar_t* input, wchar_t* output, const size_t outlen)
//{
//	const std::wstring inputStr = input;
//	const std::wstring outputStr = foo(inputStr);
//	if (buff != nullptr)
//		wcsncpy(buff, outputStr.c_str(), len);
//	return outputStr.size() + 1; // the length required to copy the string
//}

DllExport HRESULT AddMenuItem(WCHAR* ICON_FILE_NAME, WCHAR* MENU_NAME, void(*aCallBack)())
{
	//return listener->AddExtMenu(L"\\Lib\\Brush.png", L"Test");// working
	return listener->AddExtMenu(L"\\Lib\\Brush.png", MENU_NAME);
	//return listener->AddExtMenu(ICON_FILE_NAME, MENU_NAME);
}

////////////////////////////////////////////////////////////////

//int _cdecl _tInit(HWND hwnd)
//{
	// Register Window Class
	//WNDCLASS wndClass = {
	//    CS_DBLCLKS, (WNDPROC)WindowProc,
	//    0,0,0,0,0,0,0, CLASSNAME
	//};
	//RegisterClass(&wndClass);

	//HWND hwnd = CreateWindow(
	//    CLASSNAME,
	//    L"Message Listener Window",
	//    WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_VSCROLL,
	//    CW_USEDEFAULT,       // default horizontal position 
	//    CW_USEDEFAULT,       // default vertical position 
	//    CW_USEDEFAULT,       // default width 
	//    CW_USEDEFAULT,       // default height
	//    NULL, NULL, NULL, NULL);

	//console = GetStdHandle(STD_OUTPUT_HANDLE);
	//PrintStartupMessage();

	// Self init
	//if (!listener)
	//{
//	listener = new DeviceListener(console);
//	listener->Init(hwnd);
	//}
//}


//    MSG msg;
//    while (GetMessage(&msg, NULL, 0, 0))
//    {
//        TranslateMessage(&msg);
//        DispatchMessage(&msg);  // Dispatch message to WindowProc
//
//        if (msg.message == WM_QUIT)
//        {
//            Windows::Foundation::Uninitialize();
//            break;
//        }
//    }
//
//    SetConsoleTextAttribute(console, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
//}

//LRESULT CALLBACK WindowProc(
//    __in HWND hWindow,
//    __in UINT uMsg,
//    __in WPARAM wParam,
//    __in LPARAM lParam)
//{
//    switch (uMsg)
//    {
//    case WM_CLOSE:
//        DestroyWindow(hWindow);
//        break;
//    case WM_DESTROY:
//        PostQuitMessage(0);
//        break;
//    case WM_SYSKEYUP: //Press F10
//        if (!listener)
//        {
//            listener = new DeviceListener(console);
//            listener->Init(hWindow);
//        }
//    default:
//        return DefWindowProc(hWindow, uMsg, wParam, lParam);
//    }
//
//    return 0;
//}
