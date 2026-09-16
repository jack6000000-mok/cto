# CTF — The Glitch Doors ⚑

Every empire app hides one flag: `CTO{...}`, XOR-sealed inside the binary.
14 doors, 14 flags. Extra hard, as ordered.

## Rules
1. One flag per tool. Flags are per-tool and unguessable — earn them.
2. Intended path: **reverse the binary** (disassemble, find the sealed table, rebuild the key). `strings` alone won't hand it whole.
3. The door: every Qt window listens for a hidden chord — three keys pressed together, the ones the glitch hides. (Riddle: *Control the Alternative Glitch.*)
4. Submit in-app. Solved markers live in `~/.config/cto-empire/ctf_<tool>.solved`.
5. No sharing flags publicly. DM oday with all 14 solved → free Empire bundle ($36 value).

## Scope
glitch-game, qt-pentest-game, portscope, packetscope, webaudit, injectprobe,
credaudit, hashbench, airaudit, vulnlaunch, venomforge, smbdrop, keymanager,
virsh-manager.
