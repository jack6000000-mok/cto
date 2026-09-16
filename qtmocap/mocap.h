#pragma once
// QtMoCap — private mocap: HOG people boxes around you. MIT, see LICENSE-MIT.
#include <QMainWindow>
#include <QThread>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMutex>
#include <QPoint>
#include <QVector>
#include <opencv2/core.hpp>
#include <opencv2/dnn.hpp>

class SkelThread;

class CapThread : public QThread {
    Q_OBJECT
public:
    explicit CapThread(QObject *parent = nullptr);
    void run() override;
    void stop();
    void setCam(int idx);
    void snap();
    void setRecord(bool on);
    void setSkel(SkelThread *s);
signals:
    void frame(QImage img, int people, double fps);
private:
    std::atomic<bool> m_run{true};
    std::atomic<int> m_cam{0};
    std::atomic<bool> m_snap{false};
    std::atomic<bool> m_rec{false};
    SkelThread *m_skelLink = nullptr;
};

// Async DNN skeleton: never blocks video. Grabs latest frame,
// infers @256px (~3s), posts joints. Lags video by design.
class SkelThread : public QThread {
    Q_OBJECT
public:
    explicit SkelThread(QObject *parent = nullptr);
    bool ok() const { return m_ok; }
    void run() override;
    void stop();
    void pushFrame(const cv::Mat &frm, int people);
    void setOn(bool on);
signals:
    void skeleton(QVector<QPoint> pts, int n);
private:
    bool m_ok = false;
    std::atomic<bool> m_run{true};
    std::atomic<bool> m_on{true};
    std::atomic<int> m_hint{0};
    QMutex m_mtx;
    cv::Mat m_latest;
    bool m_fresh = false;
    cv::dnn::Net m_net;
};

class MoCap : public QMainWindow {
    Q_OBJECT
public:
    explicit MoCap(QWidget *parent = nullptr);
    ~MoCap();
private slots:
    void onFrame(QImage img, int people, double fps);
    void onSkeleton(QVector<QPoint> pts, int n);
    void onSnap();
    void onRec();
    void onSkel();
    void onCam(int idx);
private:
    QLabel *m_view, *m_stat;
    QPushButton *m_btnSnap, *m_btnRec, *m_btnSkel;
    QComboBox *m_cam;
    CapThread *m_thr;
    SkelThread *m_skel;
    QVector<QPoint> m_pts;
    int m_ptsN = 0;
    bool m_rec = false;
    bool m_skelOn = true;
};
