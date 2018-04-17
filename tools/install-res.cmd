@rem Copyright (C) 2018  Bal Chettri
@rem Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)

@echo OFF
@rem Installs resources to ..\bin

if exist ..\bin\web\ (
    rd /S /Q ..\bin\web
)

if not exist ..\bin\ (
    mkdir ..\bin
)

xcopy /E /Y ..\res ..\bin\

echo Done!
