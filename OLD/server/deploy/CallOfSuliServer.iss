
[Setup]
AppName=Call of Suli server
AppVersion={#COSversion}
DefaultDirName={usercf}\Call of Suli server
UninstallDisplayIcon={app}\callofsuli-server.exe
SolidCompression=yes
AllowNoIcons=yes
OutputDir=.\
OutputBaseFilename={#COSexe}
PrivilegesRequired=lowest
LicenseFile=.\build\LICENSE
AppCopyright=Copyright (C) 2012-2022 Valaczka János Pál
DisableDirPage=auto
DisableProgramGroupPage=auto

[Files]
Source: ".\build\*"; DestDir: "{app}"; Flags: recursesubdirs


[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{userprograms}\Call of Suli szerver"; Filename: "{app}\callofsuli-server.exe"
Name: "{userprograms}\{cm:UninstallProgram,Call of Suli server}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\Call of Suli szerver"; Filename: "{app}\callofsuli-server.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Registry]
Root: HKCU; Subkey: "Software\Classes\callofsuli-server"; ValueType: "string"; ValueData: "URL:Custom Protocol"; Flags: uninsdeletekey
Root: HKCU; Subkey: "Software\Classes\callofsuli-server"; ValueType: "string"; ValueName: "URL Protocol"; ValueData: ""
Root: HKCU; Subkey: "Software\Classes\callofsuli-server\DefaultIcon"; ValueType: "string"; ValueData: "{app}\callofsuli-server.exe,0"
Root: HKCU; Subkey: "Software\Classes\callofsuli-server\shell\open\command"; ValueType: "string"; ValueData: """{app}\callofsuli-server.exe"" ""%1"""

[Languages]
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"

[Run]
Filename: "{app}\callofsuli-server.exe"; Description: "{cm:LaunchProgram,Call of Suli server}"; Flags: nowait postinstall skipifsilent
