// Copyright (c) 2026 oday. All Rights Reserved.
// VirshManager Proprietary License, see LICENSE file.
#include <QApplication>
#include "manager.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    VirshManager w;
    w.show();
    return app.exec();
}
