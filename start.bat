@echo off
cd /d %~dp0
echo Building code.exe...
g++ -std=c++17 -O2 -o code.exe src\main.cpp
if errorlevel 1 ( echo Build failed. & pause & exit /b 1 )
echo Starting server at http://localhost:3000 ...
start "" http://localhost:3000
node web\server.js
