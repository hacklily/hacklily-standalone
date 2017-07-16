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
import QtWebEngine 1.5;
import QtWebChannel 1.0;

import QtQuick.Controls 1.4 as Controls1;
import QtQuick.Controls.Styles 1.4 as Controls1Style;
import QtQuick.Dialogs 1.2 as Dialogs;

import ca.nettek.Hacklily 0.1;

Controls1.ApplicationWindow {
    id: appWindow;
    flags: Qt.Window | Qt.WindowFullscreenButtonHint;
    minimumHeight: 500;
    minimumWidth: 700;
    title: qsTr("Hacklily");
    visibility: ApplicationWindow.Maximized;
    visible: true;

    onHeightChanged: {
        webEngineView.resizing = 1;
    }
    onWidthChanged: {
        webEngineView.resizing = 1;
    }

    Component.onCompleted: {
        // Workaround for Mac where if the progress view shows up before the window,
        // it doesn't get docked to the window.
        progressView.didInit = true;
    }

    menuBar: HacklilyMenuBar {
        appIsActive: webEngineView.appIsActive && !openHacklilySongDialog.visible && !saveAsDialog.visible;
        onNewSongClicked: webEngineBridge.onNewSong();
        onOpenClicked: webEngineBridge.onOpen();
        onImportClicked: webEngineBridge.onRequestImport();
        onSaveClicked: webEngineBridge.onSave();
        onExportClicked: {
            exportDialog.visible = true;
        }
        onCopyLinkClicked: {
            throw new Error("Not implemented");
        }
        onSetLocalMusicDirectoryClicked: localFilesDialog.open();
        onQuitClicked: {
            appWindow.hide();
            Qt.quit();
        }
        onUndoClicked: webEngineView.triggerWebAction(WebEngineView.Undo);
        onRedoClicked: webEngineView.triggerWebAction(WebEngineView.Redo);
        onCutClicked: webEngineView.triggerWebAction(WebEngineView.Cut);
        onCopyClicked: webEngineView.triggerWebAction(WebEngineView.Copy);
        onPasteClicked: webEngineView.triggerWebAction(WebEngineView.Paste);
        onSelectAllClicked: webEngineBridge.onSelectAll();
        onFindClicked: webEngineBridge.onFind();
        onFindAgainClicked: webEngineBridge.onFindNext();
        onViewModeClicked: webEngineBridge.onViewMode();
        onSplitModeClicked: webEngineBridge.onSplitMode();
        onCodeModeClicked: webEngineBridge.onCodeMode();
        onMyAccountClicked: {
            throw new Error("Not implemented");
        }
        onSignOutClicked: {
            // webEngineView->page()->profile()->cookieStore()->deleteAllCookies();
            throw new Error("Not implemented");
        }
        onAboutHacklilyClicked: webEngineBridge.onAboutHacklily();
        onAboutQtClicked: resourceManager.aboutQt();
        onLilypondDocumentationClicked: Qt.openUrlExternally("http://lilypond.org/doc/v2.18/Documentation/learning/index");
        onSaveDebugLogsClicked: saveLogDialog.open();
        onReportBugClicked: Qt.openUrlExternally("https://github.com/hacklily/hacklily/issues/new");
    }

    ProgressView {
        id: progressView;
        resourceManager: resourceManager
        onRejected: {
            appWindow.hide();
            Qt.quit();
        }
    }


    Rectangle {
        anchors.fill: parent;
        color: "black";
        Placeholder {

        }
        WebEngineView {
            id: webEngineView;

            property bool appShouldBeVisible: resourceManager.isReady && (!webEngineView.loading && !webEngineView.resizing || !viewingSPA);
            property bool appIsActive: webEngineView.appShouldBeVisible && viewingSPA;
            property bool viewingSPA: webEngineView.url.toString().indexOf(resourceManager.standaloneSPA) == 0;
            onAppIsActiveChanged: {
                console.log("App is active", appIsActive);
            }
            onUrlChanged: {
                console.log("URL:", url, webEngineView.appShouldBeVisible, viewingSPA, resourceManager.standaloneSPA);
            }
            onAppShouldBeVisibleChanged: {
                console.log("App should be visible", appShouldBeVisible);
            }

            property bool isInOauthFlow: false;
            property int resizing: 0;

            anchors.centerIn: parent;
            backgroundColor: "transparent";
            height: webEngineView.isInOauthFlow ? Math.min(800, parent.height - 30) : parent.height;
            opacity: webEngineView.appShouldBeVisible ? 0.01 : 0.0;
            url: resourceManager.isReady ? resourceManager.standaloneSPA : "";
            visible: resourceManager.isReady;
            width: webEngineView.isInOauthFlow ? 500 : parent.width;
            webChannel: WebChannel {
                registeredObjects: [localFiles, webEngineBridge];
            }
            onNavigationRequested: {
                console.log("Navigate to", request.url);
                if (request.url.toString().indexOf("https://github.com/login") == 0 ||
                        request.url.toString().indexOf("https://github.com/session") == 0 ) {
                    console.log("  (Oauth flow)");
                    webEngineView.isInOauthFlow = true;
                } else if (request.url.toString().indexOf(resourceManager.standaloneRoot) === 0 || request.url.toString() === webEngineView.url.toString()) {
                    if (request.url.toString() === resourceManager.standaloneRoot + "/") {
                        console.log(  '(Standalone root, with implicit index.html...)')
                        webEngineView.url = resourceManager.standaloneRoot + "/index.html";
                        request.action = WebEngineNavigationRequest.IgnoreRequest;
                    } else {
                        console.log("  (Standalone root)");
                        webEngineView.isInOauthFlow = false;
                    }
                } else if (request.url.toString() === "qrc:/") {
                    console.log("  (BUG: false root)");
                    webEngineView.isInOauthFlow = false;
                    webEngineView.url = resourceManager.standaloneSPA;
                    request.action = WebEngineNavigationRequest.IgnoreRequest;
                } else if (request.url.toString().indexOf(resourceManager.nonStandalone + "/?") === 0 && webEngineView.isInOauthFlow) { // OAuth!
                    console.log("  (Non-standalone -- redirect)");
                    var realURL = request.url.toString().replace(resourceManager.nonStandalone + "/?", resourceManager.standaloneSPA + "?");
                    webEngineView.url = realURL;
                    request.action = WebEngineNavigationRequest.IgnoreRequest;
                } else {
                    console.log("  (External)");
                    Qt.openUrlExternally(request.url);
                    request.action = WebEngineNavigationRequest.IgnoreRequest;
                }
            }

            onNewViewRequested: {
                Qt.openUrlExternally(request.requestedUrl);
            }

            onJavaScriptConsoleMessage: {
                console.log(sourceID, lineNumber, message);
            }

            SequentialAnimation {
                running: webEngineView.appShouldBeVisible;

                PauseAnimation {
                    duration: webEngineView.isInOauthFlow ? 500 : 100;
                }

                OpacityAnimator {
                    duration: 200;
                    easing.type: Easing.InOutQuad;
                    from: 0.01;
                    target: webEngineView;
                    to: 1.0;
                }
            }

            SequentialAnimation {
                running: webEngineView.resizing;

                PauseAnimation {
                    duration: 20;
                }

                PropertyAnimation {
                    duration: 0;
                    target: webEngineView;
                    property: "resizing";
                    from: 1;
                    to: 0;
                }
            }
        }
        QtObject {
            id: webEngineBridge;
            WebChannel.id: "webContentsBridge";
            property bool unsavedChangesModalVisible: false;
            property bool saveAsVisible: false;
            property bool openDialogVisible: false;
            property bool importVisible: false;
            property bool exportVisible: false;
            property bool savingVisible: false;
            property var remoteFiles: []
            onRemoteFilesChanged: {
                var files = remoteFiles.filter(function(file) { return file.path.lastIndexOf(".ly") === file.path.length - 3; });
                for (var i = 0; i < files.length; ++i) {
                    remoteFileManager.setSHA(files[i].path, files[i].sha);
                }
            }

            signal onNewSong();
            signal onOpen();
            signal onImport(string name, string src);
            signal onImportRejected();
            signal onSave();
            signal onSelectAll();
            signal onFind();
            signal onFindNext();
            signal onViewMode();
            signal onSplitMode();
            signal onCodeMode();
            signal onAboutHacklily();
            signal onRequestImport();

            signal onExportRequested(string filename);
            function finishUpExport(filename, type) {
                throw new Error("Not implemented");
            }

            signal unsavedChangesSave();
            signal unsavedChangesCancel();
            signal unsavedChangesDiscard();

            signal openCancel();
            signal openFile(string file, string source, string sha, string contents);

            signal saveAsCancel();
            signal saveAsFile(string file, string source);

            signal renderCompleted(string id, var result, string log);
            signal renderError(string error);

            function render(id, src, fileType) {
                renderer.render(id, src, fileType);
            }

            function save(src, filename, pdf) {
                exportFile.filename = resourceManager.localFilePath + "/" + filename;
                exportFile.writeText(src);

                exportFile.filename = resourceManager.localFilePath + "/" + filename.replace(/\.ly$/, '.pdf');
                exportFile.writeBase64(pdf);
            }
        }
    }

    /* --- Lilypond --- */

    AppResourceManager {
        id: resourceManager;
        property bool isReady: resourceManager.status & AppResourceManager.Ready;
        onShouldFocus: {
            appWindow.raise();
        }
    }

    Renderer {
        id: renderer;
        rendererPath: resourceManager.rendererPath;
        onRendered: {
            webEngineBridge.renderCompleted(id, result, log);
        }
        onTimeout: {
            webEngineBridge.renderError(id, "Render timeout");
        }
        onError: {
            webEngineBridge.renderError(id, error);
        }
    }

    /* --- saving log files --- */

    Logger {
        id: logger;
    }

    Dialogs.FileDialog {
        id: saveLogDialog;
        selectExisting: false;
        nameFilters: ["Log file (*.txt)"];
        modality: Qt.WindowModal;
        selectedNameFilterIndex: 0;
        folder: shortcuts.home;
    }

    File {
        id: logFile;
        filename: saveLogDialog.fileUrl;
        onFilenameChanged: {
            if (filename) {
                console.log("Writing log to ", filename);
                logFile.writeText(logger.getTheLog());
                logSavedDialog.open();
            }
        }
    }

    Dialogs.MessageDialog {
        id: logSavedDialog;
        title: qsTr("Saved");
        text: qsTr("Logs have been saved to '%1'.").arg(saveLogDialog.fileUrl)
    }

    /* --- importing --- */

    Dialogs.FileDialog {
        id: importDialog;
        visible: webEngineBridge.importVisible;
        selectExisting: true;
        nameFilters: ["Lilypond file (*.ly)"];
        modality: Qt.WindowModal;
        selectedNameFilterIndex: 0;
        folder: shortcuts.documents;
        onAccepted: {
            if (fileUrl) {
                importFile.filename = fileUrl;
                webEngineBridge.onImport(fileUrl, importFile.readText());
            } else {
                webEngineBridge.onImportRejected();
            }
        }

        onRejected: {
            webEngineBridge.onImportRejected();
        }


    }

    File {
        id: importFile;
    }

    /* --- exporting --- */

    Dialogs.FileDialog {
        id: exportDialog;
        selectExisting: false;
        nameFilters: ["Lilypond file (*.ly)", "PDF file (*.pdf)"];
        modality: Qt.WindowModal;
        selectedNameFilterIndex: 0;
        folder: shortcuts.documents;
        onAccepted: {
            exportFile.filename = fileUrl;
            if (fileUrl.indexOf(".ly") === fileUrl.length - ".ly".length) {
                console.log('Write Ly');
            } else {
                console.log('Write Pdf');
            }
        }
    }

    File {
        id: exportFile;
    }

    /* --- local files --- */

    Dialogs.FileDialog {
        id: localFilesDialog;
        selectFolder: true;
        modality: Qt.WindowModal;
        folder: shortcuts.documents;

        onFileUrlChanged: {
            resourceManager.localFilePath = fileUrl;
        }
    }

    Directory {
        WebChannel.id: "localFiles";
        id: localFiles;
        path: resourceManager.localFilePath;
        Component.onCompleted: {
            console.log("Local file path is ", path);
        }
    }

    /* --- remote files --- */

    RemoteFiles {
        id: remoteFileManager;
    }

    /** --- Changes not saved dialog --- */

    Dialogs.Dialog {
        id: unsavedChangesDialog;
        visible: webEngineBridge.unsavedChangesModalVisible;
        width: 500;

        title: qsTr("Unsaved changes");
        RowLayout {
            anchors.fill: parent;
            Image {
                source: "qrc:/hacklily.png";
                fillMode: Image.PreserveAspectFit;
                Layout.maximumWidth: 80;
                Layout.maximumHeight: 80;
            }
            ColumnLayout {
                Layout.rightMargin: 10;
                Text {
                    text: qsTr('Do you want to save the changes you made to this document?');
                    wrapMode: Text.WordWrap;
                    Layout.topMargin: 10;
                    Layout.fillWidth: true;
                    font.pointSize: 14;
                    font.bold: true;
                }
                Text {
                    text: qsTr('Your changes will be lost if you do not save them.');
                    Layout.fillWidth: true;
                    wrapMode: Text.WordWrap;
                    font.pointSize: 12;
                }
                Rectangle {
                    height: 20;
                }
            }
        }
        standardButtons: Dialogs.StandardButton.Cancel | Dialogs.StandardButton.Discard | Dialogs.StandardButton.Save;

        onAccepted: {
            webEngineBridge.unsavedChangesSave();
        }

        onRejected: {
            webEngineBridge.unsavedChangesCancel();
        }

        onDiscard: {
            webEngineBridge.unsavedChangesDiscard();
        }
    }

    /** --- Open file --- */

    Dialogs.Dialog {
        id: openHacklilySongDialog;
        visible: webEngineBridge.openDialogVisible;
        title: qsTr("Open song");

        onVisibleChanged: {
            if (openHacklilySongTabView.currentTab) {
                openHacklilySongTabView.currentTab.item.forceActiveFocus();
            }
        }

        RowLayout {
            Controls1.TabView {
                id: openHacklilySongTabView;
                Layout.minimumWidth: 400;
                Layout.minimumHeight: 300;
                property bool ready: false;
                property Item currentTab: ready ? getTab(currentIndex) : null;
                onCurrentTabChanged: {
                    if (currentTab.item) {
                        currentTab.item.forceActiveFocus();
                    }
                }

                Controls1.Tab {
                    title: qsTr("Online files");
                    property Item tableView: item;
                    property bool isLocal: false;
                    anchors.topMargin: 10;
                    anchors.margins: 8;

                    TreeViewWithDefaultSelection {

                        Controls1.TableViewColumn {
                            title: "Name"
                            role: "filename"
                        }
                        model: remoteFileManager;
                        onActivated: {
                            openHacklilySongDialog.accept();
                        }
                    }

                    onActiveChanged: {
                        // Hack -- Qml loads this tab after the parent, so we need to let the parent
                        // know it has completely finished loading and that we can trust the currentTab
                        // property!
                        openHacklilySongTabView.ready = true;
                    }
                }
                Controls1.Tab {
                    title: qsTr("Local files");
                    property Item tableView: item;
                    property bool isLocal: true;
                    anchors.topMargin: 10;
                    anchors.margins: 8;
                    Item {
                        anchors.fill: parent;
                        TreeViewWithDefaultSelection {
                            anchors.fill: parent;
                            visible: resourceManager.localFilePath !== "";

                            Controls1.TableViewColumn {
                                title: "Name"
                                role: "filename"
                            }
                            model: localFiles;
                            onActivated: {
                                openHacklilySongDialog.accept();
                            }
                        }
                        Text {
                            anchors.fill: parent;
                            visible: resourceManager.localFilePath === "";
                            text: qsTr("To see files here, select \"File > Set Local Music Directory...\"");
                            wrapMode: Text.WordWrap;
                            horizontalAlignment: Text.AlignHCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }
                    }
                }
            }
        }
        standardButtons: {
            if (!openHacklilySongTabView.currentTab) {
                return Dialogs.StandardButton.Cancel;
            }

            if (!openHacklilySongTabView.currentTab.tableView.currentIndex ||
                    openHacklilySongTabView.currentTab.tableView.currentIndex.row === -1) {
                return Dialogs.StandardButton.Cancel;
            }

            return Dialogs.StandardButton.Cancel | Dialogs.StandardButton.Open;
        }

        onAccepted: {
            accept();
        }

        function accept() {
            console.assert(Directory.FilenameRole === RemoteFiles.FilenameRole);
            var tableView = openHacklilySongTabView.currentTab.tableView;
            var filename = tableView.getData(Directory.FilenameRole);
            var contents = tableView.getData(RemoteFiles.ContentsRole);
            var sha = tableView.getData(RemoteFiles.SHARole);
            var role = openHacklilySongTabView.currentIndex === 0 ? "remote" : "local";
            webEngineBridge.openFile(filename, role, sha, contents);
        }

        onRejected: {
            webEngineBridge.openCancel(); 
        }
    }

    /** --- Save as --- */

    Dialogs.Dialog {
        id: saveAsDialog;
        visible: webEngineBridge.saveAsVisible;
        title: qsTr("Save / share song");

        RowLayout {
            Controls1.TabView {
                id: saveAsTabView;
                Layout.minimumWidth: 400;
                Layout.minimumHeight: 300;
                property bool ready: false;
                property Item currentTab: ready ? getTab(currentIndex) : null;

                Controls1.Tab {
                    id: tabRoot;
                    title: qsTr("Online files");
                    property Item tableView: item;
                    property bool isLocal: false;
                    property string filename;

                    anchors.topMargin: 10;
                    anchors.margins: 8;

                    onActiveChanged: {
                        // Hack -- Qml loads this tab after the parent, so we need to let the parent
                        // know it has completely finished loading and that we can trust the currentTab
                        // property!
                        console.log("Now ready");
                        saveAsTabView.ready = true;
                    }

                    ColumnLayout {
                        anchors.fill: parent;
                        RowLayout {
                            Text {
                                text: qsTr("Save As:");
                            }

                            Controls1.TextField {
                                id: filenameField;
                                Layout.fillWidth: true;
                                focus: true;
                                placeholderText: qsTr("cello-prelude");
                                onTextChanged: {
                                    text = text.replace(/[^a-zA-Z0-9_-]*/g, '');
                                    tabRoot.filename = text;
                                }
                            }

                            Text {
                                text: qsTr(".ly");
                            }
                        }
                        Controls1.TreeView {
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            selectionMode: Controls1.SelectionMode.NoSelection
                            model: remoteFileManager;
                            enabled: false;

                            Controls1.TableViewColumn {
                                title: "Name"
                                role: "filename"
                            }
                        }
                        Text {
                            text: qsTr("This file will be available publically on the Internet.");
                        }
                    }
                }
                Controls1.Tab {
                    id: tabRoot2;
                    title: qsTr("Local files");
                    property Item tableView: item;
                    property bool isLocal: true;
                    property string filename;
                    anchors.topMargin: 10;
                    anchors.margins: 8;
                    ColumnLayout {
                        anchors.fill: parent;
                        RowLayout {
                            visible: resourceManager.localFilePath !== "";
                            Text {
                                text: qsTr("Save As:");
                            }
                            Controls1.TextField {
                                id: filenameField;
                                Layout.fillWidth: true;
                                placeholderText: qsTr("cello-prelude");
                                onTextChanged: {
                                    text = text.replace(/[^a-zA-Z0-9_-]*/g, '');
                                    tabRoot2.filename = text;
                                }
                            }
                            Text {
                                text: qsTr(".ly");
                            }
                        }
                        Controls1.TreeView {
                            visible: resourceManager.localFilePath !== "";
                            selectionMode: Controls1.SelectionMode.NoSelection;
                            model: localFiles;
                            Layout.fillHeight: true
                            Layout.fillWidth: true
                            enabled: false;

                            Controls1.TableViewColumn {
                                title: "Name"
                                role: "filename"
                            }
                        }
                        Text {
                            visible: resourceManager.localFilePath !== "";
                            text: qsTr("This file will be stored privately on your computer.");
                        }
                        Text {
                            anchors.fill: parent;
                            visible: resourceManager.localFilePath === "";
                            text: qsTr("To save files here, select \"File > Set Local Music Directory...\"");
                            wrapMode: Text.WordWrap;
                            horizontalAlignment: Text.AlignHCenter;
                            verticalAlignment: Text.AlignVCenter;
                        }
                    }
                }
            }
        }
        standardButtons: {
            return Dialogs.StandardButton.Cancel | Dialogs.StandardButton.Save;
        }

        onAccepted: {
            accept();
        }

        function accept() {
            webEngineBridge.saveAsFile(saveAsTabView.currentTab.filename + ".ly", saveAsTabView.currentTab.isLocal ? "local" : "remote");
        }

        onRejected: {
            webEngineBridge.saveAsCancel();
        }
    }

    Dialogs.Dialog {
        visible: webEngineBridge.savingVisible;
        width: 400;
        standardButtons: Dialogs.StandardButton.Cancel;

        ColumnLayout {
            anchors.fill: parent;

            Text {
                id: statusText;

                Layout.maximumWidth: parent.width;
                Layout.minimumWidth: parent.width;
                anchors.left: parent.left;
                bottomPadding: 10;
                horizontalAlignment: Text.AlignHCenter;
                text: qsTr("Saving...");
                wrapMode: Text.Wrap;

            }
            BusyIndicator {
                anchors.horizontalCenter: parent.horizontalCenter;
            }
        }
    }
}
