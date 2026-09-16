// QtMoCap — private mocap: HOG people boxes around you. MIT, see LICENSE-MIT.
#include "mocap.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QCoreApplication>
#include <QPainter>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/objdetect.hpp>
#include <opencv2/dnn.hpp>

static const int POSE_PAIRS[17][2] = {
    {1,2},{1,5},{2,3},{3,4},{5,6},{6,7},{1,8},{8,9},{9,10},
    {1,11},{11,12},{12,13},{1,0},{0,14},{14,16},{0,15},{15,17}};

static QString findModel(const QString &name) {
    QStringList cands = {
        QCoreApplication::applicationDirPath() + "/models/" + name,
        QCoreApplication::applicationDirPath() + "/../qtmocap/models/" + name,
        QDir::home().filePath("Projects/cto/qtmocap/models/" + name),
        "/usr/share/qtmocap/models/" + name,
    };
    for (auto &c : cands)
        if (QFile::exists(c))
            return c;
    return {};
}

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
        // detect every 3rd frame @320px, hold boxes between.
        // Backlight-proofed: equalized luma + dense scan + low thresholds.
        if (n % 3 == 0) {
            cv::resize(frm, small, cv::Size(320, 240));
            cv::Mat ycc, eq[3];
            cv::cvtColor(small, ycc, cv::COLOR_BGR2YCrCb);
            cv::split(ycc, eq);
            cv::equalizeHist(eq[0], eq[0]);
            cv::merge(eq, 3, ycc);
            cv::cvtColor(ycc, small, cv::COLOR_YCrCb2BGR);
            hog.detectMultiScale(small, found, -0.2, cv::Size(4, 4), cv::Size(0, 0), 1.03, 1);
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
        if (m_skelLink)
            m_skelLink->pushFrame(frm, (int)kept.size());
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

void CapThread::setSkel(SkelThread *s) { m_skelLink = s; }

SkelThread::SkelThread(QObject *parent) : QThread(parent) {
    QString proto = findModel("pose_coco.prototxt");
    QString model = findModel("pose_coco.caffemodel");
    if (proto.isEmpty() || model.isEmpty())
        return;
    try {
        m_net = cv::dnn::readNetFromCaffe(proto.toStdString(), model.toStdString());
        m_net.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
        m_net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
        m_ok = true;
    } catch (...) {
        m_ok = false;
    }
}
void SkelThread::stop() { m_run = false; wait(); }
void SkelThread::setOn(bool on) { m_on = on; }
void SkelThread::pushFrame(const cv::Mat &frm, int people) {
    QMutexLocker l(&m_mtx);
    frm.copyTo(m_latest);
    m_hint = people;
    m_fresh = true;
}
void SkelThread::run() {
    while (m_run.load()) {
        cv::Mat frm;
        {
            QMutexLocker l(&m_mtx);
            if (!m_fresh || !m_on.load() || m_hint.load() < 1) {
                m_fresh = false;
            } else {
                m_latest.copyTo(frm);
                m_fresh = false;
            }
        }
        if (frm.empty()) {
            QThread::msleep(400);
            continue;
        }
        cv::Mat inp = cv::dnn::blobFromImage(frm, 1.0 / 255, cv::Size(256, 256),
                                             cv::Scalar(0, 0, 0), false, false);
        m_net.setInput(inp);
        cv::Mat out = m_net.forward();
        int H = out.size[2], W = out.size[3];
        QVector<QPoint> pts;
        pts.reserve(18);
        int n = 0;
        for (int p = 0; p < 18; p++) {
            cv::Mat hm(H, W, CV_32F, out.ptr(0, p));
            double minV, maxV;
            cv::Point minP, maxP;
            cv::minMaxLoc(hm, &minV, &maxV, &minP, &maxP);
            if (maxV > 0.3) {
                pts << QPoint(int(maxP.x * frm.cols / W), int(maxP.y * frm.rows / H));
                n++;
            } else {
                pts << QPoint(-1, -1);
            }
        }
        emit skeleton(pts, n);
    }
}

MoCap::MoCap(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("QtMoCap [private] — v0.1.1");
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
    m_btnSkel = new QPushButton("💀 Skel");
    m_btnSkel->setCheckable(true);
    m_btnSkel->setChecked(true);
    connect(m_btnSkel, &QPushButton::clicked, this, &MoCap::onSkel);
    row->addWidget(m_cam); row->addWidget(m_btnSnap); row->addWidget(m_btnRec); row->addWidget(m_btnSkel);
    lay->addWidget(m_view); lay->addWidget(m_stat); lay->addLayout(row);
    m_thr = new CapThread(this);
    connect(m_thr, &CapThread::frame, this, &MoCap::onFrame);
    m_thr->start();
    m_skel = new SkelThread(this);
    m_thr->setSkel(m_skel);
    connect(m_skel, &SkelThread::skeleton, this, &MoCap::onSkeleton);
    if (m_skel->ok()) {
        m_skel->start();
    } else {
        m_btnSkel->setEnabled(false);
        m_btnSkel->setText("💀 N/A");
        m_skelOn = false;
    }
}

MoCap::~MoCap() {
    m_thr->stop();
    if (m_skel->ok())
        m_skel->stop();
}

void MoCap::onSkeleton(QVector<QPoint> pts, int n) {
    m_pts = pts;
    m_ptsN = n;
}

void MoCap::onFrame(QImage img, int people, double fps) {
    if (people < 0) {
        m_view->setText("no camera — check /dev/video0");
        m_stat->setText("people: –  fps: –");
        return;
    }
    if (m_skelOn && m_pts.size() == 18) {
        QPainter p(&img);
        p.setPen(QPen(QColor(0, 255, 255), 2));
        for (auto &pr : POSE_PAIRS) {
            QPoint a = m_pts[pr[0]], b = m_pts[pr[1]];
            if (a.x() >= 0 && b.x() >= 0)
                p.drawLine(a, b);
        }
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(255, 0, 64));
        for (auto &j : m_pts)
            if (j.x() >= 0)
                p.drawEllipse(j, 4, 4);
    }
    m_view->setPixmap(QPixmap::fromImage(img).scaled(
        m_view->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_stat->setText(QString("people: %1  joints: %2  fps: %3  %4")
                        .arg(people).arg(m_skelOn ? m_ptsN : 0).arg(fps, 0, 'f', 1)
                        .arg(m_rec ? "⏺ REC" : ""));
}

void MoCap::onSnap() { m_thr->snap(); }
void MoCap::onRec() {
    m_rec = !m_rec;
    m_thr->setRecord(m_rec);
    m_btnRec->setChecked(m_rec);
}
void MoCap::onSkel() {
    m_skelOn = m_btnSkel->isChecked();
    m_skel->setOn(m_skelOn);
    if (!m_skelOn) {
        m_pts.clear();
        m_ptsN = 0;
    }
}
void MoCap::onCam(int idx) { m_thr->setCam(idx); }
