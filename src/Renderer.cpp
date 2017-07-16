/**
 * @license
 * This file is part of Hacklily, a web-based LilyPond editor.
 * Copyright (C) 2017 - present Joshua Netterfield <joshua@nettek.ca>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "Renderer.h"
#include <QProcess>
#include <QDir>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <QDebug>

static int STANDARD_TIMEOUT = 8000;

Renderer::Renderer(QObject *parent) : QObject(parent)
{

}

void Renderer::render(QString id, QString src, QString fileType) {
    if (!_rendererPath.length()) {
        emit error(id, "Renderer path not set.");
    }
    QProcess* renderProc = new QProcess(this);
    qDebug() << "Rendering " << id;

    // tmpDir will die when renderProc dies.
    // when tmpDir dies, it calls the QTemporaryDir folder which deletes the temporary folder.
    TemporaryDirObject* tempDir = new TemporaryDirObject(renderProc);
    renderProc->setWorkingDirectory(tempDir->dir.path());
    QString userContentPath = tempDir->dir.filePath("usercontent.ly");

    QTimer* timeoutTimer = new QTimer(renderProc); // Gets deleted when renderProc finishes.
    timeoutTimer->setInterval(STANDARD_TIMEOUT);
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, &QTimer::timeout, this, &Renderer::handleTimeout);
    timeoutTimer->setProperty("requestID", id);

    if (fileType == "svg") {
        src = "#(ly:set-option 'backend '" + fileType + ")\n" + src;
    } else {
        src = "\n" + src;
    }

    QFile userContent(userContentPath);
    userContent.open(QFile::WriteOnly);
    userContent.write(src.toUtf8());
    userContent.close();

    renderProc->setProperty("requestID", id);
    renderProc->setProcessChannelMode(QProcess::MergedChannels); // i.e., "2>&1"
    renderProc->start(
        _rendererPath + QDir::separator() + "lyp",
        QStringList() << "compile" << "-s" << userContentPath
    );
    connect(renderProc, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
        this, &Renderer::handleRenderCompleted);

}

void Renderer::setRendererPath(QString rendererPath) {
    this->_rendererPath = rendererPath;
    emit rendererPathChanged();
}

void Renderer::handleTimeout() {
    QTimer *timer = static_cast<QTimer*>(sender());
    QString id = timer->property("requestID").toString();
    emit timeout(id);
}

void Renderer::handleRenderCompleted() {
    qDebug() << "Render completed";
    QProcess* renderProc = qobject_cast<QProcess*>(sender());
    QString id = renderProc->property("requestID").toString();

    QDir workingDir(renderProc->workingDirectory());
    QStringList filenames = workingDir.entryList(QStringList(), QDir::Files, QDir::Name);
    QStringList fileContents;
    qDebug() << filenames;
    foreach(QString filename, filenames) {
        if (filename.endsWith(".svg")) {
            QFile file(workingDir.path() + QDir::separator() + filename);
            file.open(QFile::ReadOnly);
            fileContents.push_back(QString::fromUtf8(file.readAll()));
        } else if (filename.endsWith(".pdf")) {
            QFile file(workingDir.path() + QDir::separator() + filename);
            file.open(QFile::ReadOnly);
            fileContents.push_back(file.readAll().toBase64());
        }
    }

    qDebug() << "Rendered" << fileContents << renderProc->readAllStandardOutput();
    emit rendered(id, fileContents, renderProc->readAllStandardOutput());
    renderProc->deleteLater();
}
