#pragma once
// QtMoCap — private mocap: HOG people boxes around you. MIT, see LICENSE-MIT.
#include <QMainWindow>
#include <QThread>
#include <QImage>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QMutex>
#include <atomic>
#include <opencv2/core.hpp>

class CapThread : public QThread {
    Q_OBJECT
public:
    explicit CapThread(QObject *parent = nullptr);
    void run() override;
    void stop();
    void setCam(int idx);
    void snap();
    void setRecord(bool on);
signals:
    void frame(QImage img, int people, double fps);
private:
    std::atomic<bool> m_run{true};
    std::atomic<int> m_cam{0};
    std::atomic<bool> m_snap{false};
    std::atomic<bool> m_rec{false};
};

class MoCap : public QMainWindow {
    Q_OBJECT
public:
    explicit MoCap(QWidget *parent = nullptr);
    ~MoCap();
private slots:
    void onFrame(QImage img, int people, double fps);
    void onSnap();
    void onRec();
    void onCam(int idx);
private:
    QLabel *m_view, *m_stat;
    QPushButton *m_btnSnap, *m_btnRec;
    QComboBox *m_cam;
    CapThread *m_thr;
    bool m_rec = false;
};
