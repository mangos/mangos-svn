# Microsoft Developer Studio Generated NMAKE File, Based on httpd.dsp
!IF "$(CFG)" == ""
CFG=Null httpd - Win32 Release
!MESSAGE No configuration specified. Defaulting to Null httpd - Win32 Release.
!ENDIF 

!IF "$(CFG)" != "Null httpd - Win32 Release"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "httpd.mak" CFG="Null httpd - Win32 Release"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "Null httpd - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
OUTDIR=.\..\httpd\bin
INTDIR=.\obj
# Begin Custom Macros
OutDir=.\..\httpd\bin
# End Custom Macros

!IF "$(RECURSE)" == "0" 

ALL : "$(OUTDIR)\httpd.exe"

!ELSE 

ALL : "$(OUTDIR)\httpd.exe"

!ENDIF 

CLEAN :
	-@erase "$(INTDIR)\cgi.obj"
	-@erase "$(INTDIR)\config.obj"
	-@erase "$(INTDIR)\files.obj"
	-@erase "$(INTDIR)\format.obj"
	-@erase "$(INTDIR)\http.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\server.obj"
	-@erase "$(INTDIR)\vc50.idb"
	-@erase "$(INTDIR)\win32.obj"
	-@erase "$(OUTDIR)\httpd.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\httpd.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib libcmt.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /pdb:none /machine:I386\
 /nodefaultlib:"libc" /out:"$(OUTDIR)\httpd.exe" 
LINK32_OBJS= \
	"$(INTDIR)\cgi.obj" \
	"$(INTDIR)\config.obj" \
	"$(INTDIR)\files.obj" \
	"$(INTDIR)\format.obj" \
	"$(INTDIR)\http.obj" \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\server.obj" \
	"$(INTDIR)\win32.obj"

"$(OUTDIR)\httpd.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS"\
 /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
CPP_OBJS=.\obj/
CPP_SBRS=.

.c{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_OBJS)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(CPP_SBRS)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /o NUL /win32 

!IF "$(CFG)" == "Null httpd - Win32 Release"
SOURCE=.\cgi.c
DEP_CPP_CGI_C=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\cgi.obj" : $(SOURCE) $(DEP_CPP_CGI_C) "$(INTDIR)"


SOURCE=.\config.c
DEP_CPP_CONFI=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\config.obj" : $(SOURCE) $(DEP_CPP_CONFI) "$(INTDIR)"


SOURCE=.\files.c
DEP_CPP_FILES=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\files.obj" : $(SOURCE) $(DEP_CPP_FILES) "$(INTDIR)"


SOURCE=.\format.c
DEP_CPP_FORMA=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\format.obj" : $(SOURCE) $(DEP_CPP_FORMA) "$(INTDIR)"


SOURCE=.\http.c
DEP_CPP_HTTP_=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\http.obj" : $(SOURCE) $(DEP_CPP_HTTP_) "$(INTDIR)"


SOURCE=.\main.c
DEP_CPP_MAIN_=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\main.obj" : $(SOURCE) $(DEP_CPP_MAIN_) "$(INTDIR)"


SOURCE=.\server.c
DEP_CPP_SERVE=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\server.obj" : $(SOURCE) $(DEP_CPP_SERVE) "$(INTDIR)"


SOURCE=.\win32.c
DEP_CPP_WIN32=\
	".\main.h"\
	{$(INCLUDE)}"sys\stat.h"\
	{$(INCLUDE)}"sys\timeb.h"\
	{$(INCLUDE)}"sys\types.h"\
	

"$(INTDIR)\win32.obj" : $(SOURCE) $(DEP_CPP_WIN32) "$(INTDIR)"



!ENDIF 

