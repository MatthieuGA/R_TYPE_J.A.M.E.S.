#!/usr/bin/env python3
"""
Test TCP connection to R-Type server.

This script tests the TCP handshake implementation by:
1. Connecting to the server
2. Sending CONNECT_REQ packets
3. Receiving CONNECT_ACK responses
4. Testing edge cases (server full, duplicate usernames, etc.)

Usage:
    python3 scripts/test_tcp_connection.py [--host HOST] [--port PORT]
"""

import argparse
import socket
import struct
import sys
import time


class Color:
    """ANSI color codes for terminal output."""
    RESET = '\033[0m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'


def build_connect_req(username: str) -> bytes:
    """
    Build a CONNECT_REQ packet (OpCode 0x01).

    Packet format (44 bytes total):
    - Header (12 bytes): op_code(u8) + payload_size(u16) + packet_index(u8) +
                         tick_id(u32) + packet_count(u8) + reserved(3)
    - Payload (32 bytes): username (null-terminated, fixed size)

    Args:
        username: Player username (max 31 characters)

    Returns:
        Complete packet as bytes
    """
    # Truncate and pad username to 32 bytes
    username_bytes = username.encode('utf-8')[:31]  # Max 31 chars + null
    username_bytes = username_bytes.ljust(32, b'\x00')

    # Pack header (little-endian, 12 bytes)
    header = struct.pack(
        '<BHBIB',
        0x01,  # op_code: CONNECT_REQ (1 byte)
        32,    # payload_size: 32 bytes (2 bytes)
        0,     # packet_index: 0 (1 byte)
        0,     # tick_id: 0 (4 bytes)
        1      # packet_count: 1 (1 byte)
    ) + b'\x00\x00\x00'  # reserved: 3 bytes

    return header + username_bytes


def parse_connect_ack(response: bytes) -> tuple[int, int, int]:
    """
    Parse CONNECT_ACK packet (OpCode 0x02).

    Packet format (16 bytes):
    - Header (12 bytes): same as CONNECT_REQ
    - Payload (4 bytes): player_id(u8) + status(u8) + reserved(2)

    Args:
        response: Raw packet bytes

    Returns:
        Tuple of (op_code, player_id, status)
    """
    if len(response) < 16:
        raise ValueError(f"Response too short: {len(response)} bytes")

    # Parse header
    op_code, payload_size = struct.unpack('<BH', response[0:3])

    # Parse payload
    player_id, status = struct.unpack('<BB', response[12:14])

    return op_code, player_id, status


def send_connect_req(host: str, port: int, username: str,
                     timeout: float = 3.0) -> tuple[socket.socket | None, int, int]:
    """
    Connect to server and send CONNECT_REQ.

    Args:
        host: Server hostname/IP
        port: Server TCP port
        username: Player username
        timeout: Socket timeout in seconds

    Returns:
        Tuple of (socket, player_id, status). Socket is None on failure.
    """
    sock = None
    try:
        # Connect to server
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(timeout)
        sock.connect((host, port))

        # Send CONNECT_REQ
        packet = build_connect_req(username)
        sock.send(packet)

        # Receive CONNECT_ACK
        response = sock.recv(1024)
        op_code, player_id, status = parse_connect_ack(response)

        if op_code != 0x02:
            raise ValueError(f"Expected CONNECT_ACK (0x02), got 0x{op_code:02x}")

        return sock if status == 0 else None, player_id, status

    except Exception as e:
        if sock:
            sock.close()
        raise e


def status_name(status: int) -> str:
    """Get human-readable status name."""
    return {
        0: 'OK',
        1: 'ServerFull',
        2: 'BadUsername',
        3: 'InGame'
    }.get(status, f'Unknown({status})')


def test_basic_connection(host: str, port: int):
    """Test basic successful connection."""
    print(f"\n{Color.BOLD}Test 1: Basic Connection{Color.RESET}")
    print("=" * 60)

    try:
        sock, player_id, status = send_connect_req(host, port, "TestPlayer")
        status_str = status_name(status)

        if status == 0:
            print(f"{Color.GREEN}✓{Color.RESET} Connected successfully")
            print(f"  Player ID: {player_id}")
            print(f"  Status: {status_str}")
            sock.close()
            return True
        else:
            print(f"{Color.RED}✗{Color.RESET} Connection rejected")
            print(f"  Status: {status_str}")
            return False

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_multiple_clients(host: str, port: int, num_clients: int = 4):
    """Test connecting multiple clients."""
    print(f"\n{Color.BOLD}Test 2: Multiple Clients (max {num_clients}){Color.RESET}")
    print("=" * 60)

    sockets = []
    success_count = 0

    for i in range(1, num_clients + 1):
        username = f"Player{i}"
        try:
            sock, player_id, status = send_connect_req(host, port, username)
            status_str = status_name(status)

            if status == 0:
                print(f"{Color.GREEN}✓{Color.RESET} {username:12s} → ID={player_id}, {status_str}")
                sockets.append(sock)
                success_count += 1
            else:
                print(f"{Color.YELLOW}!{Color.RESET} {username:12s} → {status_str}")

        except Exception as e:
            print(f"{Color.RED}✗{Color.RESET} {username:12s} → Error: {e}")

        time.sleep(0.1)

    print(f"\n  Connected: {success_count}/{num_clients}")

    # Cleanup
    for sock in sockets:
        sock.close()

    return success_count == num_clients


def test_server_full(host: str, port: int):
    """Test server full rejection (5th client)."""
    print(f"\n{Color.BOLD}Test 3: Server Full{Color.RESET}")
    print("=" * 60)

    # Connect 4 clients
    sockets = []
    for i in range(1, 5):
        try:
            sock, _, status = send_connect_req(host, port, f"Player{i}")
            if status == 0 and sock:
                sockets.append(sock)
        except:
            pass
        time.sleep(0.1)

    print(f"Pre-filled server with {len(sockets)} clients")

    # Try 5th client
    try:
        sock, player_id, status = send_connect_req(host, port, "Player5")
        status_str = status_name(status)

        if status == 1:  # ServerFull
            print(f"{Color.GREEN}✓{Color.RESET} Correctly rejected 5th client")
            print(f"  Status: {status_str}")
            result = True
        else:
            print(f"{Color.RED}✗{Color.RESET} Expected ServerFull, got {status_str}")
            result = False
            if sock:
                sock.close()

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        result = False

    # Cleanup
    for sock in sockets:
        sock.close()

    return result


def test_duplicate_username(host: str, port: int):
    """Test duplicate username rejection."""
    print(f"\n{Color.BOLD}Test 4: Duplicate Username{Color.RESET}")
    print("=" * 60)

    # Connect first client
    try:
        sock1, player_id1, status1 = send_connect_req(host, port, "TestPlayer")
        if status1 != 0:
            print(f"{Color.YELLOW}!{Color.RESET} First connection failed: {status_name(status1)}")
            return False

        print(f"{Color.GREEN}✓{Color.RESET} First 'TestPlayer' connected (ID={player_id1})")

        # Try duplicate
        time.sleep(0.1)
        _, player_id2, status2 = send_connect_req(host, port, "TestPlayer")
        status_str = status_name(status2)

        if status2 == 2:  # BadUsername
            print(f"{Color.GREEN}✓{Color.RESET} Correctly rejected duplicate username")
            print(f"  Status: {status_str}")
            result = True
        else:
            print(f"{Color.RED}✗{Color.RESET} Expected BadUsername, got {status_str}")
            result = False

        sock1.close()
        return result

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_empty_username(host: str, port: int):
    """Test empty username rejection."""
    print(f"\n{Color.BOLD}Test 5: Empty Username{Color.RESET}")
    print("=" * 60)

    try:
        _, player_id, status = send_connect_req(host, port, "")
        status_str = status_name(status)

        if status == 2:  # BadUsername
            print(f"{Color.GREEN}✓{Color.RESET} Correctly rejected empty username")
            print(f"  Status: {status_str}")
            return True
        else:
            print(f"{Color.RED}✗{Color.RESET} Expected BadUsername, got {status_str}")
            return False

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def main():
    """Run all tests."""
    parser = argparse.ArgumentParser(
        description='Test R-Type server TCP connection',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('--host', default='127.0.0.1',
                        help='Server hostname/IP (default: 127.0.0.1)')
    parser.add_argument('--port', type=int, default=50000,
                        help='Server TCP port (default: 50000)')
    parser.add_argument('--test', choices=['basic', 'multi', 'full', 'duplicate', 'empty', 'all'],
                        default='all', help='Specific test to run (default: all)')

    args = parser.parse_args()

    print(f"{Color.BOLD}{Color.BLUE}")
    print("=" * 60)
    print("R-Type Server TCP Connection Test")
    print("=" * 60)
    print(f"{Color.RESET}")
    print(f"Server: {args.host}:{args.port}")

    results = {}

    try:
        if args.test in ('basic', 'all'):
            results['basic'] = test_basic_connection(args.host, args.port)

        if args.test in ('multi', 'all'):
            results['multi'] = test_multiple_clients(args.host, args.port, 4)

        if args.test in ('full', 'all'):
            results['full'] = test_server_full(args.host, args.port)

        if args.test in ('duplicate', 'all'):
            results['duplicate'] = test_duplicate_username(args.host, args.port)

        if args.test in ('empty', 'all'):
            results['empty'] = test_empty_username(args.host, args.port)

    except KeyboardInterrupt:
        print(f"\n\n{Color.YELLOW}Tests interrupted by user{Color.RESET}")
        sys.exit(1)

    # Summary
    print(f"\n{Color.BOLD}Summary{Color.RESET}")
    print("=" * 60)

    passed = sum(1 for v in results.values() if v)
    total = len(results)

    for test_name, result in results.items():
        status = f"{Color.GREEN}PASS{Color.RESET}" if result else f"{Color.RED}FAIL{Color.RESET}"
        print(f"  {test_name:15s} {status}")

    print(f"\nTotal: {passed}/{total} tests passed")

    if passed == total:
        print(f"\n{Color.GREEN}{Color.BOLD}✓ All tests passed!{Color.RESET}")
        sys.exit(0)
    else:
        print(f"\n{Color.RED}{Color.BOLD}✗ Some tests failed{Color.RESET}")
        sys.exit(1)


if __name__ == '__main__':
    main()
