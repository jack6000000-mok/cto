#pragma once
// CTO Empire CTF — extra-hard hidden crackme. Flags live XOR-encoded;
// reversing the binary is the intended path. No trial, no mercy.
#include <QString>
class QWidget;

// Install hidden hook (Ctrl+Alt+G) on the main window.
void installCtfHook(QWidget *window, const QString &tool);

// Solved markers (one per tool, ~/.config/cto-empire/ctf_<tool>.solved)
bool ctfSolved(const QString &tool);
int ctfSolvedCount();
// Elite markers (ctf_<tool>.elite), earned behind the tracer lock
bool ctfElite(const QString &tool);
int ctfEliteCount();
// Runtime derivation (no stored bytes). Exposed for tests; reversers welcome.
QString eliteFor(const QString &tool);
