unit Utils;

interface

uses
  Classes, SysUtils,  // System
  Windows // Winapi
  ;

resourcestring
  MsgDLLotFound = 'File %s not found. Program may not function properly. Please reinstall this app to fix the issue.';

function LoadLibraryWithCheck(aFilename : string) : THandle;

uses
  UITypes, AnsiStrings, System.StrUtils;

function CheckFileExistsAndWarn(aFilename : string) : boolean;
// Return true if file exists
begin
  result := FileExists(aFilename);
  if not result then
    MessageDlg(format(_(MsgDLLotFound), [aFilename]), mtWarning, [mbOK], 0);
end;

function MyLoadLibrary(aFilename : string) : THandle;
// 0 if fail loading. Return Handle (> 0) if successful.
var
  LDummyError: Cardinal;
begin
  Result := SafeLoadLibrary(PWideChar(aFilename), LDummyError);
  if Result = 0 then
    MessageDlg(format('Cannot load DLL (%s) Error: %d', [aFilename, GetLastError]), mtWarning, [mbOK], 0);
end;

function LoadLibraryWithCheck(aFilename : string) : THandle;
// Return -1 if file not found. 0 if fail loading. Return Handle (> 0) if successful.
begin
  if CheckFileExistsAndWarn(aFilename) then
  begin
    Result := MyLoadLibrary(PWideChar(aFilename));
  end
  else
    Result := THandle(-1);
end;

End.
