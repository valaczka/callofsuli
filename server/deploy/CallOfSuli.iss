
[Setup]
AppName=Call of Suli Server
AppVersion={#COSversion}
DefaultDirName={usercf}\Call of Suli Server
UninstallDisplayIcon={app}\callofsuli-server.exe
SolidCompression=yes
AllowNoIcons=yes
OutputDir=..
OutputBaseFilename={#COSexe}
PrivilegesRequired=lowest
LicenseFile=LICENSE
AppCopyright=Copyright (C) 2012-2026 Valaczka János Pál
DisableDirPage=auto
DisableProgramGroupPage=auto
MinVersion={#WinMinVersion}
OnlyBelowVersion={#WinMaxVersion}

[Files]
Source: "*"; DestDir: "{app}"; Excludes: "CallOfSuli.iss"; Flags: recursesubdirs

[InstallDelete]
Type: filesandordirs; Name: "{app}\*"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{userprograms}\Call of Suli Server"; Filename: "{app}\callofsuli-server.exe"
Name: "{userprograms}\{cm:UninstallProgram,Call of Suli Server}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\Call of Suli Server"; Filename: "{app}\callofsuli-server.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Languages]
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"

[Run]
Filename: "{app}\callofsuli-server.exe"; Description: "{cm:LaunchProgram,Call of Suli Server}"; Flags: nowait postinstall skipifsilent
