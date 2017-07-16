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

import QtQuick 2.7;

Rectangle {
    id: placeholderBackground;

    anchors.fill: parent;
    color: "black";

    Rectangle {
        id: placeholderHeaderBorder;

        anchors.top: placeholderBackground.top;
        color: Qt.rgba(0, 0, 0, 0.8);
        height: 1;
        width: parent.width;
    }

    Rectangle {
        id: placeholderEditor;

        anchors.bottom: parent.bottom;
        anchors.right: parent.horizontalCenter;
        anchors.top: placeholderHeaderBorder.top;
        color: Qt.rgba(30/255, 30/255, 30/255, 0.8);
    }
    Rectangle {
        id: placeholderPreview;

        anchors.bottom: parent.bottom;
        anchors.top: placeholderHeaderBorder.top;
        anchors.left: placeholderEditor.right;
        anchors.right: parent.right;
        color: Qt.rgba(1.0, 1.0, 1.0, 0.2);
    }
}

