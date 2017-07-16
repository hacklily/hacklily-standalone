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

#include "Directory.h"
#include <QDir>
#include <QCryptographicHash>

Directory::Directory(QObject *parent) : QAbstractListModel(parent) {

}

void Directory::setPath(QString path) {
    this->_path = path.replace("file:///", "/").replace("file://", "/");
    scan();
    emit pathChanged();
}

QString Directory::getPath() {
    return this->_path;
}

QStringList Directory::getContents() const {
    return this->_contents;
}

void Directory::scan() {
    QDir dir(this->_path);
    QStringList contents = dir.entryList(QDir::Files);

    beginRemoveRows(index(0), 0, _contents.length());
    _contents.clear();
    endRemoveRows();

    if (this->_path != "") {
        beginInsertRows(index(0), 0, contents.length());
        _contents = contents;
        endInsertRows();
    }

    emit contentsChanged();
}

QHash<int, QByteArray> Directory::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[FilenameRole] = "filename";
    return roles;
}

QVariant Directory::data(const QModelIndex &index, int role) const {
    int i = index.row();
    if (i >= 0 && i < _contents.size()) {
        switch(role) {
            case FilenameRole:
            default:
                return _contents[i];
            case ContentsRole:
                return _getFileContents(_contents[i]);
            case SHARole:
                return QString::fromUtf8(QCryptographicHash::hash(_getFileContents(_contents[i]).toUtf8(), QCryptographicHash::Sha1).toHex());

        }
    }
    return QVariant();
}

QString Directory::_getFileContents(QString filename) const {
    QFile file(_path + QDir::separator() + filename);
    file.open(QFile::ReadOnly);
    return QString::fromUtf8(file.readAll());
}
