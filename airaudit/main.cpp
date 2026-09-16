// Copyright (c) 2026 oday. All Rights Reserved.
// Proprietary License, see LICENSE file.
// Qt toolkit components remain under GNU (L)GPL by The Qt Company Ltd.
#include <QApplication>
#include "runner.h"
#include "license.h"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (!ensureLicensed("airaudit", "AirAudit", "https://github.com/jack6000000-mok/cto"))
        return 0;
    AirAudit w;
    w.show();
    return app.exec();
}
