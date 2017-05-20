cd "c:\urban chaos installer source files"
deltree /Y English\levels
deltree /Y Italian\levels
deltree /Y Spanish\levels
xcopy /E /Y /I \\sanctuary\german English\levels
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
xcopy /E /Y /I N:\UrbanChaos\talk2_german talk2
mkdir text
copy N:\UrbanChaos\THEGAME\text\citsez_german.txt text\citsez.txt
copy N:\UrbanChaos\THEGAME\text\lang_german.txt text\lang_german.txt
call c:\fallen\compress.bat
