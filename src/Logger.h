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

#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject {
    Q_OBJECT
public:
    Logger(QObject* parent = NULL);
    static void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
    Q_INVOKABLE QString getTheLog() { return _theLog; }

private:
    static QByteArray _theLog;
};

#endif // LOGGER_H
