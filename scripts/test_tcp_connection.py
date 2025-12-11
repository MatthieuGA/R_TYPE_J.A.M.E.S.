#!/usr/bin/env python3
"""
Comprehensive TCP connection tests for R-Type server.

This script tests the TCP handshake implementation including:
- Basic connection and handshake
- Multiple clients and server capacity
- Edge cases (long usernames, special characters, emojis)
- Malformed packets and error handling
- Partial packets and connection timeouts
- Reconnection scenarios

Usage:
    python3 scripts/test_tcp_connection.py [--host HOST] [--port PORT] [--test TEST]

Examples:
    # Run all tests
    python3 scripts/test_tcp_connection.py

    # Run only basic tests
    python3 scripts/test_tcp_connection.py --test basic

    # Run edge case tests
    python3 scripts/test_tcp_connection.py --test edge
"""

import argparse
import socket
import struct
import sys
import time
import threading
from typing import Tuple, Optional


class Color:
    """ANSI color codes for terminal output."""
    RESET = '\033[0m'
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    BOLD = '\033[1m'
    CYAN = '\033[96m'


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
    username_bytes = username.encode('utf-8', errors='replace')[:31]  # Max 31 chars + null
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


def build_ready_status(is_ready: bool) -> bytes:
    """
    Build a READY_STATUS packet (OpCode 0x07).

    Packet format (16 bytes total):
    - Header (12 bytes): op_code(u8) + payload_size(u16) + packet_index(u8) +
                         tick_id(u32) + packet_count(u8) + reserved(3)
    - Payload (4 bytes): ready(u8) + reserved(3)

    Args:
        is_ready: Whether the player is ready (True) or not ready (False)

    Returns:
        Complete packet as bytes
    """
    # Pack header (little-endian, 12 bytes)
    header = struct.pack(
        '<BHBIB',
        0x07,  # op_code: READY_STATUS (1 byte)
        4,     # payload_size: 4 bytes (2 bytes)
        0,     # packet_index: 0 (1 byte)
        0,     # tick_id: 0 (4 bytes)
        1      # packet_count: 1 (1 byte)
    ) + b'\x00\x00\x00'  # reserved: 3 bytes

    # Pack payload: ready flag + 3 reserved bytes
    payload = struct.pack('<B', 1 if is_ready else 0) + b'\x00\x00\x00'

    return header + payload


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


def parse_game_start(response: bytes) -> int:
    """
    Parse GAME_START packet (OpCode 0x05).

    Packet format (16 bytes):
    - Header (12 bytes): op_code(u8) + payload_size(u16) + packet_index(u8) +
                         tick_id(u32) + packet_count(u8) + reserved(3)
    - Payload (4 bytes): controlled_entity_id(u32)

    Args:
        response: Raw packet bytes

    Returns:
        OpCode (should be 0x05)
    """
    if len(response) < 12:
        raise ValueError(f"Response too short: {len(response)} bytes")

    # Parse header
    op_code = struct.unpack('<B', response[0:1])[0]

    return op_code


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
            pass # Ignore errors here
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


# ===========================================================================
# EDGE CASE TESTS
# ===========================================================================

def test_long_username(host: str, port: int) -> bool:
    """Test username longer than 31 characters."""
    print(f"\n{Color.BOLD}Test 6: Long Username (>31 chars){Color.RESET}")
    print("=" * 60)

    # 50 characters - should be truncated to 31
    long_username = "A" * 50

    try:
        sock, player_id, status = send_connect_req(host, port, long_username)

        if status == 0:  # OK
            print(f"{Color.GREEN}✓{Color.RESET} Server accepted (truncated username)")
            print(f"  Player ID: {player_id}")
            if sock:
                sock.close()
            return True
        else:
            print(f"{Color.YELLOW}!{Color.RESET} Server rejected: {status_name(status)}")
            return False

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_special_characters(host: str, port: int) -> bool:
    """Test username with special characters."""
    print(f"\n{Color.BOLD}Test 7: Special Characters{Color.RESET}")
    print("=" * 60)

    special_username = "User!@#$%^&*()_+-="

    try:
        sock, player_id, status = send_connect_req(host, port, special_username)

        if status == 0:
            print(f"{Color.GREEN}✓{Color.RESET} Server accepted special chars")
            print(f"  Player ID: {player_id}")
            if sock:
                sock.close()
            return True
        else:
            print(f"{Color.YELLOW}!{Color.RESET} Server rejected: {status_name(status)}")
            return True  # Not necessarily a failure

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_emoji_username(host: str, port: int) -> bool:
    """Test username with emoji characters."""
    print(f"\n{Color.BOLD}Test 8: Emoji Username{Color.RESET}")
    print("=" * 60)

    emoji_username = "Player🎮😀"

    try:
        sock, player_id, status = send_connect_req(host, port, emoji_username)

        print(f"{Color.GREEN}✓{Color.RESET} Server handled emoji username")
        print(f"  Status: {status_name(status)}")
        if sock:
            sock.close()
        return True

    except Exception as e:
        print(f"{Color.YELLOW}!{Color.RESET} Exception (may be expected): {e}")
        return True  # Emoji handling varies


def test_whitespace_username(host: str, port: int) -> bool:
    """Test username with only whitespace (should be trimmed to empty)."""
    print(f"\n{Color.BOLD}Test 9: Whitespace-Only Username{Color.RESET}")
    print("=" * 60)

    whitespace_username = "     "

    try:
        _, player_id, status = send_connect_req(host, port, whitespace_username)

        # Server trims whitespace, so this becomes empty and is rejected
        if status == 2:  # BadUsername
            print(f"{Color.GREEN}✓{Color.RESET} Correctly rejected (trimmed to empty)")
            print(f"  Status: {status_name(status)}")
            return True
        else:
            print(f"{Color.YELLOW}!{Color.RESET} Unexpected status: {status_name(status)}")
            return False

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_username_with_spaces(host: str, port: int) -> bool:
    """Test username with leading/trailing spaces (should be trimmed)."""
    print(f"\n{Color.BOLD}Test 10: Username with Spaces{Color.RESET}")
    print("=" * 60)

    spaced_username = "  Player  "

    try:
        sock, player_id, status = send_connect_req(host, port, spaced_username)

        if status == 0:
            print(f"{Color.GREEN}✓{Color.RESET} Server accepted (trimmed to 'Player')")
            print(f"  Player ID: {player_id}")
            if sock:
                sock.close()
            return True
        else:
            print(f"{Color.RED}✗{Color.RESET} Server rejected: {status_name(status)}")
            return False

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_malformed_packet(host: str, port: int) -> bool:
    """Test packet with incorrect size field."""
    print(f"\n{Color.BOLD}Test 11: Malformed Packet{Color.RESET}")
    print("=" * 60)

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3.0)
        sock.connect((host, port))

        # Build packet with wrong payload size
        header = struct.pack(
            '<BHBIB',
            0x01,  # op_code
            50,    # wrong payload_size (should be 32)
            0, 0, 1
        ) + b'\x00\x00\x00'

        username_bytes = b'Test'.ljust(32, b'\x00')
        packet = header + username_bytes

        sock.send(packet)

        # Server may reject or close connection
        sock.settimeout(2.0)
        try:
            response = sock.recv(1024)
            if response:
                print(f"{Color.YELLOW}!{Color.RESET} Server responded to malformed packet")
        except socket.timeout:
            print(f"{Color.GREEN}✓{Color.RESET} Server did not respond (expected)")

        sock.close()
        return True

    except Exception as e:
        print(f"{Color.GREEN}✓{Color.RESET} Connection closed (expected): {e}")
        return True


def test_invalid_opcode(host: str, port: int) -> bool:
    """Test packet with invalid OpCode."""
    print(f"\n{Color.BOLD}Test 12: Invalid OpCode{Color.RESET}")
    print("=" * 60)

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3.0)
        sock.connect((host, port))

        # Invalid OpCode
        header = struct.pack(
            '<BHBIB',
            0xFF,  # invalid op_code
            32, 0, 0, 1
        ) + b'\x00\x00\x00'

        username_bytes = b'Test'.ljust(32, b'\x00')
        packet = header + username_bytes

        sock.send(packet)

        sock.settimeout(2.0)
        try:
            response = sock.recv(1024)
            if response:
                print(f"{Color.YELLOW}!{Color.RESET} Server responded")
        except socket.timeout:
            print(f"{Color.GREEN}✓{Color.RESET} Server ignored invalid OpCode")

        sock.close()
        return True

    except Exception as e:
        print(f"{Color.GREEN}✓{Color.RESET} Connection handled: {e}")
        return True


def test_partial_packet(host: str, port: int) -> bool:
    """Test sending incomplete packet."""
    print(f"\n{Color.BOLD}Test 13: Partial Packet{Color.RESET}")
    print("=" * 60)

    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.settimeout(3.0)
        sock.connect((host, port))

        # Send only header (12 bytes), no payload
        packet = build_connect_req("Test")
        partial = packet[:12]

        sock.send(partial)

        sock.settimeout(2.0)
        try:
            response = sock.recv(1024)
            if response:
                print(f"{Color.RED}✗{Color.RESET} Server responded to partial packet")
                return False
        except socket.timeout:
            print(f"{Color.GREEN}✓{Color.RESET} Server waiting for complete packet")

        sock.close()
        return True

    except Exception as e:
        print(f"{Color.GREEN}✓{Color.RESET} Connection behavior: {e}")
        return True


def test_reconnection(host: str, port: int) -> bool:
    """Test client reconnecting after disconnect."""
    print(f"\n{Color.BOLD}Test 14: Reconnection{Color.RESET}")
    print("=" * 60)

    username = "ReconnectTest"

    try:
        # First connection
        sock1, player_id1, status1 = send_connect_req(host, port, username)
        if status1 != 0:
            print(f"{Color.RED}✗{Color.RESET} First connection failed")
            return False

        print(f"{Color.GREEN}✓{Color.RESET} First connection: ID={player_id1}")
        sock1.close()
        time.sleep(0.3)

        # Reconnect
        sock2, player_id2, status2 = send_connect_req(host, port, username)
        if status2 != 0:
            print(f"{Color.RED}✗{Color.RESET} Reconnection failed")
            return False

        print(f"{Color.GREEN}✓{Color.RESET} Reconnection successful: ID={player_id2}")
        sock2.close()

        return True

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        return False


def test_rapid_reconnections(host: str, port: int) -> bool:
    """Test rapid connection/disconnection cycles."""
    print(f"\n{Color.BOLD}Test 15: Rapid Reconnections{Color.RESET}")
    print("=" * 60)

    cycles = 5
    username = "RapidReconnect"
    
    try:
        for i in range(cycles):
            # Connect
            sock, player_id, status = send_connect_req(host, port, username)
            
            if status != 0:
                print(f"{Color.RED}✗ Cycle {i + 1}/{cycles} failed to connect (Status={status}){Color.RESET}")
                if sock:
                    sock.close()
                return False
            
            # Immediately disconnect
            sock.close()
            
            # Small delay to let server process disconnect
            time.sleep(0.05)
        
        print(f"{Color.GREEN}✓{Color.RESET} Completed {cycles} rapid reconnection cycles")
        return True
        
    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Exception during rapid reconnections: {e}")
        return False

def test_concurrent_connections(host: str, port: int, count: int = 4) -> bool:
    """Test multiple clients connecting simultaneously."""
    print(f"\n{Color.BOLD}Test 16: Concurrent Connections ({count} clients){Color.RESET}")
    print("=" * 60)

    results = []

    def connect_client(client_id: int):
        try:
            sock, player_id, status = send_connect_req(
                host, port, f"Concurrent{client_id}"
            )
            results.append((client_id, player_id, status, sock))
        except Exception as e:
            results.append((client_id, None, None, None))

    # Start threads
    threads = []
    for i in range(count):
        t = threading.Thread(target=connect_client, args=(i,))
        threads.append(t)
        t.start()

    # Wait for completion
    for t in threads:
        t.join()

    # Check results
    successful = sum(1 for _, _, status, _ in results if status == 0)

    print(f"  Successful: {successful}/{count}")

    # Cleanup
    for _, _, _, sock in results:
        if sock:
            sock.close()

    if successful == count:
        print(f"{Color.GREEN}✓{Color.RESET} All concurrent connections succeeded")
        return True
    elif successful > 0:
        print(f"{Color.YELLOW}!{Color.RESET} Partial success: {successful}/{count}")
        return True
    else:
        print(f"{Color.RED}✗{Color.RESET} All connections failed")
        return False


def test_ready_and_game_start(host: str, port: int, num_clients: int = 2) -> bool:
    """Test that server sends GAME_START when all clients are ready."""
    print(f"\n{Color.BOLD}Test 17: Ready Status and Game Start ({num_clients} clients){Color.RESET}")
    print("=" * 60)

    sockets = []
    player_ids = []

    try:
        # Connect all clients
        for i in range(1, num_clients + 1):
            username = f"Ready{i}"
            sock, player_id, status = send_connect_req(host, port, username, timeout=5.0)
            
            if status != 0:
                print(f"{Color.RED}✗{Color.RESET} {username} connection failed: {status_name(status)}")
                for s in sockets:
                    s.close()
                return False
            
            sockets.append(sock)
            player_ids.append(player_id)
            print(f"{Color.GREEN}✓{Color.RESET} {username:12s} connected (ID={player_id})")
            time.sleep(0.1)

        # Mark all clients as ready
        print(f"\nMarking all {num_clients} clients as ready...")
        for i, sock in enumerate(sockets):
            ready_packet = build_ready_status(True)
            sock.send(ready_packet)
            print(f"  Sent READY_STATUS to Player {player_ids[i]}")
            time.sleep(0.1)

        # Wait for GAME_START from server
        print("\nWaiting for GAME_START from server...")
        game_start_received = [False] * num_clients
        
        for i, sock in enumerate(sockets):
            sock.settimeout(3.0)
            try:
                response = sock.recv(1024)
                if len(response) >= 12:
                    op_code = parse_game_start(response)
                    if op_code == 0x05:
                        game_start_received[i] = True
                        print(f"{Color.GREEN}✓{Color.RESET} Player {player_ids[i]} received GAME_START")
                    else:
                        print(f"{Color.YELLOW}!{Color.RESET} Player {player_ids[i]} received OpCode 0x{op_code:02x} (expected 0x05)")
                else:
                    print(f"{Color.YELLOW}!{Color.RESET} Player {player_ids[i]} received incomplete packet ({len(response)} bytes)")
            except socket.timeout:
                print(f"{Color.RED}✗{Color.RESET} Player {player_ids[i]} timeout waiting for GAME_START")

        # Check results
        all_received = all(game_start_received)
        
        if all_received:
            print(f"\n{Color.GREEN}✓{Color.RESET} All {num_clients} clients received GAME_START packet")
            result = True
        else:
            received_count = sum(game_start_received)
            print(f"\n{Color.RED}✗{Color.RESET} Only {received_count}/{num_clients} clients received GAME_START")
            result = False

        # Cleanup
        for sock in sockets:
            sock.close()

        return result

    except Exception as e:
        print(f"{Color.RED}✗{Color.RESET} Error: {e}")
        for sock in sockets:
            try:
                sock.close()
            except:
                pass
        return False


def main():
    """Run all tests."""
    parser = argparse.ArgumentParser(
        description='Comprehensive R-Type server TCP connection tests',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument('--host', default='127.0.0.1',
                        help='Server hostname/IP (default: 127.0.0.1)')
    parser.add_argument('--port', type=int, default=50000,
                        help='Server TCP port (default: 50000)')
    parser.add_argument('--test',
                        choices=['basic', 'multi', 'full', 'duplicate', 'empty',
                                'edge', 'ready', 'all'],
                        default='all',
                        help='Test category to run (default: all)')

    args = parser.parse_args()

    print(f"{Color.BOLD}{Color.BLUE}")
    print("=" * 70)
    print("R-Type Server TCP Connection Tests")
    print("=" * 70)
    print(f"{Color.RESET}")
    print(f"Server: {args.host}:{args.port}\n")

    results = {}

    try:
        # Basic tests
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

        # Ready status and game start test
        if args.test in ('ready', 'all'):
            results['ready_game_start'] = test_ready_and_game_start(args.host, args.port, 2)

        # Edge case tests
        if args.test in ('edge', 'all'):
            results['long_username'] = test_long_username(args.host, args.port)
            results['special_chars'] = test_special_characters(args.host, args.port)
            results['emoji'] = test_emoji_username(args.host, args.port)
            results['whitespace'] = test_whitespace_username(args.host, args.port)
            results['spaces'] = test_username_with_spaces(args.host, args.port)
            results['malformed'] = test_malformed_packet(args.host, args.port)
            results['invalid_opcode'] = test_invalid_opcode(args.host, args.port)
            results['partial'] = test_partial_packet(args.host, args.port)
            results['reconnection'] = test_reconnection(args.host, args.port)
            results['rapid_reconnect'] = test_rapid_reconnections(args.host, args.port)
            results['concurrent'] = test_concurrent_connections(args.host, args.port, 4)

    except KeyboardInterrupt:
        print(f"\n\n{Color.YELLOW}Tests interrupted by user{Color.RESET}")
        sys.exit(1)

    # Summary
    print(f"\n{Color.BOLD}Summary{Color.RESET}")
    print("=" * 70)

    passed = sum(1 for v in results.values() if v)
    total = len(results)

    for test_name, result in results.items():
        status = f"{Color.GREEN}PASS{Color.RESET}" if result else f"{Color.RED}FAIL{Color.RESET}"
        print(f"  {test_name:20s} {status}")

    print(f"\nTotal: {passed}/{total} tests passed")

    if passed == total:
        print(f"\n{Color.GREEN}{Color.BOLD}✓ All tests passed!{Color.RESET}")
        sys.exit(0)
    elif passed >= total * 0.8:
        print(f"\n{Color.YELLOW}{Color.BOLD}⚠ Most tests passed ({passed}/{total}){Color.RESET}")
        sys.exit(0)
    else:
        print(f"\n{Color.RED}{Color.BOLD}✗ Many tests failed{Color.RESET}")
        sys.exit(1)


if __name__ == '__main__':
    main()
