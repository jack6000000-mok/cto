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
