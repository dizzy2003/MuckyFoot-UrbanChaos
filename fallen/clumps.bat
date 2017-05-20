deltree /Y clumps
mkdir clumps
deltree /Y server\textures
deltree /Y server\__textures
deltree /Y server\prims
deltree /Y data
xcopy /E /Y /I N:\UrbanChaos\THEGAME\server\textures server\textures
xcopy /E /Y /I N:\UrbanChaos\THEGAME\server\prims server\prims
xcopy /E /Y /I N:\UrbanChaos\THEGAME\data data
deltree /Y levels
xcopy /E /Y /I N:\UrbanChaos\THEGAME\levels levels
rename config.ini _config.ini
rename mcconfig.ini config.ini
debug\fallen.exe
@echo When Fallen.exe quits, run cdsrc.bat
