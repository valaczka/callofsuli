
[Setup]
AppName=Call of Suli
AppVersion={#COSversion}
DefaultDirName={usercf}\Call of Suli
UninstallDisplayIcon={app}\callofsuli.exe
SolidCompression=yes
AllowNoIcons=yes
OutputDir=..
OutputBaseFilename={#COSexe}
PrivilegesRequired=lowest
LicenseFile=LICENSE
AppCopyright=Copyright (C) 2012-2024 Valaczka János Pál
DisableDirPage=auto
DisableProgramGroupPage=auto
MinVersion={#WinMinVersion}
OnlyBelowVersion={#WinMaxVersion}

[Files]
Source: "*"; DestDir: "{app}"; Excludes: "CallOfSuli.iss"; Flags: recursesubdirs


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


#pragma parseroption -p-
#define FileEntry(DestDir) \
	"  FilesNotToBeDeleted.Add('" + LowerCase(DestDir) + "');\n"
#define ProcessFile(Source, Dest, FindResult, FindHandle) \
	FindResult \
		? \
			Local[0] = FindGetFileName(FindHandle), \
			Local[1] = Source + "\\" + Local[0], \
			Local[2] = Dest + "\\" + Local[0], \
			(Local[0] != "." && Local[0] != ".." \
				? FileEntry(Local[2]) + \
				  (DirExists(Local[1]) ? ProcessFolder(Local[1], Local[2]) : "") \
				: "") + \
			ProcessFile(Source, Dest, FindNext(FindHandle), FindHandle) \
		: \
			""
#define ProcessFolder(Source, Dest) \
	Local[0] = FindFirst(Source + "\\*", faAnyFile), \
	ProcessFile(Source, Dest, Local[0], Local[0])
#pragma parseroption -p+
[Code]
var
  FilesNotToBeDeleted: TStringList;
function InitializeSetup(): Boolean;
begin
  FilesNotToBeDeleted := TStringList.Create;
  FilesNotToBeDeleted.Add('\data');
  FilesNotToBeDeleted.Add('\unins000.dat');
  FilesNotToBeDeleted.Add('\unins000.exe');
  {#Trim(ProcessFolder('.\', ''))}
  FilesNotToBeDeleted.Sorted := True;
  Result := True;
end;
procedure DeleteObsoleteFiles(Path: string; RelativePath: string);
var
  FindRec: TFindRec;
  FilePath: string;
  FileRelativePath: string;
begin
  if FindFirst(Path + '\*', FindRec) then
  begin
	try
	  repeat
		if (FindRec.Name <> '.') and (FindRec.Name <> '..') then
		begin
		  FilePath := Path + '\' + FindRec.Name;
		  FileRelativePath := RelativePath + '\' + FindRec.Name;
		  if FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY <> 0 then
		  begin
			DeleteObsoleteFiles(FilePath, FileRelativePath);
		  end;
		  if FilesNotToBeDeleted.IndexOf(Lowercase(FileRelativePath)) < 0 then
		  begin
			if FindRec.Attributes and FILE_ATTRIBUTE_DIRECTORY <> 0 then
			begin
			  if RemoveDir(FilePath) then
			  begin
				Log(Format('Deleted obsolete directory %s', [FilePath]));
			  end
				else
			  begin
				Log(Format('Failed to delete obsolete directory %s', [FilePath]));
			  end;
			end
			  else
			begin
			  if DeleteFile(FilePath) then
			  begin
				Log(Format('Deleted obsolete file %s', [FilePath]));
			  end
				else
			  begin
				Log(Format('Failed to delete obsolete file %s', [FilePath]));
			  end;
			end;
		  end;
		end;
	  until not FindNext(FindRec);
	finally
	  FindClose(FindRec);
	end;
  end
	else
  begin
	Log(Format('Failed to list %s', [Path]));
  end;
end;
procedure CurStepChanged(CurStep: TSetupStep);
begin
  if CurStep = ssInstall then
  begin
	Log('Looking for obsolete files...');
	DeleteObsoleteFiles(WizardDirValue, '');
  end;
end;
