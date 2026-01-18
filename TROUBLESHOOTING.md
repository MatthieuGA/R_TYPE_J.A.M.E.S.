# Troubleshooting Guide

Common issues and solutions for R-Type J.A.M.E.S.

## 📋 Quick Diagnosis

Before diving into specific issues, run this quick checklist:

- [ ] Are you running the latest version?
- [ ] Did you build the project successfully?
- [ ] Is the server running before starting clients?
- [ ] Are firewall rules allowing UDP traffic on port 4242?
- [ ] Do you have all required dependencies installed?

## 🚀 Installation & Build Issues

### CMake Configuration Fails

**Symptom**: `cmake` command fails with dependency errors

**Solutions**:

1. **Verify vcpkg is bootstrapped**:
   ```bash
   cd vcpkg
   ./bootstrap-vcpkg.sh  # Linux
   ./bootstrap-vcpkg.bat # Windows
   ```

2. **Check CMake version**:
   ```bash
   cmake --version  # Should be 3.23+
   ```

3. **Set vcpkg toolchain**:
   ```bash
   cmake -S . -B build \
     -DCMAKE_TOOLCHAIN_FILE="./vcpkg/scripts/buildsystems/vcpkg.cmake"
   ```

4. **Clear CMake cache**:
   ```bash
   rm -rf build/
   mkdir build && cd build
   cmake ..
   ```

### Build Fails with Compiler Errors

**Symptom**: Compilation errors mentioning C++20 features

**Solutions**:

1. **Verify compiler version**:
   ```bash
   g++ --version      # Should be 12+
   clang++ --version  # Should be 15+
   ```

2. **Install correct compiler**:
   - **Linux (Ubuntu/Debian)**:
     ```bash
     sudo apt install g++-12
     sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-12 100
     ```
   - **Linux (Fedora)**:
     ```bash
     sudo dnf install gcc-c++
     ```
   - **Windows**: Install Visual Studio 2022 or later

3. **Force C++20 standard**:
   ```bash
   cmake -DCMAKE_CXX_STANDARD=20 ..
   ```

### vcpkg Dependencies Fail to Install

**Symptom**: vcpkg cannot download or build SFML, Boost, etc.

**Solutions**:

1. **Check internet connection**
2. **Update vcpkg**:
   ```bash
   cd vcpkg
   git pull
   ./bootstrap-vcpkg.sh
   ```

3. **Clear vcpkg cache**:
   ```bash
   ./vcpkg remove --outdated
   rm -rf buildtrees/ packages/
   ```

4. **Manual dependency install**:
   ```bash
   ./vcpkg install sfml boost-asio boost-lockfree boost-system
   ```

### "Cannot find SFML" Error

**Symptom**: CMake cannot find SFML package

**Solutions**:

1. **Verify SFML is installed via vcpkg**:
   ```bash
   ./vcpkg list | grep sfml
   ```

2. **Check vcpkg integration**:
   ```bash
   ./vcpkg integrate install
   ```

3. **Specify SFML path manually**:
   ```bash
   cmake -DSFML_DIR="./vcpkg_installed/x64-linux/share/sfml" ..
   ```

## 🌐 Network & Connection Issues

### Client Cannot Connect to Server

**Symptom**: "Connection refused" or "Cannot connect to server"

**Solutions**:

1. **Verify server is running**:
   ```bash
   ./r-type_server 4242
   # Should output: "Server listening on port 4242"
   ```

2. **Check server IP and port**:
   ```bash
   # Client should connect to correct IP
   ./r-type_client 192.168.1.100 4242
   ```

3. **Test localhost connection first**:
   ```bash
   # Terminal 1
   ./r-type_server 4242
   
   # Terminal 2
   ./r-type_client localhost 4242
   ```

4. **Firewall rules** (Linux):
   ```bash
   sudo ufw allow 4242/udp
   sudo ufw allow 4242/tcp
   ```

5. **Firewall rules** (Windows):
   - Open Windows Defender Firewall
   - Add inbound rule for port 4242 UDP/TCP
   - Allow r-type_server.exe and r-type_client.exe

### High Latency / Lag

**Symptom**: Delayed responses, rubber-banding movement

**Solutions**:

1. **Check network ping**:
   ```bash
   ping <server-ip>
   # Should be <50ms for good gameplay
   ```

2. **Ensure local network**:
   - Server and clients should be on same LAN
   - Avoid WiFi if possible (use ethernet)

3. **Check server CPU usage**:
   ```bash
   top  # or htop
   # r-type_server should use <50% CPU
   ```

4. **Reduce player count**:
   - Fewer players = less network traffic
   - Maximum recommended: 4 players

5. **Close bandwidth-heavy applications**:
   - Streaming, downloads, video calls

### Disconnects / Connection Drops

**Symptom**: Client randomly disconnects from server

**Solutions**:

1. **Check network stability**:
   ```bash
   ping -c 100 <server-ip>
   # Look for packet loss %
   ```

2. **Increase timeout values** (if configurable):
   - Edit server config to increase client timeout

3. **Check router NAT settings**:
   - Enable UPnP or port forwarding for port 4242

4. **Disable power saving on WiFi**:
   - Linux: `sudo iwconfig wlan0 power off`
   - Windows: Network adapter settings → Power Management

### "Port Already in Use" Error

**Symptom**: Server fails to start, says port 4242 is in use

**Solutions**:

1. **Find process using port**:
   - **Linux**:
     ```bash
     sudo lsof -i :4242
     sudo netstat -tulpn | grep 4242
     ```
   - **Windows**:
     ```cmd
     netstat -ano | findstr :4242
     ```

2. **Kill existing process**:
   - **Linux**:
     ```bash
     sudo kill -9 <PID>
     ```
   - **Windows**:
     ```cmd
     taskkill /PID <PID> /F
     ```

3. **Use different port**:
   ```bash
   ./r-type_server 4243
   ./r-type_client localhost 4243
   ```

## 🎮 Gameplay Issues

### Controls Not Responding

**Symptom**: Keypresses or gamepad input ignored

**Solutions**:

1. **Click game window**:
   - Ensure window has focus
   - Click anywhere in game window

2. **Test keyboard**:
   - Try both arrow keys and WASD
   - Check if Caps Lock is on

3. **Test gamepad**:
   - **Linux**:
     ```bash
     jstest /dev/input/js0
     ```
   - **Windows**: Control Panel → Devices → Game Controllers

4. **Reconnect controller**:
   - Unplug and replug USB controller
   - Restart game after reconnecting

5. **Check for conflicting software**:
   - Disable Steam Input (if running via Steam)
   - Close applications that capture input (AntiMicro, JoyToKey)

### Low FPS / Performance Issues

**Symptom**: Game runs slowly, choppy framerate

**Solutions**:

1. **Check system requirements**:
   - CPU: Dual-core 2.0 GHz+
   - RAM: 2 GB+
   - GPU: OpenGL 2.1+ support

2. **Update graphics drivers**:
   - **NVIDIA**: Download from nvidia.com
   - **AMD**: Download from amd.com
   - **Intel**: Use Windows Update or intel.com

3. **Close background applications**:
   ```bash
   # Linux
   top
   # Kill CPU-heavy processes
   
   # Windows
   Task Manager → End unnecessary tasks
   ```

4. **Reduce graphics settings** (when implemented):
   - Lower resolution
   - Disable V-Sync
   - Reduce particle effects

5. **Check thermal throttling**:
   ```bash
   # Linux
   sensors  # Install lm-sensors
   # CPU should be <80°C
   ```

### Game Crashes on Startup

**Symptom**: Application crashes immediately after launch

**Solutions**:

1. **Check dependencies are installed**:
   ```bash
   ldd ./r-type_client  # Linux
   # Should show all libraries found
   ```

2. **Run in debug mode**:
   ```bash
   gdb ./r-type_client
   (gdb) run
   (gdb) bt  # Get backtrace on crash
   ```

3. **Check asset files exist**:
   ```bash
   ls -la assets/
   # Should contain sprites, fonts, sounds
   ```

4. **Verify OpenGL support**:
   ```bash
   glxinfo | grep "OpenGL version"  # Linux
   # Should be 2.1+
   ```

5. **Try software rendering** (Linux):
   ```bash
   LIBGL_ALWAYS_SOFTWARE=1 ./r-type_client
   ```

### No Sound / Audio Issues

**Symptom**: Game runs but no audio output

**Solutions**:

1. **Check system audio**:
   - Ensure system volume is not muted
   - Test audio with other applications

2. **Verify audio files exist**:
   ```bash
   ls -la assets/audio/
   # Should contain .ogg or .wav files
   ```

3. **Check SFML audio module**:
   ```bash
   ldd ./r-type_client | grep sfml-audio
   ```

4. **Audio driver issues** (Linux):
   ```bash
   # Check PulseAudio is running
   pulseaudio --check
   pulseaudio --start
   ```

5. **Windows audio troubleshooter**:
   - Settings → System → Sound → Troubleshoot

### Enemies Not Spawning

**Symptom**: Game starts but no enemies appear

**Solutions**:

1. **Check level is loaded**:
   - Verify server selected a level at startup
   - Server console should show "Loading level: <name>"

2. **Verify JSON files exist**:
   ```bash
   ls -la data/
   # Should contain .json files (daemon.json, golem.json, etc.)
   ```

3. **Check JSON syntax**:
   ```bash
   # Validate JSON files
   python3 -m json.tool data/daemon.json
   ```

4. **Server console errors**:
   - Check server output for errors loading enemies
   - Look for "Failed to load actor" messages

5. **WorldGen configuration**:
   - Ensure level has enemy spawn definitions
   - Check `.wgf` level files in `data/levels/`

## 📊 Crash Reports & Logs

### Collecting Debug Information

When reporting bugs, include:

1. **Client Log**:
   ```bash
   ./r-type_client 2>&1 | tee client.log
   ```

2. **Server Log**:
   ```bash
   ./r-type_server 4242 2>&1 | tee server.log
   ```

3. **System Information**:
   - **Linux**:
     ```bash
     uname -a
     lscpu
     free -h
     glxinfo | grep "OpenGL"
     ```
   - **Windows**:
     ```cmd
     systeminfo
     ```

4. **Backtrace** (for crashes):
   ```bash
   gdb ./r-type_client
   (gdb) run
   # Let it crash
   (gdb) bt full
   ```

### Common Error Messages

| Error Message | Meaning | Solution |
|---------------|---------|----------|
| `bind: Address already in use` | Port 4242 is occupied | Kill existing process or use different port |
| `Failed to load texture` | Asset file missing | Check `assets/` folder has all files |
| `Connection timeout` | Client can't reach server | Check firewall, IP address, server running |
| `Segmentation fault` | Memory access violation | Run with `gdb` for backtrace, report bug |
| `Cannot open shared object` | Missing library | Install dependencies via vcpkg |

## 🔍 Advanced Troubleshooting

### Network Debugging

Capture packets to diagnose network issues:

```bash
# Linux
sudo tcpdump -i any port 4242 -w capture.pcap

# Analyze with Wireshark
wireshark capture.pcap
```

### Profiling Performance

Find performance bottlenecks:

```bash
# Linux - perf
perf record ./r-type_client
perf report

# Valgrind - memory issues
valgrind --leak-check=full ./r-type_client
```

### Enabling Debug Build

Build with debug symbols:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
gdb ./build/client/r-type_client
```

## 📞 Getting Help

If you can't solve your issue:

1. **Check Documentation**:
   - [How to Play](docs/docs/gameplay/how-to-play.md)
   - [Architecture](docs/docs/architecture.md)
   - [Protocol](docs/docs/protocol.md)

2. **Search Existing Issues**:
   - GitHub Issues: Look for similar problems

3. **Create New Issue**:
   - Include error messages, logs, and system info
   - Steps to reproduce the problem
   - What you've already tried

4. **Ask the Community**:
   - Project Discord/Forum (if available)
   - Be specific and include details

## 🛠️ Reset to Clean State

If all else fails, perform a complete reset:

```bash
# Backup your data
cp -r data/ data_backup/

# Clean build
rm -rf build/ vcpkg_installed/

# Reinstall dependencies
cd vcpkg
git pull
./bootstrap-vcpkg.sh
./vcpkg install sfml boost-asio boost-system boost-lockfree

# Rebuild
cd ..
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="./vcpkg/scripts/buildsystems/vcpkg.cmake"
cmake --build build -j$(nproc)

# Test
cd build/client
./r-type_client
```

---

## 📚 Additional Resources

- [AGENTS.md](AGENTS.md) - Development setup and workflow
- [README.md](README.md) - Project overview and quick start
- [CONTRIBUTING.md](CONTRIBUTING.md) - How to contribute
- [GitHub Issues](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./issues) - Known issues and bug reports

---

*Last Updated: January 2026*

**Still having issues?** Open an issue on GitHub with detailed information about your problem.
