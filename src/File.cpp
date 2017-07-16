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

#include "File.h"
#include <QFile>
#include <QDebug>

File::File(QObject *parent) : QObject(parent) {

}

void File::setFilename(QString filename) {
    this->_filename = filename.replace("file:///", "/").replace("file://", "/");
    emit filenameChanged();
}

QString File::readText() {
    QFile file(this->_filename);
    file.open(QFile::ReadOnly | QFile::Text);
    return QString::fromUtf8(file.readAll());
}

QString File::readBase64() {
    QFile file(this->_filename);
    file.open(QFile::ReadOnly);
    return QString::fromLatin1(file.readAll().toBase64());
}

void File::writeBase64(QString base64) {
    QFile file(this->_filename);
    file.open(QFile::WriteOnly);
    file.write(QByteArray::fromBase64(base64.toLocal8Bit()));
    file.close();
}

void File::writeText(QString text) {
    QFile file(this->_filename);
    file.open(QFile::WriteOnly | QFile::Text);
    file.write(text.toUtf8());
}
