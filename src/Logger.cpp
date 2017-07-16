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

#include "Logger.h"

Logger::Logger(QObject* parent) : QObject(parent) {

}

void Logger::logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    QString logItem;
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
        case QtDebugMsg:
        default:
            logItem = QString::asprintf("[d] %s\n      (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtInfoMsg:
            logItem = QString::asprintf("[i] %s\n      (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtWarningMsg:
            logItem = QString::asprintf("[w] %s\n      (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtCriticalMsg:
            logItem = QString::asprintf("[!] %s\n      (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            logItem = QString::asprintf("Fatal: %s\n      (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            abort();
    }
    QByteArray localLogItem = logItem.toLocal8Bit();
    _theLog += localLogItem;
    fprintf(stderr, "%s", localLogItem.constData());
}

QByteArray Logger::_theLog;
