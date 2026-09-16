// QtMoCap — private mocap: HOG people boxes around you. MIT, see LICENSE-MIT.
#include "mocap.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>

CapThread::CapThread(QObject *parent) : QThread(parent) {}
void CapThread::stop() { m_run = false; wait(); }
void CapThread::setCam(int idx) { m_cam = idx; }
void CapThread::snap() { m_snap = true; }
void CapThread::setRecord(bool on) { m_rec = on; }

static QImage matToQ(const cv::Mat &m) {
    cv::Mat rgb;
    cv::cvtColor(m, rgb, cv::COLOR_BGR2RGB);
    return QImage(rgb.data, rgb.cols, rgb.rows,
                  (int)rgb.step, QImage::Format_RGB888).copy();
}

void CapThread::run() {
    cv::VideoCapture cap(m_cam.load(), cv::CAP_V4L2);
    if (!cap.isOpened())
        cap.open(m_cam.load());
    if (!cap.isOpened()) {
        emit frame(QImage(), -1, 0.0);
        return;
    }
    cap.set(cv::CAP_PROP_FRAME_WIDTH, 640);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    cv::HOGDescriptor hog;
    hog.setSVMDetector(cv::HOGDescriptor::getDefaultPeopleDetector());
    cv::VideoWriter vw;
    std::vector<cv::Rect> found, kept;
    qint64 last = 0;
    int frames = 0, n = 0;
    qint64 t0 = QDateTime::currentMSecsSinceEpoch();
    cv::Mat frm, small;
    while (m_run.load()) {
        if (!cap.read(frm) || frm.empty()) {
            QThread::msleep(50);
            continue;
        }
        cv::flip(frm, frm, 1); // selfie mirror
        // detect every 3rd frame @320px, hold boxes between
        if (n % 3 == 0) {
            cv::resize(frm, small, cv::Size(320, 240));
            hog.detectMultiScale(small, found, 0, cv::Size(8, 8), cv::Size(0, 0), 1.05, 2);
            kept.clear();
            for (auto r : found) // scale back x2
                kept.emplace_back(r.x * 2, r.y * 2, r.width * 2, r.height * 2);
        }
        for (size_t i = 0; i < kept.size(); i++) {
            // shrink HOG's loose box to the body
            cv::Rect r = kept[i];
            r.x += r.width * 0.1; r.width *= 0.8;
            r.y += r.height * 0.07; r.height *= 0.8;
            cv::rectangle(frm, r, cv::Scalar(0, 255, 136), 2);
            cv::putText(frm, "person " + std::to_string(i),
                        cv::Point(r.x, r.y - 6), cv::FONT_HERSHEY_SIMPLEX,
                        0.6, cv::Scalar(0, 255, 136), 2);
        }
        if (m_snap.exchange(false)) {
            QString p = QDir::home().filePath(
                "mocap-" + QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss") + ".png");
            matToQ(frm).save(p, "PNG");
        }
        if (m_rec.load() && !vw.isOpened()) {
            QString p = QDir::home().filePath(
                "mocap-" + QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss") + ".avi");
            vw.open(p.toStdString(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),
                    20, frm.size());
        } else if (!m_rec.load() && vw.isOpened()) {
            vw.release();
        }
        if (vw.isOpened())
            vw.write(frm);
        frames++;
        qint64 now = QDateTime::currentMSecsSinceEpoch();
        double fps = 1000.0 * frames / qMax<qint64>(1, now - t0);
        if (now - last > 250) {
            last = now;
            emit frame(matToQ(frm), (int)kept.size(), fps);
        }
        n++;
    }
    cap.release();
    if (vw.isOpened())
        vw.release();
}

MoCap::MoCap(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("QtMoCap [private] — v0.1.0");
    resize(700, 600);
    setStyleSheet("QMainWindow{background:#0a0e14;} QWidget{background:#0a0e14;color:#c8e6c9;}"
                  "QPushButton{border:2px solid #00ff88;border-radius:8px;padding:10px;background:#111927;}"
                  "QPushButton:checked{border-color:#ff0040;color:#ff5577;}"
                  "QComboBox{border:1px solid #00ff88;padding:8px;background:#111927;}");
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *lay = new QVBoxLayout(central);
    m_view = new QLabel("starting camera…");
    m_view->setAlignment(Qt::AlignCenter);
    m_view->setMinimumSize(640, 480);
    m_view->setStyleSheet("border:2px solid #00ff88;background:#05070b;");
    m_stat = new QLabel("people: –  fps: –");
    m_stat->setStyleSheet("color:#00ffee;font-size:15px;font-weight:bold;");
    auto *row = new QHBoxLayout();
    m_cam = new QComboBox();
    m_cam->addItems({"/dev/video0", "/dev/video1"});
    connect(m_cam, &QComboBox::currentIndexChanged, this, &MoCap::onCam);
    m_btnSnap = new QPushButton("📷 Snap");
    connect(m_btnSnap, &QPushButton::clicked, this, &MoCap::onSnap);
    m_btnRec = new QPushButton("⏺ Rec");
    m_btnRec->setCheckable(true);
    connect(m_btnRec, &QPushButton::clicked, this, &MoCap::onRec);
    row->addWidget(m_cam); row->addWidget(m_btnSnap); row->addWidget(m_btnRec);
    lay->addWidget(m_view); lay->addWidget(m_stat); lay->addLayout(row);
    m_thr = new CapThread(this);
    connect(m_thr, &CapThread::frame, this, &MoCap::onFrame);
    m_thr->start();
}

MoCap::~MoCap() { m_thr->stop(); }

void MoCap::onFrame(QImage img, int people, double fps) {
    if (people < 0) {
        m_view->setText("no camera — check /dev/video0");
        m_stat->setText("people: –  fps: –");
        return;
    }
    m_view->setPixmap(QPixmap::fromImage(img).scaled(
        m_view->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_stat->setText(QString("people: %1  fps: %2  %3")
                        .arg(people).arg(fps, 0, 'f', 1)
                        .arg(m_rec ? "⏺ REC" : ""));
}

void MoCap::onSnap() { m_thr->snap(); }
void MoCap::onRec() {
    m_rec = !m_rec;
    m_thr->setRecord(m_rec);
    m_btnRec->setChecked(m_rec);
}
void MoCap::onCam(int idx) { m_thr->setCam(idx); }
