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
import QtQuick.Controls 2.1;
import QtQuick.Layouts 1.0;
import QtWebEngine 1.4;
import QtWebChannel 1.0;

import QtQuick.Controls 1.4 as Controls1;
import QtQuick.Dialogs 1.2 as Dialogs;

import ca.nettek.Hacklily 0.1;

Dialogs.Dialog {
    property bool didInit: false;
    property AppResourceManager resourceManager;
    id: notificationFrame;

    visible: !resourceManager.isReady && didInit;
    width: 400;
    standardButtons: Dialogs.StandardButton.Cancel;
    modality: Qt.WindowModal;

    ColumnLayout {
        anchors.fill: parent;

        Text {
            id: statusText;

            Layout.maximumWidth: parent.width;
            Layout.minimumWidth: parent.width;
            anchors.left: parent.left;
            bottomPadding: 10;
            horizontalAlignment: Text.AlignHCenter;
            text: {
                switch (resourceManager.status) {
                    case AppResourceManager.CheckingForFrontendUpdates:
                        return qsTr("Checking for updates to Hacklily...");
                    case AppResourceManager.UpdatingFrontend:
                        return qsTr("Updating frontend...");
                    case AppResourceManager.CheckingForBackendUpdates:
                        return qsTr("Checking for updates to Lilypond and lyp...");
                    case AppResourceManager.UpdatingBackend:
                        return qsTr("Updating Lilypond and/or lyp...");
                    case AppResourceManager.UpdateError:
                        return qsTr("Sorry, Hacklily could not fetch components.\nTry again later or file a bug.");
                    case AppResourceManager.HostOutOfDate:
                        return qsTr("Sorry, Hacklily could not be updated automatically.\nYou need to manual update Hacklily from %1").arg(resourceManager.nonStandalone);
                    case AppResourceManager.StartingBackend:
                        return qsTr("Starting Lilypond renderer...");
                    case AppResourceManager.FrontendResourceError:
                        return qsTr("Sorry, Hacklily could not load the editor.");
                    case AppResourceManager.OtherError:
                        return qsTr("Sorry, something went wrong. Please file a bug.");
                    default:
                        return qsTr("Unknown state. This is a BUG.\nPlease report to joshua@nettek.ca");
                }
            }
            wrapMode: Text.Wrap;

        }
        BusyIndicator {
            visible: !(resourceManager.status & AppResourceManager.Ready) && !(resourceManager.status & AppResourceManager.Error);
            anchors.horizontalCenter: parent.horizontalCenter;
        }
    }
}
