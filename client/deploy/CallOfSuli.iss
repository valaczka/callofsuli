
[Setup]
AppName=Call of Suli
AppVersion={#COSversion}
DefaultDirName={usercf}\Call of Suli
UninstallDisplayIcon={app}\callofsuli.exe
SolidCompression=yes
AllowNoIcons=yes
OutputDir=.\
OutputBaseFilename={#COSexe}
PrivilegesRequired=lowest
LicenseFile=.\build\LICENSE
AppCopyright=Copyright (C) 2012-2022 Valaczka János Pál

[Files]
Source: ".\build\*"; DestDir: "{app}"; Flags: recursesubdirs


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{userprograms}\Call of Suli"; Filename: "{app}\callofsuli.exe"
Name: "{userprograms}\{cm:UninstallProgram,Call of Suli}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\Call of Suli"; Filename: "{app}\callofsuli.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Classes\callofsuli"; ValueType: "string"; ValueData: "URL:Custom Protocol"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\callofsuli"; ValueType: "string"; ValueName: "URL Protocol"; ValueData: ""
Root: HKCU; Subkey: "Software\Classes\callofsuli\DefaultIcon"; ValueType: "string"; ValueData: "{app}\callofsuli.exe,0"
Root: HKCU; Subkey: "Software\Classes\callofsuli\shell\open\command"; ValueType: "string"; ValueData: """{app}\callofsuli.exe"" ""%1"""

[Languages]
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"

[Run]
Filename: "{app}\callofsuli.exe"; Description: "{cm:LaunchProgram,Call of Suli}"; Flags: nowait postinstall skipifsilent
