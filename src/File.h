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

#ifndef FILE_H
#define FILE_H

#include <QObject>

class File : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString filename READ getFilename WRITE setFilename NOTIFY filenameChanged)
public:
    explicit File(QObject *parent = 0);
    QString getFilename() { return _filename; }

    Q_INVOKABLE QString readText();
    Q_INVOKABLE QString readBase64();
    Q_INVOKABLE void writeText(QString text);
    Q_INVOKABLE void writeBase64(QString base64);

signals:
    void filenameChanged();

public slots:
    void setFilename(QString filename);

private:
    QString _filename;
};

#endif // FILE_H
