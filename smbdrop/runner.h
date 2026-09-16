// Copyright (c) 2026 oday. All Rights Reserved.
// Proprietary License, see LICENSE file.
// Qt toolkit components remain under GNU (L)GPL by The Qt Company Ltd.
#pragma once
#include <QMainWindow>
#include <QProcess>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>

class SmbDrop : public QMainWindow {
    Q_OBJECT
public:
    explicit SmbDrop(QWidget *parent = nullptr);
private slots:
    void runTool();
    void stopTool();
    void onOut();
    void onErr();
    void onDone(int code);
    void showAbout();
private:
    QLineEdit *m_target, *m_args;
    QTextEdit *m_log;
    QPushButton *m_btnRun, *m_btnStop, *m_btnAbout;
    QProcess *m_proc = nullptr;
};
