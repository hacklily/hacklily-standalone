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
import QtQuick.Controls 1.4 as Controls1;
import QtQml.Models 2.2;

/**
 * I'm not sure if TreeView is buggy, or if I'm confused.
 */
Controls1.TreeView {
    id: root;

    // Overwrite buggy currentIndex.
    property var currentIndex: root.selection.currentIndex;

    // For convienience
    function getData(role) {
        if (currentIndex === -1) {
            return null;
        }

        return currentIndex.model.data(currentIndex, role);
    }

    // I think this is what https://bugreports.qt.io/browse/QTBUG-45184 is about, despite neither party realizing it.
    selection: ItemSelectionModel {
        model: root.model;
    }
    selectionMode: Controls1.SelectionMode.SingleSelection;

    Component.onCompleted: {
        root.trySelect();
    }

    function trySelect() {
        if (currentIndex.row === -1) {
            var index = root.model.index(0, 0);
            root.selection.setCurrentIndex(index, ItemSelectionModel.SelectCurrent);
            root.selection.select(index, ItemSelectionModel.ClearAndSelect);
        }
        root.forceActiveFocus();
    }

    Connections {
        target: root.model;
        onRowsInserted: root.trySelect();
    }
}
