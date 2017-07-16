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

Controls1.MenuBar {
    property bool appIsActive;

    signal newSongClicked();
    signal openClicked();
    signal importClicked();
    signal saveClicked();
    signal exportClicked();
    signal copyLinkClicked();
    signal setLocalMusicDirectoryClicked();
    signal quitClicked();

    signal undoClicked();
    signal redoClicked();
    signal cutClicked();
    signal copyClicked();
    signal pasteClicked();
    signal selectAllClicked();
    signal findClicked();
    signal findAgainClicked();

    signal viewModeClicked();
    signal splitModeClicked();
    signal codeModeClicked();

    signal myAccountClicked();
    signal signOutClicked();

    signal aboutHacklilyClicked();
    signal aboutLilypondClicked();
    signal aboutQtClicked();
    signal lilypondDocumentationClicked();
    signal saveDebugLogsClicked();
    signal reportBugClicked();

    Controls1.Menu {
        title: qsTr("Hacklily");
    }

    Controls1.Menu {
        title: qsTr("File");
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: newSongClicked();
            shortcut: StandardKey.New;
            text: qsTr("&New song");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: openClicked();
            shortcut: StandardKey.Open;
            text: qsTr("Open...");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: importClicked();
            shortcut: "Ctrl+Alt+O"
            text: qsTr("Import...");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: quitClicked();
            shortcut: StandardKey.Close;
            text: qsTr("&Close");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: saveClicked();
            shortcut: StandardKey.Save;
            text: qsTr("Save");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: exportClicked();
            shortcut: StandardKey.SaveAs;
            text: qsTr("Export...")
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: copyLinkClicked();
            text: qsTr("Copy Link to Clipboard");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: setLocalMusicDirectoryClicked();
            text: qsTr("Set Local Music Directory...");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            onTriggered: quitClicked();
            shortcut: StandardKey.Quit;
            text: qsTr("&Quit Hacklily");
        }
    }

    Controls1.Menu {
        title: qsTr("Edit");
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: undoClicked();
            shortcut: StandardKey.Undo;
            text: qsTr("Undo");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: redoClicked();
            shortcut: StandardKey.Redo;
            text: qsTr("Redo");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: cutClicked();
            shortcut: StandardKey.Cut;
            text: qsTr("Cut");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: copyClicked();
            shortcut: StandardKey.Copy;
            text: qsTr("Copy");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: pasteClicked();
            shortcut: StandardKey.Paste;
            text: qsTr("Paste");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: selectAllClicked();
            shortcut: StandardKey.SelectAll;
            text: qsTr("Select All");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: findClicked();
            shortcut: StandardKey.Find;
            text: qsTr("Find");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: findAgainClicked();
            shortcut: StandardKey.FindNext;
            text: qsTr("Find Again");
        }
    }

    Controls1.Menu {
        title: qsTr("View");
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: viewModeClicked();
            shortcut: "Ctrl+1";
            text: qsTr("View Mode");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: splitModeClicked();
            shortcut: "Ctrl+2";
            text: qsTr("Split Mode");
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: codeModeClicked();
            shortcut: "Ctrl+3";
            text: qsTr("Code Only Mode");
        }
        Controls1.MenuSeparator {
            // For MacOS, so that the hidden full screen mode option appears in a new one.
        }
    }

    Controls1.Menu {
        title: qsTr("Account")
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: myAccountClicked();
            text: qsTr("My Account...")
        }
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: signOutClicked();
            text: qsTr("Sign &Out");
        }
    }

    Controls1.Menu {
        title: qsTr("Help");
        Controls1.MenuItem {
            enabled: appIsActive;
            onTriggered: aboutHacklilyClicked();
            text: qsTr("&About Hacklily");
        }
        Controls1.MenuItem {
            onTriggered: aboutQtClicked();
            text: qsTr("About Qt");
        }
        Controls1.MenuItem {
            onTriggered: lilypondDocumentationClicked();
            shortcut: StandardKey.HelpContents;
            text: qsTr("Lilypond Documentation");
        }
        Controls1.MenuSeparator {

        }
        Controls1.MenuItem {
            onTriggered: saveDebugLogsClicked();
            text: qsTr("Save Debug Logs...");
        }
        Controls1.MenuItem {
            onTriggered: reportBugClicked();
            text: qsTr("Report Bug or Give Feedback...");
        }
    }
}

