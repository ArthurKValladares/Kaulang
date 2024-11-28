@echo off
setlocal

echo:----------------------------------
echo:---- Run from file (test.kau) ----
echo:----------------------------------
.\build\Debug\kaulang test.kau
echo:
echo:---------------------------
echo:---- Run from terminal ----
echo:---------------------------
.\build\Debug\kaulang.exe
