@echo off

cd ../

rd /Q /S project\MDK-ARM(uV4)\Flash
rd /Q /S project\MDK-ARM(uV4)\CpuRAM
rd /Q /S project\MDK-ARM(uV4)\ExtSRAM
del /Q project\MDK-ARM(uV4)\*.bak
del /Q project\MDK-ARM(uV4)\*.dep
del /Q project\MDK-ARM(uV4)\JLink*
del /Q project\MDK-ARM(uV4)\project.uvgui.*

@echo

@echo on

