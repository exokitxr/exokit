#define MyAppName "Exokit"
#define MyAppShortName "Exokit"
#define MyAppLCShortName "exokit"
#define MyAppPublisher "WebMR"
#define MyAppURL "https://github.com/webmixedreality/exokit"
#define MyAppExeName "exokit.cmd"
#define MyIcon "exokit.ico"
#define ProjectRoot ".."

[Setup]
; NOTE: The value of AppId uniquely identifies this application.
; Do not use the same AppId value in installers for other applications.
; (To generate a new GUID, click Tools | Generate GUID inside the IDE.)
PrivilegesRequired=admin
AppId=26ED4ED2-804E-48D8-96CB-F89B8EE53DA1
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppVerName={#MyAppName} {#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName=C:\{#MyAppLCShortName}
DisableDirPage=no
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
LicenseFile={#ProjectRoot}\LICENSE.md
OutputDir={#ProjectRoot}\dist\{#MyAppVersion}
OutputBaseFilename={#MyAppLCShortName}-v{#MyAppVersion}-win-x64
SetupIconFile={#ProjectRoot}\metadata\{#MyIcon}
Compression=lzma
; Much faster
; Compression=none
SolidCompression=yes
ChangesEnvironment=yes
DisableProgramGroupPage=yes
ArchitecturesInstallIn64BitMode=x64 ia64
UninstallDisplayIcon={app}\metadata\{#MyIcon}
AppCopyright=Copyright (c) 2018 WebMR

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "quicklaunchicon"; Description: "{cm:CreateQuickLaunchIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked; OnlyBelowVersion: 0,6.1

[Files]
Source: "{#ProjectRoot}\*"; DestDir: "{app}"; BeforeInstall: PreInstall; Flags: ignoreversion recursesubdirs createallsubdirs; Excludes: "\dist"

[Registry]
Root: HKCR; Subkey: "Exokit"; ValueType: "string"; ValueData: "URL:Exokit Protocol"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Exokit"; ValueType: "string"; ValueName: "URL Protocol"; ValueData: ""
Root: HKCR; Subkey: "Exokit\DefaultIcon"; ValueType: "string"; ValueData: "{app}\scripts\{#MyAppExeName},0"
Root: HKCR; Subkey: "Exokit\shell\open\command"; ValueType: "string"; ValueData: """{app}\scripts\{#MyAppExeName}"" ""%1"""

[Icons]
Name: "{group}\{#MyAppName}"; WorkingDir: "{%userprofile}"; Filename: "{app}\scripts\{#MyAppExeName}"; Parameters: "-h -l"; IconFilename: "{app}\metadata\{#MyIcon}"
Name: "{group}\{#MyAppShortName} Engine CLI"; WorkingDir: "{%userprofile}"; Filename: "{app}\scripts\{#MyAppExeName}"; Parameters: "-l"; IconFilename: "{app}\metadata\{#MyIcon}"
Name: "{group}\Uninstall {#MyAppShortName}"; Filename: "{uninstallexe}"

[Code]
var
  SymlinkPage: TInputDirWizardPage;

function IsDirEmpty(dir: string): Boolean;
var
  FindRec: TFindRec;
  ct: Integer;
begin
  ct := 0;
  if FindFirst(ExpandConstant(dir + '\*'), FindRec) then
  try
    repeat
      if FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY = 0 then
        ct := ct+1;
    until
      not FindNext(FindRec);
  finally
    FindClose(FindRec);
    Result := ct = 0;
  end;
end;

//function getInstalledVErsions(dir: string):
var
  nodeInUse: string;

function TakeControl(np: string; nv: string): string;
var
  path: string;
begin
  // Move the existing node.js installation directory to the nvm root & update the path
  RenameFile(np,ExpandConstant('{app}')+'\'+nv);

  RegQueryStringValue(HKEY_LOCAL_MACHINE,
    'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
    'Path', path);

  StringChangeEx(path,np+'\','',True);
  StringChangeEx(path,np,'',True);
  StringChangeEx(path,np+';;',';',True);

  RegWriteExpandStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', path);

  RegQueryStringValue(HKEY_CURRENT_USER,
    'Environment',
    'Path', path);

  StringChangeEx(path,np+'\','',True);
  StringChangeEx(path,np,'',True);
  StringChangeEx(path,np+';;',';',True);

  RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', path);

  nodeInUse := ExpandConstant('{app}')+'\'+nv;

end;

function Ansi2String(AString:AnsiString):String;
var
 i : Integer;
 iChar : Integer;
 outString : String;
begin
 outString :='';
 for i := 1 to Length(AString) do
 begin
  iChar := Ord(AString[i]); //get int value
  outString := outString + Chr(iChar);
 end;

 Result := outString;
end;

procedure PreInstall();
var
  TmpResultFile, TmpJS, NodeVersion, NodePath: string;
  stdout: Ansistring;
  ResultCode: integer;
  msg1, msg2, msg3, dir1: Boolean;
begin
//   // Create a file to check for Node.JS
//   TmpJS := ExpandConstant('{tmp}') + '\nvm_check.js';
//   SaveStringToFile(TmpJS, 'console.log(require("path").dirname(process.execPath));', False);
//
//   // Execute the node file and save the output temporarily
//   TmpResultFile := ExpandConstant('{tmp}') + '\nvm_node_check.txt';
//   Exec(ExpandConstant('{cmd}'), '/C node "'+TmpJS+'" > "' + TmpResultFile + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
//   DeleteFile(TmpJS)
//
//   // Process the results
//   LoadStringFromFile(TmpResultFile,stdout);
//   NodePath := Trim(Ansi2String(stdout));
//   if DirExists(NodePath) then begin
//     Exec(ExpandConstant('{cmd}'), '/C node -v > "' + TmpResultFile + '"', '', SW_HIDE, ewWaitUntilTerminated, ResultCode);
//     LoadStringFromFile(TmpResultFile, stdout);
//     NodeVersion := Trim(Ansi2String(stdout));
//     msg1 := MsgBox('Node '+NodeVersion+' is already installed. Do you want NVM to control this version?', mbConfirmation, MB_YESNO) = IDNO;
//     if msg1 then begin
//       msg2 := MsgBox('NVM cannot run in parallel with an existing Node.js installation. Node.js must be uninstalled before NVM can be installed, or you must allow NVM to control the existing installation. Do you want NVM to control node '+NodeVersion+'?', mbConfirmation, MB_YESNO) = IDYES;
//       if msg2 then begin
//         TakeControl(NodePath, NodeVersion);
//       end;
//       if not msg2 then begin
//         DeleteFile(TmpResultFile);
//         WizardForm.Close;
//       end;
//     end;
//     if not msg1 then
//     begin
//       TakeControl(NodePath, NodeVersion);
//     end;
//   end;
//
//   // Make sure the symlink directory doesn't exist
//   if DirExists(SymlinkPage.Values[0]) then begin
//     // If the directory is empty, just delete it since it will be recreated anyway.
//     dir1 := IsDirEmpty(SymlinkPage.Values[0]);
//     if dir1 then begin
//       RemoveDir(SymlinkPage.Values[0]);
//     end;
//     if not dir1 then begin
//       msg3 := MsgBox(SymlinkPage.Values[0]+' will be overwritten and all contents will be lost. Do you want to proceed?', mbConfirmation, MB_OKCANCEL) = IDOK;
//       if msg3 then begin
//         RemoveDir(SymlinkPage.Values[0]);
//       end;
//       if not msg3 then begin
//         //RaiseException('The symlink cannot be created due to a conflict with the existing directory at '+SymlinkPage.Values[0]);
//         WizardForm.Close;
//       end;
//     end;
//   end;
end;

procedure InitializeWizard;
begin
//   SymlinkPage := CreateInputDirPage(wpSelectDir,
//     'Set Node.js Symlink', 'The active version of Node.js will always be available here.',
//     'Select the folder in which Setup should create the symlink, then click Next.',
//     False, '');
//   SymlinkPage.Add('This directory will automatically be added to your system path.');
//   SymlinkPage.Values[0] := ExpandConstant('{pf}\nodejs');
end;

function InitializeUninstall(): Boolean;
var
  path: string;
  nvm_symlink: string;
begin
//   MsgBox('Removing NVM for Windows will remove the nvm command and all versions of node.js, including global npm modules.', mbInformation, MB_OK);
//
//   // Remove the symlink
//   RegQueryStringValue(HKEY_LOCAL_MACHINE,
//     'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
//     'NVM_SYMLINK', nvm_symlink);
//   RemoveDir(nvm_symlink);
//
//   // Clean the registry
//   RegDeleteValue(HKEY_LOCAL_MACHINE,
//     'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
//     'NVM_HOME')
//   RegDeleteValue(HKEY_LOCAL_MACHINE,
//     'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
//     'NVM_SYMLINK')
//   RegDeleteValue(HKEY_CURRENT_USER,
//     'Environment',
//     'NVM_HOME')
//   RegDeleteValue(HKEY_CURRENT_USER,
//     'Environment',
//     'NVM_SYMLINK')
//
//   RegQueryStringValue(HKEY_LOCAL_MACHINE,
//     'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
//     'Path', path);
//
//   StringChangeEx(path,'%NVM_HOME%','',True);
//   StringChangeEx(path,'%NVM_SYMLINK%','',True);
//   StringChangeEx(path,';;',';',True);
//
//   RegWriteExpandStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', path);
//
//   RegQueryStringValue(HKEY_CURRENT_USER,
//     'Environment',
//     'Path', path);
//
//   StringChangeEx(path,'%NVM_HOME%','',True);
//   StringChangeEx(path,'%NVM_SYMLINK%','',True);
//   StringChangeEx(path,';;',';',True);
//
//   RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', path);

  Result := True;
end;

// Generate the settings file based on user input & update registry
procedure CurStepChanged(CurStep: TSetupStep);
var
  path: string;
begin
  if CurStep = ssPostInstall then
  begin
//     SaveStringToFile(ExpandConstant('{app}\settings.txt'), 'root: ' + ExpandConstant('{app}') + #13#10 + 'path: ' + SymlinkPage.Values[0] + #13#10, False);
//
//     // Add Registry settings
//     RegWriteExpandStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'NVM_HOME', ExpandConstant('{app}'));
//     RegWriteExpandStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'NVM_SYMLINK', SymlinkPage.Values[0]);
//     RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'NVM_HOME', ExpandConstant('{app}'));
//     RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'NVM_SYMLINK', SymlinkPage.Values[0]);
//
//     // Update system and user PATH if needed
//     RegQueryStringValue(HKEY_LOCAL_MACHINE,
//       'SYSTEM\CurrentControlSet\Control\Session Manager\Environment',
//       'Path', path);
//     if Pos('%NVM_HOME%',path) = 0 then begin
//       path := path+';%NVM_HOME%';
//       StringChangeEx(path,';;',';',True);
//       RegWriteExpandStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', path);
//     end;
//     if Pos('%NVM_SYMLINK%',path) = 0 then begin
//       path := path+';%NVM_SYMLINK%';
//       StringChangeEx(path,';;',';',True);
//       RegWriteExpandStringValue(HKEY_LOCAL_MACHINE, 'SYSTEM\CurrentControlSet\Control\Session Manager\Environment', 'Path', path);
//     end;
//     RegQueryStringValue(HKEY_CURRENT_USER,
//       'Environment',
//       'Path', path);
//     if Pos('%NVM_HOME%',path) = 0 then begin
//       path := path+';%NVM_HOME%';
//       StringChangeEx(path,';;',';',True);
//       RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', path);
//     end;
//     if Pos('%NVM_SYMLINK%',path) = 0 then begin
//       path := path+';%NVM_SYMLINK%';
//       StringChangeEx(path,';;',';',True);
//       RegWriteExpandStringValue(HKEY_CURRENT_USER, 'Environment', 'Path', path);
//     end;
  end;
end;

function getSymLink(o: string): string;
begin
  Result := SymlinkPage.Values[0];
end;

function getCurrentVersion(o: string): string;
begin
  Result := nodeInUse;
end;

function isNodeAlreadyInUse(): boolean;
begin
  Result := Length(nodeInUse) > 0;
end;

// [Run]
// Filename: "{cmd}"; Parameters: "/C ""mklink /D ""{code:getSymLink}"" ""{code:getCurrentVersion}"""" "; Check: isNodeAlreadyInUse; Flags: runhidden;

[UninstallDelete]
// Type: files; Name: "{app}\nvm.exe";
// Type: files; Name: "{app}\elevate.cmd";
// Type: files; Name: "{app}\elevate.vbs";
// Type: files; Name: "{app}\nodejs.ico";
// Type: files; Name: "{app}\settings.txt";
Type: filesandordirs; Name: "{app}";
