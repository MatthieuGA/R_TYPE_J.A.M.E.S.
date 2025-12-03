@echo off
powershell -NoProfile -Command "Get-ChildItem -Path client,engine,server,src -Recurse -Include '*.cpp','*.hpp' -File -ErrorAction SilentlyContinue | Where-Object { $_.FullName -notmatch '\\(build|vcpkg_installed|tests|\\.vscode|\\.github)\\' } | ForEach-Object { & python -W ignore::DeprecationWarning -m cpplint --repository=. --quiet --output=vs7 --filter='-legal/copyright,-build/c++17,+build/c++23,-runtime/references' $_.FullName }"
exit /b %ERRORLEVEL%
