@ECHO OFF
:DEFAULT
SET B=115200
SET C=COM3
GOTO MODE
:COMPORT
SET /P CN=enter Com port number:
SET C=COM%CN%
GOTO MODE
:BAUD
SET /P B=enter Baud rate:
:MODE
mode %C% BAUD=%B% PARITY=n DATA=8
:STARTMENU
ECHO choose and enter option:
ECHO.
ECHO 1 - change COM Port
ECHO 2 - change Baud rate
ECHO 3 - start test
ECHO 4 - EXIT
ECHO.
SET /P S=selection:
CLS
IF %S%==1 GOTO COMPORT
IF %S%==2 GOTO BAUD
IF %S%==3 GOTO TESTMENU
IF %S%==4 GOTO EXIT

:TESTMENU
ECHO choose and enter Test ID
ECHO.
ECHO 0	- back to start menu
ECHO.
ECHO 11	- toggle "Error" LED
ECHO 12	- toggle "sail adjustment mode" LEDs
ECHO 131	- set "manual" LED
ECHO 132	- set "automatic" LED
ECHO 14	- toggle "center pos set" LED
ECHO.
ECHO 2x	- set motor function x {0...7}
ECHO.
ECHO 3xxxx	- set motor rpm xxxx {0000...3000}
ECHO.
ECHO 41	- read endswitch vorne
ECHO 42	- read endswitch hinten
ECHO.
SET /P T=selection:
CLS
IF %T%==0 GOTO STARTMENU
SET T=00000%T%
SET T=%T:~-5%
SET /P x=%T% <nul >\\.\%C%
GOTO TESTMENU
:EXIT