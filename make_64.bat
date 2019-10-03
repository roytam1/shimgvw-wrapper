rc /DWIN64 shimgvw.rc
cl /O2 shimgvw.cpp shimgvw.res user32.lib shell32.lib comdlg32.lib shlwapi.lib /link /subsystem:windows