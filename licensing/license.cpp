// Copyright (c) 2026 oday. All Rights Reserved. Proprietary.
#include "license.h"
#include "secret.h"
#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include <QFile>
#include <QDir>
#include <QDateTime>
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>

// payload layout: tool + '|' + expiry YYYYMMDD + '|' + nonce
// key body = base32(payloadLen1 + payload + mac10) grouped 4x4
static QByteArray b32(const QByteArray &in) {
    static const char *abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QByteArray out;
    int val = 0, bits = 0;
    for (unsigned char c : in) {
        val = (val << 8) | c; bits += 8;
        while (bits >= 5) { bits -= 5; out += abc[(val >> bits) & 31]; }
    }
    if (bits) out += abc[(val << (5 - bits)) & 31];
    return out;
}
static QByteArray unb32(const QString &in) {
    QByteArray out; int val = 0, bits = 0;
    for (QChar ch : in) {
        if (ch == '-') continue;
        int v = QString("ABCDEFGHIJKLMNOPQRSTUVWXYZ234567").indexOf(ch.toUpper());
        if (v < 0) return {};
        val = (val << 5) | v; bits += 5;
        if (bits >= 8) { bits -= 8; out += char((val >> bits) & 255); }
    }
    return out;
}
static QByteArray macFor(const QByteArray &payload) {
    return QMessageAuthenticationCode::hash(
        payload, QByteArray(CTO_FLEET_SECRET), QCryptographicHash::Sha256).left(10);
}

bool License::isValid(const QString &tool, const QString &key) {
    QByteArray raw = unb32(key.trimmed());
    if (raw.size() < 12) return false;
    quint8 plen = quint8(raw[0]);
    if (raw.size() != 1 + plen + 10) return false;
    QByteArray payload = raw.mid(1, plen);
    QByteArray mac = raw.mid(1 + plen, 10);
    if (mac != macFor(payload)) return false;
    QList<QByteArray> parts = payload.split('|');
    if (parts.size() != 3 || parts[0] != tool.toUtf8()) return false;
    QDate exp = QDate::fromString(parts[1], "yyyyMMdd");
    if (!exp.isValid() || exp < QDate::currentDate()) return false;
    return true;
}

static QString base() {
    QString d = QDir::home().filePath(".config/cto-empire");
    QDir().mkpath(d);
    return d;
}
QString License::keyPath(const QString &tool) { return base() + "/" + tool + ".key"; }
QString License::readKey(const QString &tool) {
    QFile f(keyPath(tool));
    return f.open(QIODevice::ReadOnly) ? QString::fromUtf8(f.readAll()).trimmed() : QString();
}
void License::writeKey(const QString &tool, const QString &key) {
    QFile f(keyPath(tool));
    if (f.open(QIODevice::WriteOnly | QIODevice::Truncate)) { f.write(key.trimmed().toUtf8()); }
}
void License::markTrialStart(const QString &tool) {
    QFile f(base() + "/" + tool + ".trial");
    if (!f.exists() && f.open(QIODevice::WriteOnly))
        f.write(QByteArray::number(QDateTime::currentSecsSinceEpoch()));
}
bool License::trialValid(const QString &tool, int trialDays) {
    QFile f(base() + "/" + tool + ".trial");
    if (!f.open(QIODevice::ReadOnly)) return true; // never started
    bool ok = false;
    qint64 start = f.readAll().trimmed().toLongLong(&ok);
    if (!ok) return false;
    return QDateTime::currentSecsSinceEpoch() - start < qint64(trialDays) * 86400;
}

bool ensureLicensed(const QString &tool, const QString &appName, const QString &buyUrl) {
    // No trial. Valid $6 key or quit. Legacy .trial files are inert.
    if (License::isValid(tool, License::readKey(tool))) return true;
    QDialog dlg;
    dlg.setWindowTitle(appName + " — License ($6)");
    dlg.resize(420, 220);
    auto *lay = new QVBoxLayout(&dlg);
    auto *info = new QLabel(QString(
        "<b>%1</b> is proprietary ($6).<br>"
        "Enter your key, or get one. No trial.").arg(appName));
    info->setWordWrap(true);
    auto *edit = new QLineEdit();
    edit->setPlaceholderText("XXXX-XXXX-XXXX-XXXX");
    auto *row = new QHBoxLayout();
    auto *bGo = new QPushButton("Activate");
    auto *bBuy = new QPushButton("Get key ($6)");
    auto *bQuit = new QPushButton("Quit");
    row->addWidget(bGo); row->addWidget(bBuy); row->addWidget(bQuit);
    lay->addWidget(info); lay->addWidget(edit); lay->addLayout(row);
    bool done = false;
    QObject::connect(bGo, &QPushButton::clicked, [&] {
        if (License::isValid(tool, edit->text())) {
            License::writeKey(tool, edit->text());
            QMessageBox::information(&dlg, appName, "Licensed. Welcome to the empire. 👑");
            done = true; dlg.accept();
        } else {
            QMessageBox::warning(&dlg, appName, "Invalid key for this tool.");
        }
    });
    QObject::connect(bBuy, &QPushButton::clicked, [&] {
        QDesktopServices::openUrl(QUrl(buyUrl));
    });
    QObject::connect(bQuit, &QPushButton::clicked, &dlg, &QDialog::reject);
    dlg.exec();
    return done;
}
