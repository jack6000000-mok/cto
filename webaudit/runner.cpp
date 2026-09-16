// Copyright (c) 2026 oday. All Rights Reserved.
// Proprietary License, see LICENSE file.
// Qt toolkit components remain under GNU (L)GPL by The Qt Company Ltd.
#include "runner.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

static const char *kBackend = "nikto";
static const char *kDesc = "WebAudit — web server scanner over nikto.";

WebAudit::WebAudit(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("WebAudit [Proprietary] — v0.1.0");
    resize(820, 560);
    setStyleSheet("QMainWindow{background:#0a0e14;} QWidget{background:#0a0e14;color:#c8e6c9;}"
                  "QPushButton{border:2px solid #7a00ff;border-radius:8px;padding:10px;background:#111927;}"
                  "QPushButton:hover{background:#1a2b3c;}"
                  "QPushButton:disabled{color:#555;border-color:#333;}"
                  "QLineEdit{border:1px solid #7a00ff;padding:8px;background:#111927;}"
                  "QTextEdit{background:#05070b;color:#7a00ff;font-family:monospace;}");
    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *lay = new QVBoxLayout(central);
    auto *title = new QLabel(QString("%1  •  backend: %2").arg(kDesc, kBackend));
    title->setWordWrap(true);
    title->setStyleSheet("font-size:15px;font-weight:bold;");
    lay->addWidget(title);
auto *warn = new QLabel("Only scan targets you own or are authorized to test.");
warn->setWordWrap(true);
warn->setStyleSheet("color:#ff5577;font-weight:bold;");
lay->addWidget(warn);
    auto *row = new QHBoxLayout();
    m_target = new QLineEdit("http://127.0.0.1/");
    m_target->setPlaceholderText("target");
    m_args = new QLineEdit("-h");
    m_args->setPlaceholderText("args");
    m_btnRun = new QPushButton("▶ Run");
    m_btnStop = new QPushButton("■ Stop");
    m_btnStop->setEnabled(false);
    m_btnAbout = new QPushButton("ⓘ About");
    connect(m_btnRun, &QPushButton::clicked, this, &WebAudit::runTool);
    connect(m_btnStop, &QPushButton::clicked, this, &WebAudit::stopTool);
    connect(m_btnAbout, &QPushButton::clicked, this, &WebAudit::showAbout);
    row->addWidget(m_target, 1); row->addWidget(m_args, 2);
    row->addWidget(m_btnRun); row->addWidget(m_btnStop); row->addWidget(m_btnAbout);
    lay->addLayout(row);
    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_log->append(QString("WebAudit v0.1.0 — proprietary GUI. Backend: %1 (theirs).").arg(kBackend));
    lay->addWidget(m_log, 1);
}

void WebAudit::runTool() {
    if (m_proc) return;
    QStringList args = QProcess::splitCommand(m_args->text());
    if (!m_target->text().isEmpty()) args << m_target->text();
    m_log->append(QString("$ %1 %2").arg(kBackend, args.join(" ")));
    m_proc = new QProcess(this);
    connect(m_proc, &QProcess::readyReadStandardOutput, this, &WebAudit::onOut);
    connect(m_proc, &QProcess::readyReadStandardError, this, &WebAudit::onErr);
    connect(m_proc, &QProcess::finished, this, &WebAudit::onDone);
    m_proc->start(kBackend, args);
    m_btnRun->setEnabled(false); m_btnStop->setEnabled(true);
}

void WebAudit::stopTool() {
    if (m_proc) { m_log->append("[stopping…]"); m_proc->kill(); }
}

void WebAudit::onOut() { m_log->append(QString::fromLocal8Bit(m_proc->readAllStandardOutput()).trimmed()); }
void WebAudit::onErr() { m_log->append(QString::fromLocal8Bit(m_proc->readAllStandardError()).trimmed()); }

void WebAudit::onDone(int code) {
    m_log->append(QString("[exit %1]").arg(code));
    m_proc->deleteLater(); m_proc = nullptr;
    m_btnRun->setEnabled(true); m_btnStop->setEnabled(false);
}

void WebAudit::showAbout() {
    QMessageBox::about(this, "About WebAudit",
        "WebAudit v0.1.0 — © 2026 oday, All Rights Reserved (Proprietary).\n\n"
        "Thin Qt GUI over nikto (theirs). Qt toolkit remains under\n"
        "GNU (L)GPL by The Qt Company Ltd. See qt.io/licensing.");
}
