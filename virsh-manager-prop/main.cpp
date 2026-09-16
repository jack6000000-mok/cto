// Copyright (c) 2026 oday. All Rights Reserved.
// VirshManager Proprietary License, see LICENSE file.
#include <QApplication>
#include "manager.h"
#include "license.h"
#include "ctf.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    if (!ensureLicensed("virsh-manager", "VirshManager", "https://github.com/jack6000000-mok/cto"))
        return 0;
    VirshManager w;
    installCtfHook(&w, "virsh-manager");
    w.show();
    return app.exec();
}
