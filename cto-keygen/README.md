# cto-keygen — native minter (replaces keygen.py)

```bash
cmake -S cto-keygen -B cto-keygen/build && cmake --build cto-keygen/build -j$(nproc)
CTO_FLEET_SECRET='<secret>' ./cto-keygen/build/cto-keygen <tool> <YYYYMMDD> [nonce]
```

Byte-identical output to `../licensing/keygen.py` (verified in CI of the CTO's
box). Binary holds no secret — safe to keep, never ship the env.
