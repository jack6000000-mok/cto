// Copyright (c) 2026 oday. All Rights Reserved. Proprietary.
#include "ctf.h"
#include <QWidget>
#include <QShortcut>
#include <QKeySequence>
#include <QDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QDir>

struct Flag { const char *tool; unsigned char blob[21]; int len; unsigned char key[16]; };
static const Flag kFlags[] = {
    {"glitch-game", {32,5,141,17,137,228,55,11,42,17,117,19,55,192,14,162,82,104,243,92,194}, 21, {99,81,194,106,191,133,2,61,27,38,22,42,82,164,57,150}},
    {"qt-pentest-game", {242,133,44,151,162,139,13,159,178,240,154,156,105,22,79,205,137,229,84,212,230}, 21, {177,209,99,236,155,179,57,172,138,146,252,175,88,32,125,171}},
    {"portscope", {161,61,245,188,196,85,144,100,228,8,224,165,49,78,19,202,219,94,216,254,143}, 21, {226,105,186,199,242,97,169,2,133,56,211,146,85,120,33,242}},
    {"packetscope", {53,121,116,224,77,252,47,80,185,231,1,3,138,145,47,38,68,29,3,255,6}, 21, {118,45,59,155,123,157,25,98,136,215,48,50,189,166,78,71}},
    {"webaudit", {13,65,91,110,46,58,159,226,53,171,167,165,114,142,140,155,125,37,37,35,50}, 21, {78,21,20,21,79,8,249,132,5,147,159,192,67,186,189,168}},
    {"injectprobe", {23,198,104,74,48,50,93,165,121,144,110,47,98,109,69,247,108,167,69,0,46}, 21, {84,146,39,49,83,80,107,192,24,243,94,23,87,14,119,197}},
    {"credaudit", {92,14,95,157,101,198,183,100,210,59,133,243,175,33,10,172,46,104,34,130,126}, 21, {31,90,16,230,3,244,142,92,224,93,224,145,202,18,59,159}},
    {"hashbench", {249,184,46,3,85,18,129,235,179,163,184,98,162,204,0,235,141,141,86,28,30}, 21, {186,236,97,120,99,118,182,138,130,149,141,86,150,245,48,216}},
    {"airaudit", {244,106,86,55,252,32,24,55,240,241,1,239,194,107,176,107,131,91,44,121,227}, 21, {183,62,25,76,158,19,45,84,150,146,103,214,240,90,135,10}},
    {"vulnlaunch", {197,108,229,211,237,14,133,242,175,231,31,112,142,240,234,135,183,11,159,154,244}, 21, {134,56,170,168,137,63,225,148,206,208,125,21,188,149,143,225}},
    {"venomforge", {108,36,38,78,56,243,135,41,141,191,75,1,237,151,155,26,31,18,81,86,112}, 21, {47,112,105,53,13,195,230,77,187,143,120,103,139,165,253,43}},
    {"smbdrop", {116,211,136,233,175,55,255,66,169,96,61,16,178,94,235,16,2,176,241,163,176}, 21, {55,135,199,146,205,81,154,122,145,82,15,114,138,109,136,40}},
    {"keymanager", {249,181,157,248,5,131,242,119,1,100,45,20,185,184,190,130,223,212,229,176,28}, 21, {186,225,210,131,97,231,196,19,52,83,26,113,129,219,137,224}},
    {"virsh-manager", {72,224,31,115,177,187,58,106,177,159,63,160,29,175,72,79,109,132,96,63,252}, 21, {11,180,80,8,129,216,88,15,208,254,9,145,45,204,126,43}},
};
static const int kFlagCount = sizeof(kFlags) / sizeof(kFlags[0]);

static QString decode(const Flag &f) {
    QByteArray out;
    for (int i = 0; i < f.len; i++) out += char(f.blob[i] ^ f.key[i % 16]);
    return QString::fromUtf8(out);
}
static QString flagFor(const QString &tool) {
    for (int i = 0; i < kFlagCount; i++)
        if (tool == kFlags[i].tool) return decode(kFlags[i]);
    return {};
}
static QString markPath(const QString &tool) {
    QString d = QDir::home().filePath(".config/cto-empire");
    QDir().mkpath(d);
    return d + "/ctf_" + tool + ".solved";
}
bool ctfSolved(const QString &tool) { return QFile::exists(markPath(tool)); }
int ctfSolvedCount() {
    int n = 0;
    for (int i = 0; i < kFlagCount; i++)
        if (ctfSolved(kFlags[i].tool)) n++;
    return n;
}

static void showChallenge(QWidget *parent, const QString &tool) {
    QDialog dlg(parent);
    dlg.setWindowTitle("⚑ hidden challenge — " + tool);
    dlg.resize(440, 240);
    auto *lay = new QVBoxLayout(&dlg);
    auto *info = new QLabel(
        QString("You found the glitch door in <b>%1</b>.<br><br>"
                "Somewhere in this binary sleeps a flag: <tt>CTO{...}</tt> — "
                "XOR-sealed, split from its key. No strings will hand it to you whole. "
                "Disassemble. Reassemble. <b>Extra hard, as ordered.</b><br><br>"
                "Solved across the empire: %2/14. Solve them all, DM oday, claim a free bundle.")
            .arg(tool).arg(ctfSolvedCount()));
    info->setWordWrap(true);
    auto *edit = new QLineEdit();
    edit->setPlaceholderText("CTO{...}");
    auto *bGo = new QPushButton("Submit flag");
    lay->addWidget(info); lay->addWidget(edit); lay->addWidget(bGo);
    QObject::connect(bGo, &QPushButton::clicked, [&] {
        if (edit->text().trimmed() == flagFor(tool)) {
            QFile f(markPath(tool));
            if (f.open(QIODevice::WriteOnly)) f.write("solved");
            QMessageBox::information(&dlg, "⚑", "FLAG ACCEPTED. This tool is yours in spirit. 👑");
            dlg.accept();
        } else {
            QMessageBox::warning(&dlg, "⚑", "Wrong flag. Dig deeper.");
        }
    });
    dlg.exec();
}

void installCtfHook(QWidget *window, const QString &tool) {
    auto *sc = new QShortcut(QKeySequence("Ctrl+Alt+G"), window);
    QObject::connect(sc, &QShortcut::activated, window, [window, tool] {
        showChallenge(window, tool);
    });
}
