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

#include "RemoteFiles.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>

RemoteFiles::RemoteFiles(QObject *parent) :
    QAbstractListModel(parent),
    _cache("Nettek", "hacklily-remote-file-cache") {

    QJsonArray files = QJsonDocument::fromJson(_cache.value("files").toByteArray()).array();
    for (int i = 0; i < files.size(); ++i) {
        RemoteFile file;
        file.filename = files.at(i).toObject()["filename"].toString();
        file.sha = files.at(i).toObject()["sha"].toString();
        file.contents = files.at(i).toObject()["contents"].toString().toUtf8();
        file.hasContents = files.at(i).toObject()["hasContents"].toBool();
        _impl.append(file);
    }
}


QHash<int, QByteArray> RemoteFiles::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[FilenameRole] = "filename";
    roles[SHARole] = "sha";
    roles[ContentsRole] = "contents";
    roles[HasContentsRole] = "hasContents";
    return roles;
}

QVariant RemoteFiles::data(const QModelIndex &index, int role) const {
    int i = index.row();
    if (i >= 0 && i < _impl.size()) {
        switch (role) {
            case FilenameRole:
            default:
                return _impl[i].filename;
            case SHARole:
                return _impl[i].sha;
            case ContentsRole:
                return _impl[i].contents;
            case HasContentsRole:
                return _impl[i].hasContents;
        }
    }
    return QVariant();
}

void RemoteFiles::remove(QString filename) {
    for (int i = 0; i < _impl.size(); ++i) {
        if (_impl[i].filename == filename) {
            beginRemoveRows(index(0), i, i);
            _impl.removeAt(i);
            endRemoveRows();
            --i;
        }
    }
    _updateCache();
}

void RemoteFiles::setSHA(QString filename, QString sha) {
    bool found = false;
    for (int i = 0; i < _impl.size(); ++i) {
        if (_impl[i].filename == filename) {
            RemoteFile file = _impl[i];
            found = true;
            if (file.sha != sha) {
                beginRemoveRows(index(0), i, i);
                _impl.removeAt(i);
                endRemoveRows();
                beginInsertRows(index(0), i, i);
                file.sha = sha;
                file.contents = "";
                file.hasContents = false;
                _impl.insert(i, file);
                endInsertRows();
            }
        }
    }

    if (!found) {
        RemoteFile file;
        file.filename = filename;
        file.sha = sha;
        file.hasContents = false;

        beginInsertRows(QModelIndex(), _impl.size(), _impl.size());
        _impl.insert(_impl.size(), file);
        endInsertRows();
    }
    _updateCache();
}

void RemoteFiles::setContents(QString filename, QString sha, QByteArray contents) {
    bool found = false;

    for (int i = 0; i < _impl.size(); ++i) {
        if (_impl[i].filename == filename) {
            RemoteFile file = _impl[i];
            found = true;
            if (file.sha != sha || file.contents != contents) {
                beginRemoveRows(index(0), i, i);
                _impl.removeAt(i);
                endRemoveRows();
                beginInsertRows(index(0), i, i);
                file.sha = sha;
                file.contents = contents;
                file.hasContents = true;
                _impl.insert(i, file);
                endInsertRows();
            }
        }
    }

    if (!found) {
        RemoteFile file;
        file.filename = filename;
        file.sha = sha;
        file.hasContents = true;
        file.contents = contents;

        beginInsertRows(index(0), _impl.size(), _impl.size());
        _impl.insert(_impl.size(), file);
        endInsertRows();
    }
    _updateCache();
}

void RemoteFiles::_updateCache() {
    QJsonArray files;
    for (int i = 0; i < _impl.size(); ++i) {
        QJsonObject file;
        file["filename"] = _impl[i].filename;
        file["sha"] = _impl[i].sha;
        file["contents"] = QString::fromUtf8(_impl[i].contents);
        file["hasContents"] = _impl[i].hasContents;
        files.append(file);
    }

    QJsonDocument doc;
    doc.setArray(files);
    _cache.setValue("files", doc.toJson());
}
