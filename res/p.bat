@echo off

"C:\Program Files (x86)\Microsoft Visual Studio\vs2019\VC\Tools\MSVC\14.27.29110\bin\Hostx64\x86\dumpbin.exe" /ALL ./notepad_test.exe >> log_e.txt

"C:\Program Files (x86)\Microsoft Visual Studio\vs2019\VC\Tools\MSVC\14.27.29110\bin\Hostx64\x86\dumpbin.exe" /ALL ./notepad.exe >> log_c.txt
