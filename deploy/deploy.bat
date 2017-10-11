echo off

for /f "delims=" %%a in ('dir /b/a-d/oN v*.*.*.txt') do set VERSION=%%~na

set OUT_PATH=E:\fu_tools\BSOptimise_GUI_%VERSION%\
echo %OUT_PATH%
rem pause

md %OUT_PATH%
md %OUT_PATH%bin
copy *.glsl %OUT_PATH%bin\ /Y
copy face_rig_template.json %OUT_PATH%bin\ /Y

copy ..\x64\release\BSOptimise_GUI.exe %OUT_PATH%bin\ /Y
copy ..\x64\release\LibBSOptimise.dll %OUT_PATH%bin\ /Y

copy ..\x64\release\nama.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\cnnmobile.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\dde_core.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\libsgemm.dll %OUT_PATH%bin\ /Y

copy ..\x64\release\mkl_*.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\libiomp5*.dll %OUT_PATH%bin\ /Y
md %OUT_PATH%bin\1033
copy ..\x64\release\1033\*.* %OUT_PATH%bin\1033\ /Y

copy ..\x64\release\opencv_highgui310.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\opencv_videoio310.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\opencv_imgproc310.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\opencv_imgcodecs310.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\opencv_core310.dll %OUT_PATH%bin\ /Y
copy ..\x64\release\Qt5*.dll %OUT_PATH%bin\ /Y
md %OUT_PATH%bin\platforms
copy ..\x64\release\platforms\*.* %OUT_PATH%bin\platforms\ /Y

copy ..\x64\release\glew32.dll %OUT_PATH%bin\ /Y

md %OUT_PATH%resources
copy ..\resources\*.bundle %OUT_PATH%resources\ /Y

rem sample data

md %OUT_PATH%example
md %OUT_PATH%example\bs
copy ..\data\projects\bs\*.obj %OUT_PATH%example\bs\ /Y
md %OUT_PATH%example\train_data
copy ..\data\projects\train_data\*.* %OUT_PATH%example\train_data\ /Y
