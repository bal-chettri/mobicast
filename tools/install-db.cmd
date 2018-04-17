@rem Copyright (C) 2018  Bal Chettri
@rem Licensed under GNU GPL(https://www.gnu.org/licenses/gpl.html)

@echo off
@rem Installs mobicast.db to ..\bin directory.

echo WARNING ..\bin\mobicast.db will be replaced!
pause

if NOT exist mobicast.db (
  echo mobicast.db does not exist.
  exit /B 1
)

if not exist ..\bin\ (
    mkdir ..\bin
)

copy mobicast.db ..\bin\
