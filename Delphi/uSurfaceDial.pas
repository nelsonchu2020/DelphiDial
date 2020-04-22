unit uSurfaceDial;

interface

uses
  Winapi.Windows, Vcl.ExtCtrls, System.Classes;

type
  TDialRotateEvent = procedure(Delta : single) of object;
  TInControlChangeEvent = procedure(InControl : boolean) of object;

  TGetProperty = function(device_identifier: UInt; value: PSingle): Boolean; cdecl; // test
  TSimpleCallback = procedure(); cdecl;
  TMyTestFunction = function(X, Y: Integer): Integer; cdecl;

  TSetRotResFunc = function(res : single): HRESULT; cdecl;
  TInitFunc = function(hWnd: HWnd; MultiThread : boolean): HRESULT; cdecl;
  TFinalizeFunc = function(): HRESULT; cdecl;
  TGetDialStatusFunc = procedure(AccDelta : PSingle; ClickCount, LastSelectedItem : PInteger); cdecl;

  TRegisterCallbackFunc = function (GetProperty: TGetProperty;
    OnControlAcquired : TSimpleCallback; OnControlLost : TSimpleCallback): Boolean; cdecl;

  TAddMenuItemFunc = function (PNGFilename: PWChar; ItemText: PWChar; MenuCallBack : TSimpleCallback) : HRESULT; cdecl;

  TSurfaceDial = class
  private
    FWindowHandle: THandle;
    WTHandle: THandle;
    FEnabled: boolean;

    FMenuCallBack : TSimpleCallback;

    FInit : TInitFunc;
    FFinalize : TFinalizeFunc;
    FRegisterCallback : TRegisterCallbackFunc;
    FSetRotationResolution : TSetRotResFunc;
    FGetDialState : TGetDialStatusFunc;
    FAddMenuItem : TAddMenuItemFunc;

//    FOnControlAcquired : TControlChangeCallback;
//    FOnControlLost : TControlChangeCallback;

//    DoMath : TMyTestFunction; // test

    FOnClick: TNotifyEvent;
    FOnRotate: TDialRotateEvent;
    FRotationResolution: single;
    FSelectedItem: integer;
    FOnInControlChange: TInControlChangeEvent;
//    FInControl: boolean;
    FTempDisable: boolean;
    FOnBeforeProcess: TNotifyEvent;
    FOnAfterProcess: TNotifyEvent;
    FOnInitFail: TNotifyEvent;

    procedure SetEnabled(const Value: boolean);
    procedure SetOnRotate(const Value: TDialRotateEvent);
    procedure SetRotationResolution(res : single);
    procedure SetSelectedItem(const Value: integer);
    procedure ApplyRotationResForMenuItems(item : integer);
    function GetEnabled: boolean;
    procedure SetOnAfterProcess(const Value: TNotifyEvent);
    procedure SetOnBeforeProcess(const Value: TNotifyEvent);

    function GetInControl: boolean;
    procedure SetOnInitFail(const Value: TNotifyEvent);

    procedure DoInit(aHWnd: THandle);

  public
    constructor Create;
    destructor Destroy; override;

    procedure LoadDLL(aDLLNameWithFullPath : string);
    procedure UnLoadDLL;
    function Init(aHWnd: THandle) : string;

    function LibLoaded : boolean;
    procedure AddMenuItem(PNGFilename, Text : WideString);

    function ProcessInput : boolean;
    procedure OnMenuVisibleToggle(HUDMenuVisible: boolean; HUDMenuSelect : integer);

    property Enabled : boolean read GetEnabled write SetEnabled;
    property TempDisable : boolean read FTempDisable write FTempDisable;
    property SelectedItem : integer read FSelectedItem write SetSelectedItem;
    property RotationResolution : single read FRotationResolution write SetRotationResolution;
    property InControl : boolean read GetInControl;

    property OnClick : TNotifyEvent read FOnClick write FOnClick;
    property OnRotate : TDialRotateEvent read FOnRotate write SetOnRotate;
    property OnInControlChange : TInControlChangeEvent read FOnInControlChange write FOnInControlChange;
    property OnBeforeProcess : TNotifyEvent read FOnBeforeProcess write SetOnBeforeProcess;
    property OnAfterProcess : TNotifyEvent read FOnAfterProcess write SetOnAfterProcess;
    property OnInitFail : TNotifyEvent read FOnInitFail write SetOnInitFail;
  end;

implementation

uses
  Vcl.Dialogs, System.SysUtils, Utils;

const
{$ifdef Win64}
  DialDLLName     = 'DialBridge64.dll';
  DialDLLNameFullPath = 'Lib\x64\' + DialDLLName;
{$else}
  DialDLLName     = 'DialBridge.dll';
  DialDLLNameFullPath = 'Lib\' + DialDLLName;
{$endif}


resourcestring
  GetProcFailed = 'Procedure %s failed to load properly.';

//function RegisterCallback(GetProperty: TGetProperty;
//  OnControlAcquired : TControlChangeCallback;
//  OnControlLost : TControlChangeCallback
//  ): Boolean; cdecl; external DialDLLNameFullPath;

var
  UInControl : boolean;
  Win10OrLater : boolean;

{ TSurfaceDial }

procedure _OnControlAcquired; cdecl;
begin
  UInControl := true;
end;

procedure _OnControlLost; cdecl;
begin
  UInControl := false;
end;

function Callback(device_identifier: UInt; value: PSingle): Boolean cdecl;
var
  LSingle : single;
begin
  Result := true;
  LSingle := value^;
  value^ := 14.5;
end;

constructor TSurfaceDial.Create;
begin
  inherited;
end;

destructor TSurfaceDial.Destroy;
begin
  if Libloaded then
  begin
    FFinalize();
//    UnLoadDLL;
  end;
  inherited;
end;

procedure TSurfaceDial.DoInit(aHWnd: THandle);
var
//  i : integer;
  b : boolean;
  h : HRESULT;
begin
  if Win10OrLater then
  begin
    LoadDLL(AppPath + DialDLLNameFullPath);
    if Enabled then
    begin
//    i := DoMath(12, 3);  // tested to work
      h := FInit(aHWnd, false);
      if h = 0 then
      begin
        b := FRegisterCallback(Callback, _OnControlAcquired, _OnControlLost); // working
        AddMenuItem('\\Lib\\Brush.png', 'Brush1'); // experimental
      end
      else
      begin
//        MessageDlg(format('Unable to initialize Dial control (code: %d).', [h]), mtError, [mbOK], 0);
        if Libloaded then
        begin
          UnLoadDLL;
        end;
        Enabled := false;
        if Assigned(OnInitFail) then
          OnInitFail(self);
      end;
    end;
  end;
end;

function TSurfaceDial.GetEnabled: boolean;
begin
  result := FEnabled and not FTempDisable;
end;

function TSurfaceDial.GetInControl: boolean;
begin
  result := UInControl;
end;

procedure TSurfaceDial.AddMenuItem(PNGFilename, Text : WideString);
var
  h : HRESULT;
  myWideString  : WideString;
  myWideCharPtr : PWideChar;
  ArrayStrCmd : array[0..134] of WideChar;
begin
  myWideString  := 'Play';
  ArrayStrCmd := '1Test' + #0;
  // Point to the first character in the string
  myWideCharPtr := Addr(myWideString[1]);
//  h := FAddMenuItem(@PNGFilename, @Text, FMenuCallBack);
//  h := FAddMenuItem(@PNGFilename, myWideCharPtr, FMenuCallBack);
//  h := FAddMenuItem(PWideChar(PNGFilename), PWideChar(myWideString), FMenuCallBack); // wrong string displayed
end;

function TSurfaceDial.Init(aHWnd: THandle) : string;
begin
  try
    DoInit(aHWnd);
    result := '';
  except
    on E: Exception do
    begin
      result := E.Message;
    end;
  end;
end;

function TSurfaceDial.LibLoaded: boolean;
begin
  result := wtHandle <> 0;
end;

procedure TSurfaceDial.LoadDLL(aDLLNameWithFullPath : string);

  procedure ValidateProc(var ProcPtr: Pointer; const ProcName: string);
  begin
    ProcPtr := GetProcAddress(wtHandle, PChar(ProcName));
    if not Assigned(ProcPtr) then
      raise Exception.CreateFmt(GetProcFailed, [ProcName]);
  end;

  procedure LoadProcs;
  begin
    ValidateProc(@FInit, 'Init');
    ValidateProc(@FSetRotationResolution, 'SetRotationResolution');
    ValidateProc(@FGetDialState, 'GetDialState');
    ValidateProc(@FFinalize, 'Finalize');
    ValidateProc(@FRegisterCallback, 'RegisterCallback');
    ValidateProc(@FAddMenuItem, 'AddMenuItem');
//    DoMath := GetProcAddress(wtHandle, 'DoMath');
  end;

begin
  if not FileExists(aDLLNameWithFullPath) then
  begin
    Enabled := false;
    MessageDlg(format(MsgDLLotFound, [aDLLNameWithFullPath]), mtError, [mbOK], 0);
  end;

  wtHandle := LoadLibraryWithCheck(PChar(aDLLNameWithFullPath));

  try
  if LibLoaded then
  begin
    LoadProcs;
    Enabled := true;
  end
  except
    on E: Exception do
    begin
      MessageDlg('Error loading SurfaceDial.dll functions ' + '(' + E.Message + ')'+ #13#10 +
                 'You may need to update Windows to get proper Surface Dial support.', mtError, [mbOK], 0);
      Enabled := false;
    end;
  end;
end;

procedure TSurfaceDial.ApplyRotationResForMenuItems(item : integer);
// 'Brush Size', 'Loading', 'Wetness', 'Scratchiness', 'Zoom', 'Rotate' , Undo / Redo
begin
  if item in [5, 6] then
    RotationResolution := 2 // higher precision
  else
    RotationResolution := 10;
end;

procedure TSurfaceDial.OnMenuVisibleToggle(HUDMenuVisible: boolean; HUDMenuSelect : integer);
begin
  if HUDMenuVisible then
    RotationResolution := 10 // menu selection using coarser
  else
  begin
    if SelectedItem = 0 then // Brush Item selected
      ApplyRotationResForMenuItems(HUDMenuSelect)
    else
      RotationResolution := 2 // for rotate
  end;
end;

function TSurfaceDial.ProcessInput : boolean;
// Return true if there's input
var
  aAccDelta : Single;
  aClickCount, aLastSelectedItem : Integer;
begin
  result := false;

  if Enabled then
  begin
    if Assigned(OnBeforeProcess) then
      OnBeforeProcess(self);

//    if InControl <> aInControl then
//    begin
//      UInControl := aInControl;
//      if Assigned(OnInControlChange) then
//        OnInControlChange(InControl);
//    end;

    if InControl then // skipping if not in control
    begin
      FGetDialState(@aAccDelta, @aClickCount, @aLastSelectedItem); // causing black cached rendering - not sure why

      if aLastSelectedItem <> FSelectedItem then
      begin
        SelectedItem := aLastSelectedItem;
        ApplyRotationResForMenuItems(SelectedItem);
      end;

      if aClickCount > 0 then
      begin
        if Assigned(FOnClick) then
          FOnClick(self);
        result := true;
      end;

      if aAccDelta <> 0 then
      begin
        if Assigned(OnRotate) then
          OnRotate(aAccDelta);
        result := true;
      end;
    end;

    if Assigned(OnAfterProcess) then
      OnAfterProcess(self);
  end;
end;

procedure TSurfaceDial.SetEnabled(const Value: boolean);
begin
  FEnabled := Value and LibLoaded;
end;

procedure TSurfaceDial.SetOnAfterProcess(const Value: TNotifyEvent);
begin
  FOnAfterProcess := Value;
end;

procedure TSurfaceDial.SetOnBeforeProcess(const Value: TNotifyEvent);
begin
  FOnBeforeProcess := Value;
end;

procedure TSurfaceDial.SetOnInitFail(const Value: TNotifyEvent);
begin
  FOnInitFail := Value;
end;

procedure TSurfaceDial.SetOnRotate(const Value: TDialRotateEvent);
begin
  FOnRotate := Value;
end;

procedure TSurfaceDial.SetRotationResolution(res: single);
begin
  FRotationResolution := res;
  if LibLoaded then
    FSetRotationResolution(res);
end;

procedure TSurfaceDial.SetSelectedItem(const Value: integer);
begin
  FSelectedItem := Value;
end;

procedure TSurfaceDial.UnLoadDLL;
begin
  if Libloaded then
  begin
    if FreeLibrary(wtHandle) then // Free the lib first
    begin
      wtHandle := 0;
    end;
  end;
end;

initialization
  Win10OrLater := CheckWin32Version(6, 2); // win10+

end.
