;; This assumes that you are building soource and have the OPeNDAP Tools
;; installed in the default location.
;;
;; Subsequently hacked by jhrg 24 April 2008

[Setup]
AppName=Matlab OPeNDAP loaddap
AppVerName=Matlab OPeNDAP loaddap 3.6.1
AppPublisher=OPeNDAP
DefaultDirName={sd}\opendap
DefaultGroupName=Matlab OPeNDAP loaddap
AllowNoIcons=yes
InfoBeforeFile=BeforeInstall.txt
InfoAfterFile=AfterInstall.txt
OutputBaseFilename=loaddap_3.6.1
Compression=lzma/ultra
SolidCompression=yes
LicenseFile=License.txt

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"

;; The source paths must be absolute and include a drive letter. You'll probably need to edit this and set
;; these to 'C:' but I built from code checked out onto E:.
[Files]
;Source: "..\..\prepkg\opendap\libdap\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
Source: "..\..\prepkg\opendap\libdap\dll\*"; DestDir: "{app}\loaddap"; Flags: ignoreversion
Source: "..\..\prepkg\opendap\ml-structs\bin\*"; DestDir: "{app}\loaddap"; Flags: ignoreversion
Source: "..\..\prepkg\opendap\ml-structs\dll\*"; DestDir: "{app}\loaddap"; Flags: ignoreversion

Source: "C:\opendap-tools\dll\*"; DestDir: "{app}\loaddap"; Flags: ignoreversion

;; These are the VC++ DLLs needed to actually run the code
Source: "C:\Program Files\Microsoft Visual Studio 9.0\VC\redist\x86\*"; DestDir: "{app}\loaddap"; Flags: ignoreversion recursesubdirs createallsubdirs

;; This somewhat inscrutable code sets the 'Path' environment variable so users can run getdap, et c.,
;; In a shell (cmd.exe).
[Registry]
Root: HKLM; Subkey: "SYSTEM\CurrentControlSet\Control\Session Manager\Environment"; ValueType: string; ValueName: "Path"; ValueData: "{olddata};{app}\loaddap"

[Icons]
Name: "{group}\{cm:UninstallProgram,Matlab OPeNDAP loaddap}"; Filename: "{uninstallexe}"



