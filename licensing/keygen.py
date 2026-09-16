#!/usr/bin/env python3
"""CTO keygen — mint $6 license keys. KEEP SECRET OFF-REPO.
Usage: CTO_FLEET_SECRET='<secret>' python3 keygen.py <tool> <YYYYMMDD> [nonce]
Secret must match licensing/secret.h. Store keys sold in a private ledger.
"""
import os, sys, hmac, hashlib, base64

def b32(data: bytes) -> str:
    out = base64.b32encode(data).decode().rstrip("=")
    return "-".join(out[i:i+4] for i in range(0, len(out), 4))

def mint(tool: str, expiry: str, nonce: str = "0000") -> str:
    secret = os.environ.get("CTO_FLEET_SECRET", "")
    if len(secret) < 16:
        sys.exit("Set CTO_FLEET_SECRET (16+ chars) in env. Never commit it.")
    payload = f"{tool}|{expiry}|{nonce}".encode()
    mac = hmac.new(secret.encode(), payload, hashlib.sha256).digest()[:10]
    raw = bytes([len(payload)]) + payload + mac
    return b32(raw)

if __name__ == "__main__":
    if len(sys.argv) < 3:
        sys.exit("usage: keygen.py <tool> <YYYYMMDD> [nonce]")
    print(mint(sys.argv[1], sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else "0000"))
