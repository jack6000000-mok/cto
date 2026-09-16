# CTO EMPIRE 👑
### Pentest tools by oday — Qt6, Kali-first.

| Tool | Version | What |
|---|---|---|
| `qt-pentest-game-cpp/` | v1.3.2 | **PENTEST // GLITCH PROTOCOL** — 15-min pentest drill quiz. Wrong answers trigger 3D glitch punishment (tearing, RGB split, window shake). About panel with live Tor + i2pd status probes. Ships `.deb` + Windows CI (`.zip` + NSIS setup) |
| `qt-pentest-game/` | v1.0 | Original Python/PySide6 prototype of the glitch game |
| `virsh-manager-prop/` | v0.2.2 | Thin Qt GUI over `virsh`/libvirt: VM list, start / shutdown / force-off, live log console. Ships `.deb` |
| `portscope/` | v0.2.2 | Scan profiler + history over nmap |
| `packetscope/` | v0.2.2 | Capture viewer over tshark |
| `webaudit/` | v0.2.2 | Web server scanner over nikto |
| `injectprobe/` | v0.2.2 | Injection tester over sqlmap (lab targets only) |
| `credaudit/` | v0.2.2 | Credential auditor over hydra (lab only) |
| `hashbench/` | v0.2.2 | Hash auditor + benchmarks over hashcat |
| `airaudit/` | v0.2.2 | Wireless auditor over aircrack-ng |
| `vulnlaunch/` | v0.2.2 | Vulnerable-lab VM launcher over virsh |
| `venomforge/` | v0.2.2 | Payload generator GUI over msfvenom (lab use only) |
| `smbdrop/` | v0.1.1 | SMB share dropper GUI over impacket smbserver (lab use only) |
| `keymanager/` | v0.1.1 | Read-only auth key viewer — fingerprints only, never secrets |
| `qtmocap/` | v0.1.1 | Private mocap: backlight-proof HOG boxes + async COCO skeleton, snap + record (MIT, needs OpenCV + 200M model, see folder) |
| `licensing/` | v1.2.0 (no trial) | Shared $6 gate: HMAC keys, trial, dialog, keygen (`cto-licensing` deb) |
| `scaffold/` | — | Fleet generator: `python3 scaffold/scaffold.py` stamps new tools |

## Install (Kali/Debian)
```bash
sudo dpkg -i glitch-game_1.2.0_amd64.deb virsh-manager_0.1.0_amd64.deb
glitch-game      # play the drill
virsh-manager    # manage VMs (needs libvirt: sudo apt install libvirt-daemon-system)
```

## Build from source
```bash
# GlitchGame
cmake -S qt-pentest-game-cpp -B build-game && cmake --build build-game -j$(nproc)
# VirshManager
cmake -S virsh-manager-prop -B build-virsh && cmake --build build-virsh -j$(nproc)
```
Needs: `cmake`, `qt6-base-dev` (Widgets + Network for the game). Windows: open `CMakeLists.txt` in QtCreator (MinGW) — see `windows-README.md`; CI builds `.exe` + installer automatically.

## Docs
- `qt-pentest-game-cpp/ROADMAP.md` — where the empire goes next
- `qt-pentest-game-cpp/RELEASE_CHECKLIST.md` — how a release gets cut
- `qt-pentest-game-cpp/installer/GlitchGame.nsi` — Windows installer script

## License
All first-party code is **proprietary** — see `LICENSE` in each tool folder (single-device use, no redistribution).
Qt toolkit components are **not** ours: dynamically linked under GNU (L)GPL by The Qt Company Ltd. — see [qt.io/licensing](https://www.qt.io/licensing).
