@echo off
mkdir .build
pushd .build

set Files=../ScreenColors.cpp ScreenColors.res
set MsvcLinkFlags=-incremental:no -opt:ref -machine:x64 -manifest:no
set MsvcCompileFlags=-nologo -Zi -Zo -Gy -GF -GR- -EHs- -EHc- -EHa- -WX -W4 -FC -diagnostics:column -fp:except- -fp:fast -wd4100 -wd4189 -wd4201 -wd4505 -wd4996
set Common=%MsvcCompileFlags% %Files% -link %MsvcLinkFlags%

echo ---- Building resources ----
rc.exe -nologo -fo ScreenColors.res ../ScreenColors.rc

echo ---- Building debug ----
set DebugName=ScreenColors_DebugMsvc
call cl -Fe%DebugName% -Fo%DebugName% -Od %Common%

echo ---- Building release ----
set ReleaseName=ScreenColors_ReleaseMsvc
call cl -Fe%ReleaseName% -Fo%ReleaseName% -O2 %Common%

popd
