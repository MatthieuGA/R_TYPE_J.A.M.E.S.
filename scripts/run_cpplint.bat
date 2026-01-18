@echo off
rem Detect Python launcher or python executable and run cpplint via PowerShell
where py >nul 2>&1
if %ERRORLEVEL%==0 (
	set "PYEXEC=py"
	set "PYARGS=-3"
) else (
	where python3 >nul 2>&1
	if %ERRORLEVEL%==0 (
		set "PYEXEC=python3"
		set "PYARGS="
	) else (
		where python >nul 2>&1
		if %ERRORLEVEL%==0 (
			set "PYEXEC=python"
			set "PYARGS="
		) else (
			echo.
			echo Python was not found. Install Python 3 from https://www.python.org/downloads/ or via the Microsoft Store.
			echo If you previously installed Python, disable the "App execution aliases" for Python in Settings > Apps > Advanced app settings.
			echo Ensure either `py`, `python3` or `python` is on your PATH, then re-run this script.
			exit /b 1
		)
	)
)

:: Verify cpplint is importable
%PYEXEC% %PYARGS% -c "import cpplint" >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
	echo cpplint is not installed for %PYEXEC% %PYARGS%.
	echo Install it with:
	echo   %PYEXEC% %PYARGS% -m pip install --user cpplint
	echo Then re-run this script.
	exit /b 1
)

powershell -NoProfile -Command "Get-ChildItem -Path client,engine,server,src -Recurse -Include '*.cpp','*.hpp' -File -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\(build|vcpkg_installed|tests|\\.vscode|\\.github)\\' } | ForEach-Object { & %PYEXEC% %PYARGS% -W ignore::DeprecationWarning -m cpplint --repository=. --quiet --output=vs7 --filter='-legal/copyright,-build/c++17,+build/c++20,-runtime/references' $_.FullName }"
exit /b %ERRORLEVEL%
