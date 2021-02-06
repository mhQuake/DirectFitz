attrib *.ncb -r -a -s -h /s /d
attrib *.sdf -r -a -s -h /s /d
attrib *.suo -r -a -s -h /s /d
attrib *.user -r -a -s -h /s /d

del *.ncb /s /q
del *.sdf /s /q
del *.suo /s /q
del *.user /s /q

rmdir ipch /s /q
rmdir Debug_D3D9 /s /q
rmdir Release_D3D9 /s /q
rmdir Debug_OpenGL /s /q
rmdir Release_OpenGL /s /q

cd DF

rmdir Debug_D3D9 /s /q
rmdir Release_D3D9 /s /q
rmdir Debug_OpenGL /s /q
rmdir Release_OpenGL /s /q

cd ..

echo done
pause
