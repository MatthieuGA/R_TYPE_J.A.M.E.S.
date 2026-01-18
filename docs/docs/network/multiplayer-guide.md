---
sidebar_position: 3
---

# Multiplayer Guide

Complete guide to playing R-Type J.A.M.E.S. with friends over local network or internet.

## 🌐 Overview

R-Type J.A.M.E.S. supports up to **4 simultaneous players** in cooperative multiplayer mode. The game uses an authoritative server model with UDP for gameplay and TCP for session management.

### Network Architecture

```
┌─────────────┐
│   Server    │ ← Authoritative game state
│  (Port 42)  │
└──────┬──────┘
       │
   ┌───┴────┬────────┬────────┐
   │        │        │        │
┌──▼──┐ ┌──▼──┐ ┌──▼──┐ ┌──▼──┐
│ P1  │ │ P2  │ │ P3  │ │ P4  │
└─────┘ └─────┘ └─────┘ └─────┘
```

- **Server**: Runs game logic, validates inputs, broadcasts game state
- **Clients**: Render graphics, send inputs, receive snapshots

## 🏠 Local Network (LAN) Setup

### Step 1: Start the Server

On one computer (the host):

```bash
./r-type_server 4242
```

Expected output:
```
Server configuration:
  Port: 4242
  Max players: 4

Available levels:
1. Level 1 (Finite)
2. Wave Defense (Infinite)
3. Boss Rush (Finite)

Select level number: 
```

Select a level by entering its number.

### Step 2: Find Server IP Address

**Linux/macOS**:
```bash
hostname -I
# Example output: 192.168.1.100
```

**Windows**:
```cmd
ipconfig
# Look for "IPv4 Address" under your active network adapter
# Example: 192.168.1.100
```

Share this IP address with other players.

### Step 3: Connect Clients

Each player runs:

```bash
./r-type_client <server-ip> 4242
```

Example:
```bash
./r-type_client 192.168.1.100 4242
```

### Step 4: Play!

- Players join automatically as P1, P2, P3, P4
- All players see the same game world
- Cooperative gameplay - work together to survive!

## 🌍 Internet Play

:::warning Port Forwarding Required
Playing over the internet requires port forwarding on the host's router. This may expose your network to security risks. Only play with trusted friends.
:::

### Remote Players Connect

Remote players connect using your public IP:

```bash
./r-type_client 203.0.113.45 4242
```

## 🎯 Multiplayer Best Practices

### Network Requirements

| Requirement | Minimum | Recommended |
|-------------|---------|-------------|
| **Latency** | &lt;150ms | &lt;50ms |
| **Bandwidth** | 1 Mbps | 5 Mbps |
| **Packet Loss** | &lt;5% | &lt;1% |
| **Jitter** | &lt;30ms | &lt;10ms |

### Server Hosting Tips

1. **Use Wired Connection**: Ethernet is more stable than WiFi
2. **Close Background Apps**: Free up CPU and network bandwidth
3. **Dedicated Server**: Run server on separate machine if possible
4. **Monitor Performance**: Watch server CPU usage (&lt;50% ideal)
5. **Stable Connection**: Use ISP with reliable uptime

### Player Tips

1. **Stay on LAN**: Best experience is on same local network
2. **Low Latency**: Join servers with low ping (&lt;50ms)
3. **Wired Connection**: Use ethernet cable if possible
4. **Close Downloads**: Pause torrents, updates, streaming



## ⚙️ Server Configuration

### Command Line Options

```bash
./r-type_server
```

### Level Selection

At server startup, select from available levels:
- **Finite Levels**: Have a defined end point
- **Infinite Levels**: Endless waves, survive as long as possible

Server displays:
```
Available levels:
1. Level 1 (Finite) - Classic R-Type recreation
2. Wave Defense (Infinite) - Endless enemy waves
3. Boss Rush (Finite) - Sequential boss battles

Select level number: _
```

## 📊 Network Monitoring

### Check Connection Quality

**Linux**:
```bash
ping -c 100 <server-ip>
# Look for:
# - avg latency (should be &lt;50ms)
# - packet loss (should be 0%)
```

**Windows**:
```cmd
ping -n 100 <server-ip>
```

### Diagnose Lag

If experiencing lag:

1. **Check Ping**:
   ```bash
   ping <server-ip>
   ```
   - < 30ms: Excellent
   - 30-50ms: Good
   - 50-100ms: Playable
   - 100-150ms: Noticeable lag
   - \> 150ms: Poor experience

2. **Check Packet Loss**:
   - 0%: Perfect
   - 1-2%: Acceptable
   - 3-5%: Noticeable issues
   - \> 5%: Severe problems

3. **Check Bandwidth**:
   ```bash
   iftop -i eth0  # Linux
   # R-Type uses ~100KB/s per client
   ```

### Network Optimization

**Reduce Latency**:
- Use wired connection (not WiFi)
- Close bandwidth-heavy apps
- Connect to nearby servers
- Use QoS on router (prioritize game traffic)

**Reduce Packet Loss**:
- Check cable connections
- Update router firmware
- Avoid network congestion times
- Use better quality ethernet cables

## 🔒 Security Considerations

### Server Security

When hosting over internet:

1. **Only Play with Trusted Friends**: Server has no authentication
2. **Use Strong Router Password**: Prevent unauthorized access
3. **Close Server When Not Playing**: Don't leave port forwarded 24/7
4. **Monitor Connections**: Watch server logs for suspicious activity
5. **Consider VPN**: Use Hamachi, ZeroTier, or WireGuard for private network

### Client Security

When connecting to remote servers:

1. **Only Join Trusted Servers**: Server can see your IP address
2. **Use Firewall**: Allow only necessary ports
3. **Monitor Network Activity**: Watch for unusual data usage
4. **Disconnect When Done**: Don't stay connected unnecessarily

## 🚨 Troubleshooting Multiplayer

### Cannot Connect to Server

Quick checks:
- Is server running?
- Is IP address correct?
- Is port correct (4242)?
- Is firewall blocking connection?
- Is port forwarded (for internet play)?

### High Lag / Latency

1. Check ping to server
2. Ensure no one is downloading/streaming
3. Use wired connection instead of WiFi
4. Connect to closer server
5. Reduce player count

### Frequent Disconnects

1. Check network stability (ping tests)
2. Update router firmware
3. Use better quality network cable
4. Disable WiFi power saving
5. Check for ISP issues

### Players Out of Sync

If game state differs between players:
- Server is authoritative - trust server state
- Client prediction may show temporary desync
- Server will correct client state on next snapshot
- Check for high packet loss (causes frequent corrections)

## 📚 Technical Details

### Network Protocol

- **TCP**: Session management (connect, disconnect, game start)
- **UDP**: Real-time gameplay (inputs, snapshots, events)

### Packet Flow

```
Client → Server: Input packets (60 Hz)
  - Player movement
  - Fire button presses
  - Other actions

Server → Clients: Snapshot packets (20 Hz)
  - All entity positions
  - Entity states
  - Game events
```

### Client-Side Prediction

Clients predict movement locally for smooth gameplay:
1. Client sends input to server
2. Client immediately applies input (prediction)
3. Server processes input (authoritative)
4. Server sends corrected state in snapshot
5. Client reconciles prediction with server state

This creates smooth movement even with network latency.

## 🎮 Dedicated Server Setup

For 24/7 server hosting:

### Linux (systemd service)

Create `/etc/systemd/system/rtype-server.service`:

```ini
[Unit]
Description=R-Type J.A.M.E.S. Game Server
After=network.target

[Service]
Type=simple
User=rtype
WorkingDirectory=/opt/rtype
ExecStart=/opt/rtype/r-type_server 4242
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

Enable and start:
```bash
sudo systemctl enable rtype-server
sudo systemctl start rtype-server
sudo systemctl status rtype-server
```

View logs:
```bash
sudo journalctl -u rtype-server -f
```

### Docker Container *(Coming Soon)*

Future releases will provide Docker images for easy deployment.

## 📈 Scalability

Current limitations:
- **Maximum players**: 4 per server
- **Concurrent games**: 1 per server instance
- **Load balancing**: Run multiple server instances on different ports

For more players:
- Run multiple server instances: ports 4242, 4243, 4244, etc.
- Clients choose which server to join

## 📞 Getting Help

For multiplayer issues:
- [GitHub Issues](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./issues)
- [Protocol Documentation](../protocol.md)

---

*Last Updated: January 2026*

**Happy multiplayer gaming!** 🎮🎉
