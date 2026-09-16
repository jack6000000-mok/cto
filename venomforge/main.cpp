// Copyright (c) 2026 oday. All Rights Reserved.
// Proprietary License, see LICENSE file.
// Qt toolkit components remain under GNU (L)GPL by The Qt Company Ltd.
#include <QApplication>
#include "runner.h"
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    VenomForge w;
    w.show();
    return app.exec();
}
