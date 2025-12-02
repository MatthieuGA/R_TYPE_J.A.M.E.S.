@echo off
powershell cmake --build build --config Debug; ctest --test-dir build --output-on-failure