#!/usr/bin/env python3
"""
Dummy R-Type Client for Testing

This script spawns multiple test clients to connect to the R-Type server
and optionally send READY_STATUS packets. Useful for testing lobby and
ready state functionality.

Usage:
    python3 scripts/dummy_client.py [host] [port]
    
Example:
    python3 scripts/dummy_client.py localhost 50000
"""

import socket
import struct
import sys
import time
from typing import Optional


# Packet types from RFC (server/include/server/PacketTypes.hpp)
class PacketType:
    CONNECT_REQ = 0x01
    CONNECT_ACK = 0x02
    DISCONNECT_REQ = 0x03
    NOTIFY_DISCONNECT = 0x04
    GAME_START = 0x05
    GAME_END = 0x06
    READY_STATUS = 0x07


class DummyClient:
    """A simple R-Type client for testing purposes."""
    
    def __init__(self, client_id: int, host: str, port: int):
        self.client_id = client_id
        self.host = host
        self.port = port
        self.sock: Optional[socket.socket] = None
        self.player_id: Optional[int] = None
        self.username = f"TestClient{client_id}"
        
    def connect(self) -> bool:
        """Connect to the server and send CONNECT_REQ packet."""
        try:
            self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.sock.settimeout(5.0)
            
            print(f"[Client {self.client_id}] Connecting to {self.host}:{self.port}...")
            self.sock.connect((self.host, self.port))
            print(f"[Client {self.client_id}] Connected successfully")
            
            # Send CONNECT_REQ packet
            self._send_connect_req()
            
            # Wait for CONNECT_ACK (connection stays open regardless of status)
            return self._receive_connect_ack()
            
        except Exception as e:
            print(f"[Client {self.client_id}] Connection failed: {e}")
            if self.sock:
                self.sock.close()
                self.sock = None
            return False
    
    def _send_connect_req(self):
        """Send CONNECT_REQ packet (opcode 0x01)."""
        # Header (12 bytes): OpCode (u8) + PayloadSize (u16) + PacketIndex (u8) + TickId (u32) + PacketCount (u8) + Reserved (u8[3])
        # Payload: Username (32 bytes, null-terminated)
        
        op_code = PacketType.CONNECT_REQ
        payload_size = 32
        packet_index = 0  # Single packet
        tick_id = 0  # TCP packets use TickId = 0
        packet_count = 1  # Single packet
        reserved = (0, 0, 0)  # 3 reserved bytes
        
        # Prepare username (32 bytes, null-padded)
        username_bytes = self.username.encode('utf-8')[:31]  # Max 31 chars + null terminator
        username_padded = username_bytes + b'\x00' * (32 - len(username_bytes))
        
        # Pack header (12 bytes) - Little Endian as per RFC Section 1.2
        header = struct.pack(
            '<BHBIBBBB',  # Little-endian: u8, u16, u8, u32, u8, u8, u8, u8
            op_code,
            payload_size,
            packet_index,
            tick_id,
            packet_count,
            reserved[0],
            reserved[1],
            reserved[2]
        )
        
        # Complete packet = header + payload
        packet = header + username_padded
        
        print(f"[Client {self.client_id}] Sending CONNECT_REQ with username '{self.username}'")
        self.sock.sendall(packet)
    
    def _receive_connect_ack(self) -> bool:
        """Receive and parse CONNECT_ACK packet."""
        try:
            # Receive header (12 bytes)
            header = self.sock.recv(12)
            if len(header) != 12:
                print(f"[Client {self.client_id}] Failed to receive complete header")
                return False
            
            # Parse header: OpCode (u8) + PayloadSize (u16) + PacketIndex (u8) + TickId (u32) + PacketCount (u8) + Reserved (u8[3])
            op_code, payload_size, packet_index, tick_id, packet_count, res1, res2, res3 = struct.unpack('<BHBIBBBB', header)
            
            if op_code != PacketType.CONNECT_ACK:
                print(f"[Client {self.client_id}] Expected CONNECT_ACK (0x02), got 0x{op_code:02x}")
                return False
            
            # Receive payload (4 bytes)
            payload = self.sock.recv(payload_size)
            if len(payload) != payload_size:
                print(f"[Client {self.client_id}] Failed to receive complete payload")
                return False
            
            # Parse payload: PlayerId (1 byte) + Status (1 byte) + Reserved (2 bytes)
            player_id, status, _, _ = struct.unpack('<BBBB', payload)
            
            status_names = {
                0: "OK",
                1: "ServerFull",
                2: "BadUsername",
                3: "InGame"
            }
            
            status_str = status_names.get(status, f"Unknown({status})")
            print(f"[Client {self.client_id}] Received CONNECT_ACK: PlayerId={player_id}, Status={status_str}")
            
            if status == 0:  # OK
                self.player_id = player_id
                return True
            else:
                print(f"[Client {self.client_id}] Connection rejected: {status_str}")
                print(f"[Client {self.client_id}] Staying connected (can retry later)")
                return False
                
        except Exception as e:
            print(f"[Client {self.client_id}] Error receiving CONNECT_ACK: {e}")
            return False
    def send_ready_status(self, is_ready: bool = True):
        """Send READY_STATUS packet (opcode 0x07)."""
        if not self.sock or self.player_id is None:
            print(f"[Client {self.client_id}] Not connected, cannot send READY_STATUS")
            return
        
        op_code = PacketType.READY_STATUS
        payload_size = 4
        packet_index = 0
        tick_id = 0  # TCP packets use TickId = 0
        packet_count = 1
        reserved = (0, 0, 0)
        
        # Header (12 bytes) - Little Endian
        header = struct.pack(
            '<BHBIBBBB',
            op_code,
            payload_size,
            packet_index,
            tick_id,
            packet_count,
            reserved[0],
            reserved[1],
            reserved[2]
        )
        
        # Payload: IsReady (1 byte) + Reserved (3 bytes)
        payload = struct.pack('<BBBB', 1 if is_ready else 0, 0, 0, 0)
        
        packet = header + payload
        
        print(f"[Client {self.client_id}] Sending READY_STATUS: {'Ready' if is_ready else 'Not Ready'}")
        self.sock.sendall(packet)
    
    def disconnect(self):
        """Close the connection."""
        if self.sock:
            print(f"[Client {self.client_id}] Disconnecting...")
            self.sock.close()
            self.sock = None
    
    def keep_alive(self):
        """Keep the connection alive (blocking)."""
        if not self.sock:
            return
        
        try:
            print(f"[Client {self.client_id}] Staying connected (press Ctrl+C to stop all clients)")
            # Just wait for server messages or connection close
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            pass


def main():
    """Main entry point for the dummy client launcher."""
    # Parse command line arguments
    host = sys.argv[1] if len(sys.argv) > 1 else "localhost"
    port = int(sys.argv[2]) if len(sys.argv) > 2 else 50000
    
    print("=" * 60)
    print("R-Type Dummy Client Launcher")
    print("=" * 60)
    print(f"Target: {host}:{port}")
    print()
    
    # Ask user how many clients
    while True:
        try:
            num_clients = int(input("How many clients do you want to spawn? (min 1): "))
            if 1 <= num_clients:
                break
            print("Please enter a number greater than or equal to 1")
        except ValueError:
            print("Invalid input, please enter a number")
    
    # Ask if clients should send ready packet
    while True:
        send_ready_input = input("Should clients send READY_STATUS packet? (y/n): ").lower().strip()
        if send_ready_input in ['y', 'n', 'yes', 'no']:
            send_ready = send_ready_input in ['y', 'yes']
            break
        print("Please answer 'y' or 'n'")
    
    # Ask if there should be a delay between clients
    while True:
        delay_input = input("Wait 1 second between each client connection? (y/n): ").lower().strip()
        if delay_input in ['y', 'n', 'yes', 'no']:
            use_delay = delay_input in ['y', 'yes']
            break
        print("Please answer 'y' or 'n'")
    
    print()
    print(f"Spawning {num_clients} client(s)...")
    print(f"Ready status: {'Will send READY' if send_ready else 'Will NOT send ready'}")
    print(f"Delay between clients: {'1 second' if use_delay else 'None (immediate)'}")
    print()
    
    clients = []
    
    try:
        # Spawn clients with 1 second delay
        for i in range(1, num_clients + 1):
            client = DummyClient(i, host, port)
            
            success = client.connect()
            
            # Add to clients list if TCP connection succeeded (regardless of auth status)
            if client.sock:
                clients.append(client)
                
                # Send ready status only if authenticated and requested
                if success and send_ready:
                    time.sleep(0.5)  # Small delay before sending ready
                    client.send_ready_status(is_ready=True)
            else:
                print(f"[Client {i}] Failed to establish TCP connection, skipping...")
            
            # Wait 1 second before spawning next client (except for the last one)
            if use_delay and i < num_clients:
                time.sleep(1)
        
        print()
        print("=" * 60)
        print(f"Successfully connected {len(clients)}/{num_clients} client(s)")
        print("Clients will stay connected until you press Ctrl+C")
        print("=" * 60)
        print()
        
        # Keep all clients alive
        if clients:
            clients[0].keep_alive()  # Use first client to wait for Ctrl+C
        
    except KeyboardInterrupt:
        print("\n\nShutting down all clients...")
    finally:
        # Disconnect all clients
        for client in clients:
            client.disconnect()
        print("All clients disconnected. Goodbye!")


if __name__ == "__main__":
    main()
