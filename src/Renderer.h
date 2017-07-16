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

#ifndef RENDERER_H
#define RENDERER_H

#include <QObject>
#include <QTemporaryDir>

/**
 * @brief QObject wrapper around QTemporaryDir. This exists for resource management.
 * When the parent dies, we can kill this as well.
 */
class TemporaryDirObject : public QObject {
    Q_OBJECT
public:
    TemporaryDirObject(QObject* parent) : QObject(parent) {}
    QTemporaryDir dir;
};

class Renderer : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString rendererPath READ getRendererPath WRITE setRendererPath NOTIFY rendererPathChanged)
public:
    explicit Renderer(QObject *parent = 0);
    Q_INVOKABLE void render(QString id, QString src, QString fileType);
    void setRendererPath(QString rendererPath);
    QString getRendererPath() const { return _rendererPath; }

signals:
    void error(QString id, QString error);
    void rendered(QString id, QStringList result, QString log);
    void timeout(QString id);
    void rendererPathChanged();

private slots:
    void handleTimeout();
    void handleRenderCompleted();

private:
    QString _rendererPath;
};

#endif // RENDERER_H
