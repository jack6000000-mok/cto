#!/usr/bin/env python3
"""CTO Empire fleet scaffolder — stamps proprietary Qt GUI skeletons over CLI backends.
Usage: python3 scaffold.py  (run from ~/Projects/cto)
Own Qt code = proprietary. Backends stay theirs. Qt dynamically linked (LGPL).
"""
import os, textwrap

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

TOOLS = [
 dict(d="portscope", cls="PortScope", exe="portscope", backend="nmap",
      color="#00ff88", desc="PortScope — scan profiler + history over nmap.",
      args="-sV -sC -Pn", target="127.0.0.1", warn=""),
 dict(d="packetscope", cls="PacketScope", exe="packetscope", backend="tshark",
      color="#00ffee", desc="PacketScope — capture viewer over tshark.",
      args="-i any -c 100", target="", warn="Needs root/cap_net_raw for live capture."),
 dict(d="webaudit", cls="WebAudit", exe="webaudit", backend="nikto",
      color="#7a00ff", desc="WebAudit — web server scanner over nikto.",
      args="-h", target="http://127.0.0.1/", warn="Only scan targets you own or are authorized to test."),
 dict(d="injectprobe", cls="InjectProbe", exe="injectprobe", backend="sqlmap",
      color="#ffdd57", desc="InjectProbe — injection tester over sqlmap.",
      args="--batch --level=1", target="http://127.0.0.1/?id=1",
      warn="LAB TARGETS ONLY. Unauthorized testing is illegal."),
 dict(d="credaudit", cls="CredAudit", exe="credaudit", backend="hydra",
      color="#ff8000", desc="CredAudit — credential auditor over hydra.",
      args="-l admin -P /usr/share/wordlists/rockyou.txt", target="ssh://127.0.0.1",
      warn="LAB TARGETS ONLY. Unauthorized testing is illegal."),
 dict(d="hashbench", cls="HashBench", exe="hashbench", backend="hashcat",
      color="#ff0040", desc="HashBench — hash auditor + benchmarks over hashcat.",
      args="-b", target="", warn="Audit only hashes you own or are authorized to test."),
 dict(d="airaudit", cls="AirAudit", exe="airaudit", backend="aircrack-ng",
      color="#00aaff", desc="AirAudit — wireless auditor over aircrack-ng.",
      args="--help", target="", warn="Needs monitor-mode interface. Only your own networks / lab."),
 dict(d="vulnlaunch", cls="VulnLaunch", exe="vulnlaunch", backend="virsh",
      color="#00ff88", desc="VulnLaunch — vulnerable-lab VM launcher over virsh.",
      args="-c qemu:///system list --all", target="", warn="Boot lab images (Metasploitable/DVWA) isolated from prod networks."),
]

HEADER = "// Copyright (c) 2026 oday. All Rights Reserved.\n// Proprietary License, see LICENSE file.\n// Qt toolkit components remain under GNU (L)GPL by The Qt Company Ltd.\n"

def w(path, content):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "w") as f:
        f.write(content)

def gen(t):
    d = os.path.join(ROOT, t["d"])
    exe, cls = t["exe"], t["cls"]
    w(os.path.join(d, "CMakeLists.txt"), textwrap.dedent(f"""\
        cmake_minimum_required(VERSION 3.16)
        project({t['d']} LANGUAGES CXX)
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
        set(CMAKE_AUTOMOC ON)
        find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)
        add_executable({exe} WIN32 main.cpp runner.cpp runner.h)
        target_link_libraries({exe} PRIVATE Qt6::Widgets Qt6::Gui Qt6::Core)
        """))
    w(os.path.join(d, "runner.h"), HEADER + textwrap.dedent(f"""\
        #pragma once
        #include <QMainWindow>
        #include <QProcess>
        #include <QTextEdit>
        #include <QLineEdit>
        #include <QPushButton>

        class {cls} : public QMainWindow {{
            Q_OBJECT
        public:
            explicit {cls}(QWidget *parent = nullptr);
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
        }};
        """))
    w(os.path.join(d, "runner.cpp"), HEADER + textwrap.dedent(f"""\
        #include "runner.h"
        #include <QVBoxLayout>
        #include <QHBoxLayout>
        #include <QLabel>
        #include <QMessageBox>

        static const char *kBackend = "{t['backend']}";
        static const char *kDesc = "{t['desc']}";

        {cls}::{cls}(QWidget *parent) : QMainWindow(parent) {{
            setWindowTitle("{cls} [Proprietary] — v0.1.0");
            resize(820, 560);
            setStyleSheet("QMainWindow{{background:#0a0e14;}} QWidget{{background:#0a0e14;color:#c8e6c9;}}"
                          "QPushButton{{border:2px solid {t['color']};border-radius:8px;padding:10px;background:#111927;}}"
                          "QPushButton:hover{{background:#1a2b3c;}}"
                          "QPushButton:disabled{{color:#555;border-color:#333;}}"
                          "QLineEdit{{border:1px solid {t['color']};padding:8px;background:#111927;}}"
                          "QTextEdit{{background:#05070b;color:{t['color']};font-family:monospace;}}");
            auto *central = new QWidget(this);
            setCentralWidget(central);
            auto *lay = new QVBoxLayout(central);
            auto *title = new QLabel(QString("%1  •  backend: %2").arg(kDesc, kBackend));
            title->setWordWrap(true);
            title->setStyleSheet("font-size:15px;font-weight:bold;");
            lay->addWidget(title);
        """) + (textwrap.dedent(f"""\
            auto *warn = new QLabel("{t['warn']}");
            warn->setWordWrap(true);
            warn->setStyleSheet("color:#ff5577;font-weight:bold;");
            lay->addWidget(warn);
        """) if t["warn"] else "") + textwrap.dedent(f"""\
            auto *row = new QHBoxLayout();
            m_target = new QLineEdit("{t['target']}");
            m_target->setPlaceholderText("target");
            m_args = new QLineEdit("{t['args']}");
            m_args->setPlaceholderText("args");
            m_btnRun = new QPushButton("▶ Run");
            m_btnStop = new QPushButton("■ Stop");
            m_btnStop->setEnabled(false);
            m_btnAbout = new QPushButton("ⓘ About");
            connect(m_btnRun, &QPushButton::clicked, this, &{cls}::runTool);
            connect(m_btnStop, &QPushButton::clicked, this, &{cls}::stopTool);
            connect(m_btnAbout, &QPushButton::clicked, this, &{cls}::showAbout);
            row->addWidget(m_target, 1); row->addWidget(m_args, 2);
            row->addWidget(m_btnRun); row->addWidget(m_btnStop); row->addWidget(m_btnAbout);
            lay->addLayout(row);
            m_log = new QTextEdit();
            m_log->setReadOnly(true);
            m_log->append(QString("{cls} v0.1.0 — proprietary GUI. Backend: %1 (theirs).").arg(kBackend));
            lay->addWidget(m_log, 1);
        }}

        void {cls}::runTool() {{
            if (m_proc) return;
            QStringList args = QProcess::splitCommand(m_args->text());
            if (!m_target->text().isEmpty()) args << m_target->text();
            m_log->append(QString("$ %1 %2").arg(kBackend, args.join(" ")));
            m_proc = new QProcess(this);
            connect(m_proc, &QProcess::readyReadStandardOutput, this, &{cls}::onOut);
            connect(m_proc, &QProcess::readyReadStandardError, this, &{cls}::onErr);
            connect(m_proc, &QProcess::finished, this, &{cls}::onDone);
            m_proc->start(kBackend, args);
            m_btnRun->setEnabled(false); m_btnStop->setEnabled(true);
        }}

        void {cls}::stopTool() {{
            if (m_proc) {{ m_log->append("[stopping…]"); m_proc->kill(); }}
        }}

        void {cls}::onOut() {{ m_log->append(QString::fromLocal8Bit(m_proc->readAllStandardOutput()).trimmed()); }}
        void {cls}::onErr() {{ m_log->append(QString::fromLocal8Bit(m_proc->readAllStandardError()).trimmed()); }}

        void {cls}::onDone(int code) {{
            m_log->append(QString("[exit %1]").arg(code));
            m_proc->deleteLater(); m_proc = nullptr;
            m_btnRun->setEnabled(true); m_btnStop->setEnabled(false);
        }}

        void {cls}::showAbout() {{
            QMessageBox::about(this, "About {cls}",
                "{cls} v0.1.0 — © 2026 oday, All Rights Reserved (Proprietary).\\n\\n"
                "Thin Qt GUI over {t['backend']} (theirs). Qt toolkit remains under\\n"
                "GNU (L)GPL by The Qt Company Ltd. See qt.io/licensing.");
        }}
        """))
    w(os.path.join(d, "main.cpp"), HEADER + textwrap.dedent(f"""\
        #include <QApplication>
        #include "runner.h"
        int main(int argc, char *argv[]) {{
            QApplication app(argc, argv);
            {cls} w;
            w.show();
            return app.exec();
        }}
        """))
    w(os.path.join(d, "LICENSE"), textwrap.dedent(f"""\
        {cls.upper()} PROPRIETARY LICENSE (v1.0)

        Copyright (c) 2026 oday. All Rights Reserved.

        1. GRANT. Non-exclusive, non-transferable license to install and use
           one copy of {cls} ("Software") on a single device.
        2. RESTRICTIONS. No copy, redistribution, sublicense, rent, lease,
           lend, or reverse-engineering (except where prohibited by law).
           No removal of proprietary notices.
        3. OWNERSHIP. Licensed, not sold. All rights remain with licensor.
        4. THIRD-PARTY COMPONENTS. Backend tool ({t['backend']}) and the Qt
           toolkit keep their own licenses (Qt: GNU (L)GPL, The Qt Company).
           Qt source: https://www.qt.io.
        5. NO WARRANTY. Provided "AS IS". 6. TERMINATION on breach.

        Contact: oday@local
        """))
    w(os.path.join(d, f"{exe}.desktop"), textwrap.dedent(f"""\
        [Desktop Entry]
        Type=Application
        Name={cls}
        Comment={t['desc']}
        Exec={exe}
        Icon={exe}
        Terminal=false
        Categories=System;Security;
        Keywords=pentest;security;{t['backend']};
        StartupNotify=true
        """))
    w(os.path.join(d, f"{exe}.svg"), textwrap.dedent(f"""\
        <svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 128 128">
          <rect width="128" height="128" rx="20" fill="#0a0e14"/>
          <rect x="20" y="22" width="88" height="60" rx="8" fill="none" stroke="{t['color']}" stroke-width="5"/>
          <text x="64" y="66" font-family="monospace" font-size="34" font-weight="bold" fill="{t['color']}" text-anchor="middle">{cls[0]}</text>
          <rect x="52" y="86" width="24" height="8" fill="{t['color']}"/>
          <rect x="34" y="96" width="60" height="7" rx="3.5" fill="{t['color']}" opacity="0.7"/>
        </svg>
        """))
    print("scaffolded", t["d"])

for t in TOOLS:
    gen(t)
print("fleet done:", len(TOOLS))
