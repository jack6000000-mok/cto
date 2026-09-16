// Copyright (c) 2026 oday. All Rights Reserved. Proprietary.
// cto-keygen — native $6 key minter. Secret comes from env CTO_FLEET_SECRET
// (never in binary, never in repo). Must mint byte-identical keys to keygen.py.
#include <QCoreApplication>
#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include <QTextStream>
#include <cstdlib>

static QString b32(const QByteArray &in) {
    static const char *abc = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
    QString out;
    int val = 0, bits = 0;
    for (unsigned char c : in) {
        val = (val << 8) | c;
        bits += 8;
        while (bits >= 5) {
            bits -= 5;
            out += abc[(val >> bits) & 31];
        }
    }
    if (bits)
        out += abc[(val << (5 - bits)) & 31];
    QString grouped;
    for (int i = 0; i < out.size(); i += 4) {
        if (i)
            grouped += '-';
        grouped += out.mid(i, 4);
    }
    return grouped;
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QTextStream out(stdout), err(stderr);
    if (argc < 3) {
        err << "usage: cto-keygen <tool> <YYYYMMDD> [nonce]\n";
        return 2;
    }
    const char *secret = std::getenv("CTO_FLEET_SECRET");
    if (!secret || QString(secret).size() < 16) {
        err << "Set CTO_FLEET_SECRET (16+ chars) in env. Never commit it.\n";
        return 3;
    }
    QString nonce = argc > 3 ? argv[3] : "0000";
    QByteArray payload = QString("%1|%2|%3").arg(argv[1], argv[2], nonce).toUtf8();
    QByteArray mac = QMessageAuthenticationCode::hash(
        payload, secret, QCryptographicHash::Sha256).left(10);
    QByteArray raw;
    raw += char(payload.size());
    raw += payload;
    raw += mac;
    out << b32(raw) << "\n";
    return 0;
}
