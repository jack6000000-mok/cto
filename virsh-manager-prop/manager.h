#pragma once
// Copyright (c) 2026 oday. All Rights Reserved.
// VirshManager Proprietary License, see LICENSE file.
#include <QMainWindow>
#include <QProcess>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>

class VirshManager : public QMainWindow {
    Q_OBJECT
public:
    explicit VirshManager(QWidget *parent = nullptr);
private slots:
    void refresh();
    void runVirsh(const QStringList &args);
    void startVm();
    void shutdownVm();
    void destroyVm();
    void showAbout();
private:
    QString selectedVm() const;
    void runAndLog(const QString &prog, const QStringList &args);

    QTableWidget *m_table;
    QTextEdit *m_log;
    QLineEdit *m_uri;
    QPushButton *m_btnRefresh, *m_btnStart, *m_btnShutdown, *m_btnDestroy, *m_btnAbout;
    QProcess *m_proc = nullptr;
};
