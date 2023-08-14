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
ECHO 1	- LED Test
ECHO.
ECHO 2x	- set motor function x {0...7}
ECHO.
ECHO 3xxxx	- set motor rpm xxxx {0000...3000}
ECHO.
ECHO 4	- Endswitch Test
ECHO.
ECHO 5	- Motor Test
ECHO 511	- OUT1: start rpm measurement
ECHO 512	- OUT1: get rpm value
ECHO 52	- OUT2: get motor error
ECHO 53	- OUT3: get motor direction
ECHO.
ECHO 6	- Button Test
ECHO.
ECHO 7	- FRAM Test

ECHO.
SET /P T=selection:
CLS
IF %T%==0 GOTO STARTMENU
SET T=00000%T%
SET T=%T:~-5%
ECHO %T%
SET /P x=%T% <nul >\\.\%C%
GOTO TESTMENU

:EXIT