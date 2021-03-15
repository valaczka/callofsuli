[Setup]
AppName=Call of Suli
AppVersion=0.21
DefaultDirName={commonpf}\Call of Suli
UninstallDisplayIcon={app}\callofsuli.exe
SolidCompression=yes
AllowNoIcons=yes
OutputDir=.\
OutputBaseFilename=CallOfSuli_0.21.8_install
PrivilegesRequired=lowest

[Files]
Source: ".\out\*"; DestDir: "{app}"; Flags: recursesubdirs

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"

[Icons]
Name: "{userprograms}\Call of Suli"; Filename: "{app}\callofsuli.exe"
Name: "{userprograms}\{cm:UninstallProgram,Call of Suli szerver}"; Filename: "{uninstallexe}"
Name: "{userdesktop}\Call of Suli"; Filename: "{app}\callofsuli.exe"; WorkingDir: "{app}"; Tasks: desktopicon

[Languages]
Name: "hungarian"; MessagesFile: "compiler:Languages\Hungarian.isl"

[Run]
Filename: "{app}\callofsuli.exe"; Description: "{cm:LaunchProgram,Call of Suli}"; Flags: nowait postinstall skipifsilent
