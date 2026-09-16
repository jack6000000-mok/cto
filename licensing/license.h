#pragma once
// CTO Empire licensing — $6/tool. HMAC-SHA256 keys, no trial.
// Honest limits: offline check, secret is obfuscated in-binary, clock is
// local. Stops casual sharing, not determined reversers. Real armor =
// Ed25519 + server activation (v2 roadmap).
#include <QString>

class License {
public:
    // Key format: XXXX-XXXX-XXXX-XXXX (base32 of tool+expiry+mac, truncated)
    static bool isValid(const QString &tool, const QString &key);
    static bool trialValid(const QString &tool, int trialDays = 7);
    static void markTrialStart(const QString &tool);
    static QString keyPath(const QString &tool);
    static QString readKey(const QString &tool);
    static void writeKey(const QString &tool, const QString &key);
};

// Blocking gate. Returns true only for a valid stored/entered key.
// Shows Activate / Buy($6) / Quit dialog otherwise. No trial.
bool ensureLicensed(const QString &tool, const QString &appName,
                    const QString &buyUrl);
