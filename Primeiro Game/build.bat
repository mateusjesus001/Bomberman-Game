@echo off
REM Build script for Bomberman 8-bit (Windows/MinGW + SDL2)
REM Ensure SDL2 is installed and adjust paths if needed.

set SDL_INCLUDE=C:\msys64\mingw64\include\SDL2
set SDL_INCLUDE2=C:\msys64\mingw64\include
set SDL_LIB=C:\msys64\mingw64\lib

echo Compiling Bomberman 8-bit...
gcc -o bomberman.exe main.c ^
    src\map.c src\player.c src\bomb.c src\explosion.c ^
    src\item.c src\enemy.c src\hud.c src\audio.c ^
    src\render.c src\game.c ^
    -I"%SDL_INCLUDE%" -I"%SDL_INCLUDE2%" -L"%SDL_LIB%" ^
    -lmingw32 -lSDL2main -lSDL2

if %errorlevel%==0 (
    echo Build successful: bomberman.exe
    REM Copy SDL2.dll next to the executable if not already present
    if not exist SDL2.dll (
        if exist "%SDL_LIB%\..\bin\SDL2.dll" (
            copy /Y "%SDL_LIB%\..\bin\SDL2.dll" SDL2.dll >nul
            echo SDL2.dll copied.
        ) else (
            echo WARNING: SDL2.dll not found. Copy it manually next to bomberman.exe
        )
    )
) else (
    echo Build FAILED.
    exit /b 1
)
