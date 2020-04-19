#include "stdafx.h"

#include "DeviceListener.h"
#include <windows.h>
#include <Windowsx.h>
#include <tchar.h>
#include <StrSafe.h>
#include <wrl\implements.h>
#include <wrl\module.h>
#include <wrl\event.h>
#include <roapi.h>
#include <wrl.h>
#include <map>
#include <vector>
#include <Pathcch.h>
#include <windows.foundation.h>
#include <windows.foundation.numerics.h>
#include <windows.foundation.collections.h>
#include <windows.ui.input.h>
#include <wrl\client.h>
#include <winstring.h>

#define RETURN_IF_FAILED(hr) { if(FAILED(hr)) return hr; }

using namespace ABI::Windows::UI::Input;
using namespace ABI::Windows::Foundation;
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;

DeviceListener::DeviceListener(HANDLE console)
{
    //_console = console;
}

HRESULT DeviceListener::Init(HWND hwnd, bool MultiThread)
{
    bool isRoInit = false;

	RO_INIT_TYPE MyType = (MultiThread) ? RO_INIT_MULTITHREADED : RO_INIT_SINGLETHREADED;

    HRESULT hr = Windows::Foundation::Initialize(MyType);	
	
    if (SUCCEEDED(hr))
    {
    //    PrintMsg(L"RO_INIT_MULTITHREADED SUCCEEDED\n", FOREGROUND_GREEN);
        isRoInit = true;
    }
    else
    {
    //    PrintMsg(L"RO_INIT_MULTITHREADED FAILED\n", FOREGROUND_RED);
    }

    if (isRoInit)
    { 
        hr = RegisterForEvents(hwnd);

        if (SUCCEEDED(hr))
        {
            hr = PopulateMenuItems();
        }

        if (SUCCEEDED(hr))
        {
            PrintMsg(L"Successfully initialized \n", FOREGROUND_GREEN);
        }
        else
        {
            PrintMsg(L"Failed to initialize\n", FOREGROUND_RED);
        }
    }

    return hr;
}

HRESULT DeviceListener::SetRotationResolution(double res)
{
    return _controller->put_RotationResolutionInDegrees(res);
}

HRESULT DeviceListener::Finalize()
{
	// remove them so that when we are closing the caller program, we don't get access violation 
	RETURN_IF_FAILED(_controller->remove_ControlLost(_controlLostToken));
	RETURN_IF_FAILED(_controller->remove_ControlAcquired(_controlAcquiredToken));
}

HRESULT DeviceListener::RegisterForEvents(HWND hwnd)
{
    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Input_RadialController).Get(),
        &_controllerInterop));

    RETURN_IF_FAILED(_controllerInterop->CreateForWindow(hwnd, IID_PPV_ARGS(&_controller)));

    // Wire events
    //RETURN_IF_FAILED(_controller->add_ScreenContactContinued(
    //    Callback<ITypedEventHandler<RadialController*, RadialControllerScreenContactContinuedEventArgs*>>(this, &DeviceListener::OnScreenContactContinued).Get(),
    //    &_screenContactContinuedToken));

    //RETURN_IF_FAILED(_controller->add_ScreenContactStarted(
    //    Callback<ITypedEventHandler<RadialController*, RadialControllerScreenContactStartedEventArgs*>>(this, &DeviceListener::OnScreenContactStarted).Get(),
    //    &_screenContactStartedToken));

    //RETURN_IF_FAILED(_controller->add_ScreenContactEnded(
    //    Callback<ITypedEventHandler<RadialController*, IInspectable*>>(this, &DeviceListener::OnScreenContactEnded).Get(),
    //    &_screenContactEndedToken));

    RETURN_IF_FAILED(_controller->add_ControlLost(
        Callback<ITypedEventHandler<RadialController*, IInspectable*>>(this, &DeviceListener::OnControlLost).Get(),
        &_controlLostToken));

    RETURN_IF_FAILED(_controller->add_ControlAcquired(
        Callback<ITypedEventHandler<RadialController*, RadialControllerControlAcquiredEventArgs*>>(this, &DeviceListener::OnControlAcquired).Get(),
        &_controlAcquiredToken));

    RETURN_IF_FAILED(_controller->add_RotationChanged(
        Callback<ITypedEventHandler<RadialController*, RadialControllerRotationChangedEventArgs*>>(this, &DeviceListener::OnRotationChanged).Get(),
        &_rotatedToken));

    // Lambda callback
    RETURN_IF_FAILED(_controller->add_ButtonClicked(
        Callback<ITypedEventHandler<RadialController*, RadialControllerButtonClickedEventArgs*>>([this]
        (IRadialController*, IRadialControllerButtonClickedEventArgs* args)
    {
//        PrintMsg(L"ButtonClicked\n", FOREGROUND_BLUE | FOREGROUND_GREEN);
		
		//if (CallBack!= NULL)
		//{			
		//	CallBack(1, &FAccDelta); // not working
		//}

		if (!FReadingState)
			FClickCount += 1;

//        RETURN_IF_FAILED(LogContact(args));

        return S_OK;
    }).Get(),
        &_buttonClickedToken));

    return S_OK;
}

HRESULT DeviceListener::ClearMenuItems()
{
	ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
	RETURN_IF_FAILED(_menu->get_Items(&menuItems));
	RETURN_IF_FAILED(menuItems->Clear());
}

HRESULT DeviceListener::PopulateMenuItems()
{
    RETURN_IF_FAILED(_controller->get_Menu(&_menu));

    PrintMsg(L"Got Menu \n", FOREGROUND_BLUE);	

    RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_UI_Input_RadialControllerMenuItem).Get(),
        &_menuItemStatics));

	RETURN_IF_FAILED(ClearMenuItems());

	//RETURN_IF_FAILED(AddMenuItemFromKnownIcon(HStringReference(L"Rotate").Get(),
	//	RadialControllerMenuKnownIcon::RadialControllerMenuKnownIcon_NextPreviousTrack));

	//RETURN_IF_FAILED(AddMenuItemFromKnownIcon(HStringReference(L"Brush").Get(),
	//	RadialControllerMenuKnownIcon::RadialControllerMenuKnownIcon_PenType));
	
	RETURN_IF_FAILED(AddMenuItemFromSystemFont());

//	AddExtMenu(L"\\Lib\\Rotate.png", L"Rotate");
	AddExtMenu(L"\\Lib\\Brush.png", L"Brush");		

    return S_OK;
}

HRESULT DeviceListener::AddMenuItemFromSystemFont()
{
	// Create item from system font
	ComPtr<IRadialControllerMenuItemStatics2> menuItemStatics2;
	RETURN_IF_FAILED(Windows::Foundation::GetActivationFactory(
		HStringReference(RuntimeClass_Windows_UI_Input_RadialControllerMenuItem).Get(),
		&menuItemStatics2));

	HSTRING toolDisplayName = HStringReference(L"Rotate").Get();
	ComPtr<IRadialControllerMenuItem> systemFontItem;
	menuItemStatics2->CreateFromFontGlyph(toolDisplayName, HStringReference(L"\xE7AD").Get(), HStringReference(L"Segoe MDL2 Assets").Get(), &systemFontItem);

	RETURN_IF_FAILED(AddMenuItem(systemFontItem, toolDisplayName, _systemFontItemToken));

	//// Get font uri
	//ComPtr<IUriRuntimeClass> fontUri;
	//RETURN_IF_FAILED(GetFontUri(&fontUri));

	//// Create item from custom font
	//toolDisplayName = HStringReference(L"Custom Font Item").Get();
	//ComPtr<IRadialControllerMenuItem> customFontItem;
	//menuItemStatics2->CreateFromFontGlyphWithUri(toolDisplayName, HStringReference(L"\ue102").Get(), HStringReference(L"Symbols").Get(), fontUri.Get(), &customFontItem);

	//RETURN_IF_FAILED(AddMenuItem(customFontItem, toolDisplayName, _customFontItemToken));
}

HRESULT DeviceListener::AddMenuItem(_In_ ComPtr<IRadialControllerMenuItem> item, _In_ HSTRING itemName, _In_ EventRegistrationToken registrationToken)
{
	// Set Callback
	RETURN_IF_FAILED(item->add_Invoked(
		Callback<ITypedEventHandler<RadialControllerMenuItem*, IInspectable*>>(this, &DeviceListener::OnItemInvoked).Get(),
		&registrationToken));

	// Get menu items
	ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
	RETURN_IF_FAILED(_menu->get_Items(&menuItems));

	// Add item to menu
	RETURN_IF_FAILED(menuItems->Append(item.Get()));

	// Log new item
	wchar_t message[2000];
	swprintf_s(message, 2000, L"Added %s to menu\n", WindowsGetStringRawBuffer(itemName, nullptr));
	PrintMsg(message, FOREGROUND_BLUE | FOREGROUND_GREEN);

	return S_OK;
}

HRESULT DeviceListener::GetFontUri(_Out_ ComPtr<IUriRuntimeClass>* fontUri)
{
	WCHAR currentPath[MAX_PATH];
	GetCurrentDirectory(MAX_PATH, currentPath);
	WCHAR fontFile[] = L"..\\shared\\Symbols.ttf";

	WCHAR fontPath[MAX_PATH];
	RETURN_IF_FAILED(PathCchCombine(fontPath, MAX_PATH, currentPath, fontFile));

	ComPtr<IUriRuntimeClassFactory> uriRuntimeClassFactory;
	RETURN_IF_FAILED(GetActivationFactory(HStringReference(RuntimeClass_Windows_Foundation_Uri).Get(), &uriRuntimeClassFactory));

	RETURN_IF_FAILED(uriRuntimeClassFactory->CreateUri(HStringReference(fontPath).Get(), &(*fontUri)));

	return S_OK;
}

HRESULT DeviceListener::AddExtMenu(WCHAR* ICON_FILE_NAME, WCHAR* MENU_NAME)
{
	// Icon's filename for adding
	//const WCHAR*    ICON_FILE_NAME = L"\\Lib\\Rotate.png";
	//const WCHAR*    MENU_NAME = L"Rotate";

	HRESULT hr;
	ComPtr<IActivationFactory> pFactory;
	hr = Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(RuntimeClass_Windows_Storage_StorageFile).Get(), &pFactory);
	if (S_OK != hr)
		__debugbreak();

	ComPtr<ABI::Windows::Storage::IStorageFileStatics> pStorageFileStatics;
	hr = pFactory.As(&pStorageFileStatics);
	if (S_OK != hr)
		__debugbreak();

	// Open the file.
	WCHAR   wchFullPath[_MAX_PATH] = {};
	GetCurrentDirectory(_MAX_PATH, wchFullPath);
	wcscat_s(wchFullPath, ICON_FILE_NAME);

	HString path;
	path.Set(wchFullPath);

	ComPtr<ABI::Windows::Foundation::__FIAsyncOperation_1_Windows__CStorage__CStorageFile_t> async;
	pStorageFileStatics->GetFileFromPathAsync(path.Get(), &async);

	typedef IAsyncOperationCompletedHandler<ABI::Windows::Storage::StorageFile*> HandlerType;
	auto handler = Microsoft::WRL::Callback<HandlerType>([this, MENU_NAME](IAsyncOperation<ABI::Windows::Storage::StorageFile*>* async, AsyncStatus status)
	{
		HRESULT     hr = S_FALSE;
		ComPtr<ABI::Windows::Storage::IStorageFile> file;
		switch (status)
		{
		case Started:
			break;
		case Completed:
		case Error:
			hr = async->GetResults(&file);
			break;
		case Canceled:
			break;
		}
		if (file.Get())
		{
			ComPtr<IActivationFactory> pStreamRefFactory;
			const WCHAR* runtime_class_name = L"Windows.Storage.Streams.RandomAccessStreamReference";
			hr = Windows::Foundation::GetActivationFactory(Microsoft::WRL::Wrappers::HStringReference(runtime_class_name).Get(), &pStreamRefFactory);
			if (S_OK != hr)
				__debugbreak();

			ComPtr<ABI::Windows::Storage::Streams::IRandomAccessStreamReferenceStatics> pStreamRefStatic;
			hr = pStreamRefFactory.As(&pStreamRefStatic);
			if (S_OK != hr)
				__debugbreak();

			ComPtr<ABI::Windows::Storage::Streams::IRandomAccessStreamReference> streamRef;
			hr = pStreamRefStatic->CreateFromFile(file.Get(), &streamRef);
			AddMenuItemFromUnknownIcon(HStringReference(MENU_NAME).Get(), streamRef);
		}
		return hr;
	});
	async->put_Completed(handler.Get());
	return hr;
}

HRESULT DeviceListener::AddMenuItemFromUnknownIcon(_In_ HSTRING itemName, 
	_In_ ComPtr<ABI::Windows::Storage::Streams::IRandomAccessStreamReference> icon)
{
	// Get menu items
	ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
	RETURN_IF_FAILED(_menu->get_Items(&menuItems));

	// Create item
	ComPtr<IRadialControllerMenuItem> menuItem;
	HRESULT hr = _menuItemStatics->CreateFromIcon(itemName, icon.Get(), &menuItem);

	RETURN_IF_FAILED(hr);

	// Set Callback
	RETURN_IF_FAILED(menuItem->add_Invoked(
		Callback<ITypedEventHandler<RadialControllerMenuItem*, IInspectable*>>(this, &DeviceListener::OnItemInvoked).Get(),
		&_menuItem2Token));

	// Add item to menu
	RETURN_IF_FAILED(menuItems->Append(menuItem.Get()));

	// Log new item
	//wchar_t message[2000];
	//swprintf_s(message, 2000, L"Added %s to menu\n", WindowsGetStringRawBuffer(itemName, nullptr));
	//WriteDebugStringW(DEBUG_OUTPUT_TYPE_DEBUG_CONSOLE, message);

	return S_OK;
}

HRESULT DeviceListener::AddMenuItemFromKnownIcon(_In_ HSTRING itemName, _In_ RadialControllerMenuKnownIcon icon)
{
    // Get menu items
    ComPtr<Collections::IVector<RadialControllerMenuItem*>> menuItems;
    RETURN_IF_FAILED(_menu->get_Items(&menuItems));

	//menuItems->Clear(); // test

    // Create item
    ComPtr<IRadialControllerMenuItem> menuItem;

    RETURN_IF_FAILED(_menuItemStatics->CreateFromKnownIcon(itemName, icon, &menuItem));

    // Set Callback
    RETURN_IF_FAILED(menuItem->add_Invoked(
        Callback<ITypedEventHandler<RadialControllerMenuItem*, IInspectable*>>(this, &DeviceListener::OnItemInvoked).Get(),
        &_menuItem2Token));

    // Add item to menu
    RETURN_IF_FAILED(menuItems->Append(menuItem.Get()));

    // Log new item
    //wchar_t message[2000];
    //swprintf_s(message, 2000, L"Added %s to menu\n", WindowsGetStringRawBuffer(itemName, nullptr));
    //PrintMsg(message, FOREGROUND_BLUE | FOREGROUND_GREEN);

    return S_OK;
}

HRESULT DeviceListener::OnItemInvoked(_In_ IRadialControllerMenuItem* item, _In_ IInspectable* args)
{
	FLastSelectedItem = -4;

    if (item != nullptr)
    {
        //wchar_t message[2000];
        HSTRING str;
				
        item->get_DisplayText(&str);
				
		if (wcscmp(WindowsGetStringRawBuffer(str, nullptr), L"Brush") == 0)
		{
			FLastSelectedItem = 0;
		}
		else
		if (wcscmp(WindowsGetStringRawBuffer(str, nullptr), L"Rotate") == 0)
		{
			FLastSelectedItem = 1;
		}		
		else
		{
			FLastSelectedItem = -1;
		}

        //swprintf_s(message, 2000, L"ItemInvoked %s\n", WindowsGetStringRawBuffer(str, nullptr));
        //PrintMsg(message, FOREGROUND_RED | FOREGROUND_GREEN);
    }

    return S_OK;
}

HRESULT DeviceListener::OnScreenContactContinued(_In_ IRadialController* /*sender*/, _In_ IRadialControllerScreenContactContinuedEventArgs* args)
{
    RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

HRESULT DeviceListener::OnScreenContactStarted(_In_ IRadialController* /*sender*/, _In_ IRadialControllerScreenContactStartedEventArgs* args)
{
    RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

HRESULT DeviceListener::OnScreenContactEnded(_In_ IRadialController* /*sender*/, _In_ IInspectable* args)
{
    PrintMsg(L"ScreenContactEnded\n", FOREGROUND_BLUE | FOREGROUND_GREEN);

    return S_OK;
}

HRESULT DeviceListener::OnControlLost(_In_ IRadialController* /*sender*/, _In_ IInspectable* args)
{
    //PrintMsg(L"ControlLost\n", FOREGROUND_RED);

	FDialInControl = false;

	if (_OnControlLost != NULL)
	_OnControlLost();

    return S_OK;
}

HRESULT DeviceListener::OnControlAcquired(_In_ IRadialController* /*sender*/, _In_ IRadialControllerControlAcquiredEventArgs* args)
{
    //PrintMsg(L"ControlAcquired\n", FOREGROUND_GREEN);
    //RETURN_IF_FAILED(LogContact(args));

	FDialInControl = true;

	if (_OnControlAcquired != NULL)
	_OnControlAcquired();

    return S_OK;
}

float DeviceListener::GetAccDelta()
{
	return FAccDelta;	
}

int DeviceListener::GetClickCount()
{
	return FClickCount;
}

int DeviceListener::GetLastSelectedItem()
{
	return FLastSelectedItem;
}

void DeviceListener::ResetQuery()
{
	FAccDelta = 0.0;
	FClickCount = 0;	
}

HRESULT DeviceListener::OnRotationChanged(_In_ IRadialController* /*sender*/, _In_ IRadialControllerRotationChangedEventArgs* args)
{
    //wchar_t message[2000];
    double delta = 0;

    args->get_RotationDeltaInDegrees(&delta);
    //swprintf_s(message, 2000, L"RotationChanged delta=%lf\n", delta);
    //PrintMsg(message, FOREGROUND_GREEN | FOREGROUND_RED);

	if (!FReadingState)
		FAccDelta += (float)delta;

    //RETURN_IF_FAILED(LogContact(args));

    return S_OK;
}

template<typename TContactArgs>
HRESULT DeviceListener::LogContact(_In_ TContactArgs* args)
{
    // Get contact
    ComPtr<IRadialControllerScreenContact> contact;
    RETURN_IF_FAILED(args->get_Contact(&contact));

    if (contact != nullptr)
    {
        // Get contact info
        Point position;
        Rect bounds;
        RETURN_IF_FAILED(contact->get_Position(&position));
        RETURN_IF_FAILED(contact->get_Bounds(&bounds));

        // Log contact info
        wchar_t message[2000];
        swprintf_s(message, 2000, L"\t Postion X=%lf, Y=%lf & \n \t Bounds Height=%lf, Width=%lf, X=%lf, Y=%lf\n",
            position.X, position.Y,
            bounds.Height, bounds.Width, bounds.X, bounds.Y);

        PrintMsg(message, FOREGROUND_GREEN | FOREGROUND_BLUE);
    }

    return S_OK;
}

void DeviceListener::GetDialState(float* AccDelta, int* ClickCount, int* LastSelectedItem)
{
	FReadingState = true;

	*AccDelta = GetAccDelta();
	*ClickCount = GetClickCount();
	*LastSelectedItem = GetLastSelectedItem();	

	ResetQuery();

	FReadingState = false;
}

void DeviceListener::PrintMsg(WCHAR *message, WORD messageColor)
{
    //UINT bufferSize = 2000;
    //size_t textLength;
    //DWORD charsWritten;
    //StringCchLength(message, bufferSize, &textLength);

    // Set font to message color
    //SetConsoleTextAttribute(_console, messageColor);

    // Send message
    //WriteConsole(_console, message, (DWORD)textLength, &charsWritten, NULL);

    // Return font to original color
    //SetConsoleTextAttribute(_console, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}