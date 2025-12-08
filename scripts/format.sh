#!/bin/bash

find engine client server tests -type f \( -name "*.cpp" -o -name "*.tpp" -o -name "*.hpp" \) -exec clang-format -i {} +