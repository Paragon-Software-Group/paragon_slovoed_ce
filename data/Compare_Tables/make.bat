for %%f in (*.txt_u) do @echo %%~nf >> list_of_tables.txt
for /F %%i in (list_of_tables.txt) do (UnicodeCMP.exe %%i.txt_u %%i.ucmp)
del "list_of_tables.txt"
pause