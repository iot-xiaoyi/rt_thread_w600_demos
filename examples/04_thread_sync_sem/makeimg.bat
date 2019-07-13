@rem ����0: exe
@rem ����1: ����bin�ļ� ,ԭʼ�ļ�����ѹ�����ļ�
@rem ����2: ����ļ�(Ŀ�������ļ���
@rem ����3: �����ļ�����,0��image�ļ���1��FLASHBOOT�ļ� 2��secboot�ļ�
@rem ����4: �Ƿ�ѹ���ļ���0��plain�ļ���1��ѹ�������ļ�
@rem ����5: �汾���ļ�
@rem ����6�������ļ���FLASH��Ĵ��λ�ã����λ�ã�
@rem ����7����������ļ�����λ�ã����λ�ã�
@rem ����8��ԭʼbin�ļ�

@echo off

set wm_librarie_path=..\..\libraries\WM_Libraries
set out_path=.\Bin
set bin_file=.\rtthread.bin

set debug_info=0

if "%wm_librarie_path:~0,1%" == "." (set wm_librarie_path=%~dp0%wm_librarie_path%)
if "%out_path:~0,1%" == "." (set out_path=%~dp0%out_path%)
if "%bin_file:~0,1%" == "." (set bin_file=%~dp0%bin_file%)

set version_file=%wm_librarie_path%\rtthread\version.txt
set secboot_file=%wm_librarie_path%\rtthread\secboot.img

set wm_gzip_file=%wm_librarie_path%\Tools\wm_gzip.exe
set makeimg_file=%wm_librarie_path%\Tools\makeimg.exe
set makeimg_all_file=%wm_librarie_path%\Tools\makeimg_all.exe

if not exist "%out_path%" (md "%out_path%")

if exist "%bin_file%" (copy "%bin_file%" "%out_path%") else (echo makeimg err! No bin file found: %bin_file% & goto end)
if exist "%version_file%" (copy "%version_file%" "%out_path%") else (echo makeimg err! No version file found: %version_file% & goto end)
if exist "%secboot_file%" (copy "%secboot_file%" "%out_path%") else (echo makeimg err! No secboot file found: %secboot_file% & goto end)

if not exist "%wm_gzip_file%" (echo makeimg err! No wm_gzip file found: "%wm_gzip_file%" & goto end)
if not exist "%makeimg_file%" (echo makeimg err! No makeimg file found: "%makeimg_file%" & goto end)
if not exist "%makeimg_all_file%" (echo makeimg err! No makeimg_all file found: "%makeimg_all_file%" & goto end)

for /f "delims=" %%A in ('dir /b %bin_file%') do set "bin_file_name=%%A"
for /f "delims=." %%A in ('dir /b %bin_file%') do set bin_name=%%A
for /f "delims=%bin_name%" %%A in ('dir /b %bin_file%') do set bin_extend=%%A
for /f "delims=" %%A in ('dir /b %version_file%') do set "version_file_name=%%A"
for /f "delims=" %%A in ('dir /b %secboot_file%') do set "secboot_file_name=%%A"

if not "%debug_info%"=="0" (echo bin_file_name:%bin_file_name% & echo bin_name:%bin_name% & echo bin_extend:%bin_extend% & echo version_file_name:%version_file_name% & echo secboot_file_name:%secboot_file_name%)

echo makeimg...

if not "%debug_info%"=="0" (echo wm_gzip "%out_path%\%bin_file_name%")
if not "%debug_info%"=="0" (echo makeimg "%out_path%\%bin_file_name%" "%out_path%\%bin_name%.img" 0 0 "%out_path%\%version_file_name%" 90000 10100)
if not "%debug_info%"=="0" (echo makeimg "%out_path%\%bin_file_name%.gz" "%out_path%\%bin_name%_GZ.img" 0 1 "%out_path%\%version_file_name%" 90000 10100 "%out_path%\%bin_file_name%")
if not "%debug_info%"=="0" (echo makeimg "%out_path%\%bin_file_name%" "%out_path%\%bin_name%_SEC.img" 0 0 "%out_path%\%version_file_name%" 90000 10100)
if not "%debug_info%"=="0" (echo makeimg_all "%out_path%\%secboot_file_name%" "%out_path%\%bin_name%.img" "%out_path%\%bin_name%.FLS")

"%wm_gzip_file%" "%out_path%\%bin_file_name%"
"%makeimg_file%" "%out_path%\%bin_file_name%" "%out_path%\%bin_name%.img" 0 0 "%out_path%\%version_file_name%" 90000 10100
"%makeimg_file%" "%out_path%\%bin_file_name%.gz" "%out_path%\%bin_name%_GZ.img" 0 1 "%out_path%\%version_file_name%" 90000 10100 "%out_path%\%bin_file_name%" 
"%makeimg_file%" "%out_path%\%bin_file_name%" "%out_path%\%bin_name%_SEC.img" 0 0 "%out_path%\%version_file_name%" 90000 10100
"%makeimg_all_file%" "%out_path%\%secboot_file_name%" "%out_path%\%bin_name%.img" "%out_path%\%bin_name%.FLS"

if exist "%out_path%\rtthread.img" (del "%out_path%\rtthread.img")

:end
echo end
