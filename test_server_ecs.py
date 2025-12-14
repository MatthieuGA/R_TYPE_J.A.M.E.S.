#!/usr/bin/env python3
"""
Simple R-Type Server ECS Manual Test
This script spawns entities and monitors the server's ECS behavior
"""

import time
import sys

def print_header(text):
    print(f"\n{'='*60}")
    print(f"  {text}")
    print(f"{'='*60}\n")

def main():
    print_header("R-Type Server ECS Manual Test")

    print("This test will help you verify the ECS is working correctly.")
    print("\nWhat to check:")
    print("1. The server starts without errors")
    print("2. Component registration completes")
    print("3. Systems are registered")
    print("4. The server runs the game loop")
    print("\nTo test entity spawning and movement:")
    print("- Use the C++ interactive test below")
    print("- Or connect a client that creates entities")
    print("\nPress Ctrl+C to stop the server\n")

    print("Start the server with: ./run_server.sh")
    print("\nFor interactive testing, compile and run: ./build/tests/server_manual_test")

if __name__ == "__main__":
    main()
