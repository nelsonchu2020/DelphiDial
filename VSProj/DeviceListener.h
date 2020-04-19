#pragma once

#include <wrl\client.h>
#include <windows.ui.input.h>
#include <RadialControllerInterop.h>

#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <roapi.h>
#include <wrl.h>
#include <wrl\client.h>
#include <windows.foundation.h>
#include <windows.foundation.numerics.h>
#include <windows.foundation.collections.h>
#include <windows.ui.input.h>
#include <Windows.ApplicationModel.h>
#include <Windows.ApplicationModel.datatransfer.h>

using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

class DeviceListener
{
public:
    DeviceListener(HANDLE console);
    ~DeviceListener() {}

    HRESULT Init(HWND hwnd, bool MultiThread);
    HRESULT SetRotationResolution(double res);
	HRESULT Finalize();

	void GetDialState(float* AccDelta, int* ClickCount, int* LastSelectedItem);
	void ResetQuery();
	float GetAccDelta();
	int GetClickCount();
	int GetLastSelectedItem();

	HRESULT AddExtMenu(WCHAR* ICON_FILE_NAME, WCHAR* MENU_NAME);
	HRESULT ClearMenuItems();

	void(*_OnControlAcquired) ();
	void(*_OnControlLost) ();
	/// Test:
	bool(*CallBack)(UINT device_identifier, float * value);

private:
	bool FReadingState  = false;
	bool FDialInControl = false;

    HRESULT RegisterForEvents(HWND hwnd);
    HRESULT PopulateMenuItems();

    HRESULT AddMenuItemFromKnownIcon(_In_ HSTRING itemName, _In_ ABI::Windows::UI::Input::RadialControllerMenuKnownIcon icon);

	HRESULT AddMenuItemFromUnknownIcon(_In_ HSTRING itemName,
		_In_ ComPtr<ABI::Windows::Storage::Streams::IRandomAccessStreamReference> icon);
///////////////////////

	//HRESULT AddMenuItemFromKnownIcon(_In_ HSTRING itemName, _In_ ABI::Windows::UI::Input::RadialControllerMenuKnownIcon icon, _In_ EventRegistrationToken registrationToken);
	HRESULT AddMenuItemFromSystemFont();
	HRESULT AddMenuItem(_In_ Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialControllerMenuItem> item, _In_ HSTRING itemName, _In_ EventRegistrationToken registrationToken);
	HRESULT GetFontUri(_Out_ Microsoft::WRL::ComPtr<ABI::Windows::Foundation::IUriRuntimeClass>* fontUri);
///////////////////////	

    HRESULT OnItemInvoked(_In_ ABI::Windows::UI::Input::IRadialControllerMenuItem* item, _In_ IInspectable* args);

    HRESULT OnScreenContactContinued(_In_ ABI::Windows::UI::Input::IRadialController*, _In_ ABI::Windows::UI::Input::IRadialControllerScreenContactContinuedEventArgs* args);
    HRESULT OnScreenContactStarted(_In_ ABI::Windows::UI::Input::IRadialController*, _In_ ABI::Windows::UI::Input::IRadialControllerScreenContactStartedEventArgs* args);
    HRESULT OnScreenContactEnded(_In_ ABI::Windows::UI::Input::IRadialController* /*sender*/, _In_ IInspectable* args);
    HRESULT OnControlLost(_In_ ABI::Windows::UI::Input::IRadialController* /*sender*/, _In_ IInspectable* args);
    HRESULT OnControlAcquired(_In_ ABI::Windows::UI::Input::IRadialController* /*sender*/, _In_ ABI::Windows::UI::Input::IRadialControllerControlAcquiredEventArgs* args);
    HRESULT OnRotationChanged(_In_ ABI::Windows::UI::Input::IRadialController* sender, _In_ ABI::Windows::UI::Input::IRadialControllerRotationChangedEventArgs* args);

    template<typename TContactArgs>
    HRESULT LogContact(_In_ TContactArgs* args);	
	void PrintMsg(WCHAR *message, WORD messageColor);

    DWORD _threadId = 0;
    HANDLE _console = nullptr;	

	float FAccDelta = 0.0;
	int FClickCount = 0;
	int FLastSelectedItem = -3;

    Microsoft::WRL::ComPtr<IRadialControllerInterop> _controllerInterop;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialController> _controller;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialControllerMenu> _menu;
    Microsoft::WRL::ComPtr<ABI::Windows::UI::Input::IRadialControllerMenuItemStatics> _menuItemStatics;

	EventRegistrationToken _rotatedToken;
	EventRegistrationToken _screenContactStartedToken;
	EventRegistrationToken _screenContactEndedToken;
	EventRegistrationToken _screenContactContinuedToken;
	EventRegistrationToken _controlLostToken;
	EventRegistrationToken _controlAcquiredToken;
	EventRegistrationToken _buttonClickedToken;
	EventRegistrationToken _knownIconItem1Token;
	EventRegistrationToken _knownIconItem2Token;
	EventRegistrationToken _systemFontItemToken;
	EventRegistrationToken _customFontItemToken;

	EventRegistrationToken _menuItem2Token;
};

