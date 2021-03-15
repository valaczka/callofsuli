[Setup]
AppName=Call of Suli szerver
AppVersion=0.21
DefaultDirName={commonpf}\Call of Suli server
UninstallDisplayIcon={app}\callofsuli-server.exe
SolidCompression=yes
AllowNoIcons=yes
OutputDir=.\
OutputBaseFilename=CallOfSuli_server_0.21.8_install
PrivilegesRequired=lowest

[Files]
Source: ".\out\*"; DestDir: "{app}"; Flags: recursesubdirs

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{userprograms}\Call of Suli szerver"; Filename: "{app}\callofsuli-server.exe"
Name: "{userprograms}\{cm:UninstallProgram,Call of Suli szerver}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\Call of Suli szerver"; Filename: "{app}\callofsuli-server.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Languages]
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"

[Run]
Filename: "{app}\callofsuli-server.exe"; Description: "{cm:LaunchProgram,Call of Suli szerver}"; Flags: nowait postinstall skipifsilent
