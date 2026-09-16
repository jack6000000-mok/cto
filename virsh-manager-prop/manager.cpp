// Copyright (c) 2026 oday. All Rights Reserved.
// VirshManager Proprietary License, see LICENSE file.
#include "manager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QDateTime>

VirshManager::VirshManager(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("VirshManager [Proprietary] — v0.1.0");
    resize(860, 600);
    setStyleSheet("QMainWindow{background:#0a0e14;} QWidget{background:#0a0e14;color:#c8e6c9;}"
                  "QPushButton{border:2px solid #00ff88;border-radius:8px;padding:10px;background:#111927;}"
                  "QPushButton:hover{background:#1a2b3c;}"
                  "QLineEdit{border:1px solid #00ff88;padding:8px;background:#111927;}"
                  "QTableWidget{background:#111927;gridline-color:#1a2b3c;}"
                  "QTextEdit{background:#05070b;color:#00ff88;font-family:monospace;}");

    auto *central = new QWidget(this);
    setCentralWidget(central);
    auto *lay = new QVBoxLayout(central);

    auto *top = new QHBoxLayout();
    m_uri = new QLineEdit("qemu:///system");
    m_uri->setPlaceholderText("libvirt URI, e.g. qemu:///system");
    m_btnRefresh = new QPushButton("⟳ Refresh");
    connect(m_btnRefresh, &QPushButton::clicked, this, &VirshManager::refresh);
    m_btnAbout = new QPushButton("ⓘ About");
    connect(m_btnAbout, &QPushButton::clicked, this, &VirshManager::showAbout);
    top->addWidget(m_uri, 1); top->addWidget(m_btnRefresh); top->addWidget(m_btnAbout);
    lay->addLayout(top);

    m_table = new QTableWidget(0, 3);
    m_table->setHorizontalHeaderLabels({"ID", "Name", "State"});
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->setSelectionBehavior(QTableWidget::SelectRows);
    m_table->setEditTriggers(QTableWidget::NoEditTriggers);
    lay->addWidget(m_table, 2);

    auto *row = new QHBoxLayout();
    m_btnStart = new QPushButton("▶ Start");
    m_btnShutdown = new QPushButton("■ Shutdown");
    m_btnDestroy = new QPushButton("✘ Force-off");
    connect(m_btnStart, &QPushButton::clicked, this, &VirshManager::startVm);
    connect(m_btnShutdown, &QPushButton::clicked, this, &VirshManager::shutdownVm);
    connect(m_btnDestroy, &QPushButton::clicked, this, &VirshManager::destroyVm);
    row->addWidget(m_btnStart); row->addWidget(m_btnShutdown); row->addWidget(m_btnDestroy);
    row->addStretch();
    lay->addLayout(row);

    m_log = new QTextEdit();
    m_log->setReadOnly(true);
    m_log->append("VirshManager v0.1.0 — proprietary. Uses system virsh; Qt under LGPL.");
    lay->addWidget(m_log, 1);

    refresh();
}

QString VirshManager::selectedVm() const {
    auto items = m_table->selectedItems();
    if (items.isEmpty()) return {};
    return m_table->item(items.first()->row(), 1)->text();
}

void VirshManager::runAndLog(const QString &prog, const QStringList &args) {
    m_log->append(QString("$ %1 %2").arg(prog, args.join(" ")));
    QProcess p;
    p.start(prog, args);
    p.waitForFinished(15000);
    QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
    QString err = QString::fromLocal8Bit(p.readAllStandardError());
    if (!out.isEmpty()) m_log->append(out.trimmed());
    if (!err.isEmpty()) m_log->append(QString("[stderr] ") + err.trimmed());
    if (p.exitCode() != 0) m_log->append(QString("[exit %1]").arg(p.exitCode()));
}

void VirshManager::refresh() {
    QProcess p;
    p.start("virsh", {"-c", m_uri->text(), "list", "--all"});
    p.waitForFinished(15000);
    QString out = QString::fromLocal8Bit(p.readAllStandardOutput());
    m_table->setRowCount(0);
    m_log->append(QString("[%1] refreshed %2").arg(QDateTime::currentDateTime().toString("hh:mm:ss"), m_uri->text()));
    for (const QString &line : out.split("\n")) {
        QString t = line.trimmed();
        if (t.isEmpty() || t.startsWith("Id") || t.startsWith("---")) continue;
        QStringList parts = t.split(" ", Qt::SkipEmptyParts);
        if (parts.size() < 3) continue;
        int r = m_table->rowCount();
        m_table->insertRow(r);
        m_table->setItem(r, 0, new QTableWidgetItem(parts[0]));
        m_table->setItem(r, 1, new QTableWidgetItem(parts[1]));
        m_table->setItem(r, 2, new QTableWidgetItem(parts.mid(2).join(" ")));
    }
    if (m_table->rowCount() == 0) m_log->append("(no domains — create one in virt-manager first)");
}

void VirshManager::runVirsh(const QStringList &args) {
    QString vm = selectedVm();
    if (vm.isEmpty()) {
        QMessageBox::information(this, "No VM", "Select a VM row first.");
        return;
    }
    QStringList full = QStringList{"-c", m_uri->text()} + args + QStringList{vm};
    runAndLog("virsh", full);
    refresh();
}

void VirshManager::startVm() { runVirsh({"start"}); }
void VirshManager::shutdownVm() { runVirsh({"shutdown"}); }
void VirshManager::destroyVm() {
    if (QMessageBox::question(this, "Force off?", "Destroy (force power-off) selected VM?") == QMessageBox::Yes)
        runVirsh({"destroy"});
}

void VirshManager::showAbout() {
    QMessageBox::about(this, "About VirshManager",
        "VirshManager v0.1.0 — © 2026 oday, All Rights Reserved (Proprietary).\n\n"
        "Thin Qt GUI over system virsh/libvirt. Qt toolkit remains under\n"
        "GNU (L)GPL by The Qt Company Ltd. See qt.io/licensing.");
}
