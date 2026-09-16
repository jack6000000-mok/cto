# cto-licensing v1.1.0 — shared $6 gate (source lib)

HMAC-SHA256 keys, no trial. Drop into any Qt app:

```cmake
target_sources(myapp PRIVATE ../licensing/license.cpp ../licensing/license.h)
target_include_directories(myapp PRIVATE ../licensing)
```
```cpp
#include "license.h"
// in main(), after QApplication app(argc, argv):
if (!ensureLicensed("<tool-id>", "<AppName>", "<buy-url>"))
    return 0;
```

Key format: base32 groups, payload `tool|YYYYMMDD|nonce` + 10-byte MAC.
Mint: `CTO_FLEET_SECRET='<secret>' python3 keygen.py <tool> <YYYYMMDD>`.
Secret lives in `secret.h` (gitignored, local-only) — back it up off-box.
Limits: offline check, local clock, obfuscated secret. See `license.h`.
