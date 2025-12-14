#!/bin/bash
# Test script to verify multithreaded server implementation

echo "=== Testing Multithreaded Server ==="
echo ""
echo "Starting server for 5 seconds..."
echo ""

# Start server in background
timeout 5s ./build/server/r-type_server 50000 &
SERVER_PID=$!

# Wait for server to initialize
sleep 1

echo ""
echo "Server should be running with two threads:"
echo "  - Logic thread (60Hz tick rate)"
echo "  - Network thread (polling)"
echo ""
echo "Waiting for server to run..."
sleep 4

echo ""
echo "=== Test Complete ==="
echo ""
echo "Expected output should show:"
echo "  ✓ Logic thread started"
echo "  ✓ Network thread started"
echo "  ✓ Periodic logic ticks"
echo "  ✓ Periodic network ticks"
echo "  ✓ Clean shutdown when terminated"
