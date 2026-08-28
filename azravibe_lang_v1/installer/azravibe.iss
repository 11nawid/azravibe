#ifndef MyAppVersion
  #error "MyAppVersion define is required."
#endif

#ifndef MyAppVersionNumeric
  #error "MyAppVersionNumeric define is required."
#endif

#ifndef SourceRoot
  #error "SourceRoot define is required."
#endif

#ifndef OutputRoot
  #error "OutputRoot define is required."
#endif

#ifndef InstallerAssetsDir
  #error "InstallerAssetsDir define is required."
#endif

#ifndef ConsoleAppRoot
  #error "ConsoleAppRoot define is required."
#endif

#define MyAppName "Azravibe"
#define MyAppPublisher "Azravibe"
#define MyAppExeName "azravibe.exe"
#define MyAppConsoleExeName "Azravibe IDE.exe"

[Setup]
AppId={{E6DB61DA-BD3D-4C37-B4E8-D25F2ABCF019}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={localappdata}\Programs\Azravibe
DefaultGroupName=Azravibe
DisableDirPage=no
DisableProgramGroupPage=no
LicenseFile={#SourceRoot}\LICENSE
OutputDir={#OutputRoot}
OutputBaseFilename=Azravibe-Setup-{#MyAppVersion}
Compression=lzma2/ultra64
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
ChangesEnvironment=yes
UninstallDisplayIcon={app}\{#MyAppExeName}
UninstallDisplayName=Azravibe
VersionInfoVersion={#MyAppVersionNumeric}
VersionInfoCompany=Azravibe
VersionInfoDescription=Azravibe Installer
VersionInfoProductName=Azravibe Setup
WizardImageFile={#InstallerAssetsDir}\wizard.bmp
WizardSmallImageFile={#InstallerAssetsDir}\wizard-small.bmp
SetupLogging=yes

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "addtopath"; Description: "Add Azravibe command-line tools to your PATH"; Flags: checkedonce
Name: "desktopicon"; Description: "Create a desktop shortcut"; Flags: checkedonce

[Files]
Source: "{#SourceRoot}\azravibe.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\azr.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\versions.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\README.md"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourceRoot}\LICENSE"; DestDir: "{app}"; DestName: "LICENSE.txt"; Flags: ignoreversion
Source: "{#SourceRoot}\stdlib\*"; DestDir: "{app}\stdlib"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#ConsoleAppRoot}\*"; DestDir: "{app}\console"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "{#SourceRoot}\installer\fonts\Vazir-Code.ttf"; DestDir: "{autofonts}"; FontInstall: "Vazir Code"; Flags: onlyifdoesntexist uninsneveruninstall
Source: "{#SourceRoot}\installer\fonts\Vazir-Code-LICENSE.txt"; DestDir: "{app}\fonts"; Flags: ignoreversion

[Icons]
Name: "{group}\Azravibe"; Filename: "{app}\console\{#MyAppConsoleExeName}"; Parameters: "--console"; WorkingDir: "{app}\console"
Name: "{group}\Azravibe Command Line"; Filename: "{app}\azravibe.exe"; WorkingDir: "{app}"
Name: "{group}\Azravibe README"; Filename: "{app}\README.md"; WorkingDir: "{app}"
Name: "{group}\Uninstall Azravibe"; Filename: "{uninstallexe}"
Name: "{autodesktop}\Azravibe"; Filename: "{app}\console\{#MyAppConsoleExeName}"; Parameters: "--console"; WorkingDir: "{app}\console"; Tasks: desktopicon

[Code]
const
  AZR_HWND_BROADCAST = $FFFF;
  AZR_WM_SETTINGCHANGE = $001A;
  AZR_SMTO_ABORTIFHUNG = $0002;

function SendMessageTimeout(
  hWnd: Integer;
  Msg: Integer;
  wParam: Integer;
  lParam: string;
  fuFlags: Integer;
  uTimeout: Integer;
  var lpdwResult: DWORD): DWORD;
  external 'SendMessageTimeoutW@user32.dll stdcall';

function NormalizePath(const Value: string): string;
begin
  Result := RemoveBackslashUnlessRoot(Trim(Value));
end;

function ConsumeNextPathSegment(var Value: string): string;
var
  SepPos: Integer;
begin
  SepPos := Pos(';', Value);
  if SepPos = 0 then
  begin
    Result := Value;
    Value := '';
  end
  else
  begin
    Result := Copy(Value, 1, SepPos - 1);
    Delete(Value, 1, SepPos);
  end;
end;

function PathContainsEntry(const CurrentPath: string; const Entry: string): Boolean;
var
  Remaining: string;
  Segment: string;
  NormalizedEntry: string;
begin
  Result := False;
  Remaining := CurrentPath;
  NormalizedEntry := NormalizePath(Entry);

  while Remaining <> '' do
  begin
    Segment := Trim(ConsumeNextPathSegment(Remaining));
    if (Segment <> '') and (CompareText(NormalizePath(Segment), NormalizedEntry) = 0) then
    begin
      Result := True;
      Exit;
    end;
  end;
end;

function RemovePathEntryFromList(const CurrentPath: string; const Entry: string): string;
var
  Remaining: string;
  Segment: string;
  Updated: string;
  NormalizedEntry: string;
begin
  Remaining := CurrentPath;
  Updated := '';
  NormalizedEntry := NormalizePath(Entry);

  while Remaining <> '' do
  begin
    Segment := Trim(ConsumeNextPathSegment(Remaining));
    if (Segment <> '') and (CompareText(NormalizePath(Segment), NormalizedEntry) <> 0) then
    begin
      if Updated = '' then
        Updated := Segment
      else
        Updated := Updated + ';' + Segment;
    end;
  end;

  Result := Updated;
end;

procedure BroadcastEnvironmentChange();
var
  MsgResult: DWORD;
begin
  MsgResult := 0;
  SendMessageTimeout(AZR_HWND_BROADCAST, AZR_WM_SETTINGCHANGE, 0, 'Environment', AZR_SMTO_ABORTIFHUNG, 5000, MsgResult);
end;

procedure AddUserPathEntry(const Entry: string);
var
  CurrentPath: string;
  UpdatedPath: string;
begin
  if not RegQueryStringValue(HKCU, 'Environment', 'Path', CurrentPath) then
    CurrentPath := '';

  if PathContainsEntry(CurrentPath, Entry) then
    Exit;

  if CurrentPath = '' then
    UpdatedPath := Entry
  else
    UpdatedPath := CurrentPath + ';' + Entry;

  RegWriteExpandStringValue(HKCU, 'Environment', 'Path', UpdatedPath);
  BroadcastEnvironmentChange();
end;

procedure RemoveUserPathEntry(const Entry: string);
var
  CurrentPath: string;
  UpdatedPath: string;
begin
  if not RegQueryStringValue(HKCU, 'Environment', 'Path', CurrentPath) then
    Exit;

  UpdatedPath := RemovePathEntryFromList(CurrentPath, Entry);
  if UpdatedPath = CurrentPath then
    Exit;

  if UpdatedPath = '' then
    RegDeleteValue(HKCU, 'Environment', 'Path')
  else
    RegWriteExpandStringValue(HKCU, 'Environment', 'Path', UpdatedPath);

  BroadcastEnvironmentChange();
end;

procedure CurStepChanged(CurStep: TSetupStep);
begin
  if (CurStep = ssPostInstall) and WizardIsTaskSelected('addtopath') then
    AddUserPathEntry(ExpandConstant('{app}'));
end;

procedure CurUninstallStepChanged(CurUninstallStep: TUninstallStep);
begin
  if CurUninstallStep = usUninstall then
    RemoveUserPathEntry(ExpandConstant('{app}'));
end;
