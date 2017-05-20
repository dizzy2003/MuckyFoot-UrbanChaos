cd "c:\urban chaos installer source files"
deltree /Y English\levels
deltree /Y Italian\levels
deltree /Y Spanish\levels
xcopy /E /Y /I c:\fallen\levels English\levels
xcopy /E /Y /I \\sanctuary\spanish Spanish\levels
xcopy /E /Y /I \\sanctuary\italian Italian\levels
deltree /Y server
mkdir server
xcopy /E /Y /I c:\fallen\server\prims server\prims
xcopy /E /Y /I c:\fallen\server\textures server\textures
deltree /Y clumps
xcopy /E /Y /I c:\fallen\clumps clumps
deltree /Y bink
deltree /Y data
deltree /Y stars
deltree /Y talk2
deltree /Y text
xcopy /E /Y /I N:\UrbanChaos\THEGAME\bink bink
xcopy /E /Y /I c:\fallen\data data
xcopy /E /Y /I N:\UrbanChaos\THEGAME\stars stars
xcopy /E /Y /I N:\UrbanChaos\THEGAME\talk2 talk2
xcopy /E /Y /I N:\UrbanChaos\THEGAME\text text
call c:\fallen\compress.bat
