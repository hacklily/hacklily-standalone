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

#include <QApplication>
#include <QQmlApplicationEngine>
#include <QtWebEngine>
#include <QtGui/QPlatformSurfaceEvent>
#include <QJsonArray>
#include <QTcpServer>
#include <QMessageBox>

#include "AppResourceManager.h"
#include "Directory.h"
#include "File.h"
#include "Logger.h"
#include "RemoteFiles.h"
#include "Renderer.h"

#include "SingleApplication.h"

static const char* MOD_NAME = "ca.nettek.Hacklily";
static int VER_MAJ = 0;
static int VER_MIN = 1;

int main(int argc, char *argv[]) {
    qInstallMessageHandler(&Logger::logHandler);
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    SingleApplication app(argc, argv, true);
    if (app.isSecondary()) {
        app.sendMessage("focus");
        return 0;
    }

    QtWebEngine::initialize();

    qmlRegisterType<AppResourceManager>(MOD_NAME, VER_MAJ, VER_MIN, "AppResourceManager");
    qmlRegisterType<Directory>(MOD_NAME, VER_MAJ, VER_MIN, "Directory");
    qmlRegisterType<File>(MOD_NAME, VER_MAJ, VER_MIN, "File");
    qmlRegisterType<Logger>(MOD_NAME, VER_MAJ, VER_MIN, "Logger");
    qmlRegisterType<RemoteFiles>(MOD_NAME, VER_MAJ, VER_MIN, "RemoteFiles");
    qmlRegisterType<Renderer>(MOD_NAME, VER_MAJ, VER_MIN, "Renderer");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QLatin1String("qrc:/main.qml")));

    return app.exec();
}
