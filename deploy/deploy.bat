echo off

for /f "delims=" %%a in ('dir /b/a-d/oN v*.*.*.txt') do set VERSION=%%~na

set OUT_PATH=E:\fu_tools\BigScren_%VERSION%\
echo %OUT_PATH%
rem pause

set BIN_PATH=%OUT_PATH%bin\

set SRC_BIN_PATH=..\x64\Release\

echo %SRC_BIN_PATH%


md %BIN_PATH%
copy %SRC_BIN_PATH%BigScreen.exe %BIN_PATH% /Y


copy %SRC_BIN_PATH%libsgemm.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%cnnmobile.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%dde_core.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%nama.dll %BIN_PATH% /Y

rem CEGUI
copy %SRC_BIN_PATH%CEGUI*.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%gl*.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%expat.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%freetype.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%jpeg.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%libpng.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%pcre.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%SILLY.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%zlib.dll %BIN_PATH% /Y

rem opencv
xcopy %SRC_BIN_PATH%platforms %BIN_PATH%platforms /Y /S
copy %SRC_BIN_PATH%opencv_highgui310.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%opencv_video310.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%opencv_videoio310.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%opencv_imgproc310.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%opencv_imgcodecs310.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%opencv_core310.dll %BIN_PATH% /Y
copy %SRC_BIN_PATH%Qt5*.dll %BIN_PATH% /Y

xcopy ..\resources %OUT_PATH%resources\ /Y /E

explorer %OUT_PATH%