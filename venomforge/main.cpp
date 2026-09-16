// Copyright (c) 2026 oday. All Rights Reserved.
// Proprietary License, see LICENSE file.
// Qt toolkit components remain under GNU (L)GPL by The Qt Company Ltd.
#include <QApplication>
#include "runner.h"
#include "license.h"
#include "ctf.h"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (!ensureLicensed("venomforge", "VenomForge", "https://github.com/jack6000000-mok/cto"))
        return 0;
    VenomForge w;
    installCtfHook(&w, "venomforge");
    w.show();
    return app.exec();
}
