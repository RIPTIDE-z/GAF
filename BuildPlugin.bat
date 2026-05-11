@echo off
setlocal EnableExtensions

title Build GAF Plugin

set "SourceBuildVersion=5.7"
set "InstalledBuildVersion=5.7"
set "DefaultEngineDirectory=E:\epic\UE_5.7"

set "ScriptDirectory=%~dp0"
set "PluginPath=%ScriptDirectory%GameAnimationFramework.uplugin"
set "OutputPath=%ScriptDirectory%Build"

if not exist "%PluginPath%" (
    echo Can't find plugin descriptor:
    echo   %PluginPath%
    exit /b 1
)

if defined UE_ENGINE_DIR (
    set "EngineDirectory=%UE_ENGINE_DIR%"
)

if not defined EngineDirectory (
    for /f "skip=2 tokens=2*" %%a in ('reg query "HKEY_CURRENT_USER\Software\Epic Games\Unreal Engine\Builds" /v "%SourceBuildVersion%" 2^>nul') do (
        set "EngineDirectory=%%b"
    )
)

if not defined EngineDirectory (
    for /f "skip=2 tokens=2*" %%a in ('reg query "HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%InstalledBuildVersion%" /v "InstalledDirectory" 2^>nul') do (
        set "EngineDirectory=%%b"
    )
)

if not defined EngineDirectory (
    if exist "%DefaultEngineDirectory%\Engine\Build\BatchFiles\RunUAT.bat" (
        set "EngineDirectory=%DefaultEngineDirectory%"
    )
)

if not defined EngineDirectory (
    echo Can't find a path to Unreal Engine %InstalledBuildVersion%.
    echo Checked:
    echo   UE_ENGINE_DIR
    echo   HKEY_CURRENT_USER\Software\Epic Games\Unreal Engine\Builds /v %SourceBuildVersion%
    echo   HKEY_LOCAL_MACHINE\SOFTWARE\EpicGames\Unreal Engine\%InstalledBuildVersion% /v InstalledDirectory
    echo   %DefaultEngineDirectory%
    exit /b 1
)

set "AutomationToolPath=%EngineDirectory%\Engine\Build\BatchFiles\RunUAT.bat"

if not exist "%AutomationToolPath%" (
    echo Can't find Unreal AutomationTool:
    echo   %AutomationToolPath%
    exit /b 1
)

echo Engine Directory: %EngineDirectory%
echo Automation Tool:  %AutomationToolPath%
echo Plugin:           %PluginPath%
echo Output:           %OutputPath%
echo.

call "%AutomationToolPath%" BuildPlugin -Plugin="%PluginPath%" -Package="%OutputPath%" -Rocket -TargetPlatforms=Win64
set "BuildResult=%ERRORLEVEL%"

echo.
if not "%BuildResult%" == "0" (
    echo GAF plugin build failed with exit code %BuildResult%.
    exit /b %BuildResult%
)

echo GAF plugin build succeeded.
exit /b 0
