wmake /f WIN1.WMK DEBUG=0
@IF ERRORLEVEL 1 goto error

wmake /f WIN1.WMK DEBUG=1
@IF ERRORLEVEL 1 goto error

@goto end

:error
@echo.
@echo	ERROR - Unable to build libraries.
@echo	Examine ErrorLog.txt for details.
@echo.

:end
