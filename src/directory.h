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

#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <QAbstractListModel>
#include <QHash>

class Directory : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(QString path READ getPath WRITE setPath NOTIFY pathChanged)
    Q_PROPERTY(QStringList contents READ getContents NOTIFY contentsChanged)
public:
    enum Roles {
        FilenameRole = Qt::UserRole + 1, // Keep synced with RemoteFiles
        SHARole,
        ContentsRole,
    };
    Q_ENUM(Roles)

    QHash<int, QByteArray> roleNames() const;
    virtual int rowCount(const QModelIndex&) const { return getContents().size(); }
    QVariant data(const QModelIndex &index, int role) const;

    explicit Directory(QObject *parent = 0);
    void setPath(QString path);
    QString getPath();

    QStringList getContents() const;

signals:
    void pathChanged();
    void contentsChanged();

public slots:
    void scan();

private:
    QString _path;
    QStringList _contents;
    QString _getFileContents(QString filename) const;
};

#endif // DIRECTORY_H
