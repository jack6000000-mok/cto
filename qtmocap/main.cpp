// QtMoCap — private mocap: HOG people boxes around you. MIT, see LICENSE-MIT.
#include <QApplication>
#include "mocap.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MoCap w;
    w.show();
    return app.exec();
}
