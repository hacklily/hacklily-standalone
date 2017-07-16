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

#ifndef REMOTEFILES_H
#define REMOTEFILES_H

#include <QAbstractListModel>
#include <QSettings>

struct RemoteFile {
    QString filename;
    QString sha;
    QByteArray contents;
    bool hasContents;
};

class RemoteFiles : public QAbstractListModel {
    Q_OBJECT
public:
    enum Roles {
        FilenameRole = Qt::UserRole + 1, // keep synced with Directory
        SHARole,
        ContentsRole,
        HasContentsRole
    };
    Q_ENUM(Roles)

    explicit RemoteFiles(QObject *parent = 0);

    QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex&) const { return _impl.size(); }
    QVariant data(const QModelIndex &index, int role) const;

    Q_INVOKABLE void remove(QString filename);
    Q_INVOKABLE void setSHA(QString filename, QString sha);
    Q_INVOKABLE void setContents(QString filename, QString sha, QByteArray contents);

private:
    QList<RemoteFile> _impl;
    QSettings _cache;
    void _updateCache();
};

#endif // REMOTEFILES_H
