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

#ifndef APPRESOURCEMANAGER_H
#define APPRESOURCEMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QProcess>

class AppResourceManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(Status status READ getStatus NOTIFY statusChanged)
    Q_PROPERTY(float progress READ getProgress NOTIFY progressChanged)
    Q_PROPERTY(bool progressIndeterminate READ getProgressIndeterminate NOTIFY progressIndeterminateChanged)
    Q_PROPERTY(QString localFilePath READ getLocalFilePath WRITE setLocalFilePath NOTIFY localFilePathChanged)

    Q_PROPERTY(QString standaloneRoot READ getStandaloneRoot CONSTANT)
    Q_PROPERTY(QString standaloneSPA READ getStandaloneSPA CONSTANT)
    Q_PROPERTY(QString nonStandalone READ getNonStandalone CONSTANT)
    Q_PROPERTY(QString rendererPath READ getRendererPath CONSTANT)
public:
    explicit AppResourceManager(QObject *parent = 0);
    virtual ~AppResourceManager();

    enum Status {
        CheckingForUpdates = 2 << 0,
        Updating = 2 << 1,
        Starting = 2 << 2,
        Error = 2 << 3,
        Ready = 2 << 4,

        Frontend = 2 << 10,
        Backend = 2 << 11,
        Host = 2 << 12,
        Unknown = 2 << 13,

        CheckingForFrontendUpdates = CheckingForUpdates | Frontend,
        UpdatingFrontend = Updating | Frontend,
        CheckingForBackendUpdates = CheckingForUpdates | Backend,
        UpdatingBackend = Updating | Backend,
        UpdateError = Updating | Error,
        HostOutOfDate = Updating | Host | Error,
        StartingBackend = Starting | Backend,
        BackendError = Backend | Error,
        FrontendResourceError = Frontend | Error,
        OtherError = Unknown | Error,
    };
    Q_ENUM(Status)

    Status getStatus() { return _status; }
    double getProgress() { return _progress; }
    bool getProgressIndeterminate() { return _progressIndeterminate; }
    QString getLocalFilePath();
    void setLocalFilePath(QString localFilePath);

    QString getStandaloneRoot();
    QString getStandaloneSPA();
    QString getNonStandalone();

signals:
    void statusChanged();
    void progressChanged();
    void progressIndeterminateChanged();
    void localFilePathChanged();
    void shouldFocus();

public slots:
    void aboutQt();

private slots:
    void _fsm_checkingForFrontendUpdates_go();
    void _fsm_checkingForFrontendUpdates_gotVersions();

    void _fsm_updatingFrontend_go();
    void _fsm_updatingFrontend_downloadProgress(qint64 received, qint64 total);
    void _fsm_updatingFrontend_stored();

    void _fsm_checkingForBackendUpdates_go();
    void _fsm_checkingForBackendUpdates_gotVersions();

    void _fsm_updatingBackend_go();
    void _fsm_updatingBackend_downloadProgress(qint64 received, qint64 total);
    void _fsm_updatingBackend_stored();

    void _fsm_updateError_go();

    void _fsm_hostOutOfDate_go();

    void _fsm_startingBackend_go();
    void _fsm_startingBackend_extracted(int exitCode, QProcess::ExitStatus status);
    void _fsm_startingBackend_stopped(int exitCode, QProcess::ExitStatus status);
    void _fsm_startingBackend_fakeProgress();
    void _fsm_startingBackend_started();

    void _fsm_backendError_go();

    void _fsm_frontendResourceError_go();

    void _fsm_otherError_go();

    void _fsm_ready_go();

    QString _getAppDataDir();
    QString _getTempDir();
    QString _getRCCPath();
    QString _getLilypondBundlePath();
    QString getRendererPath();
    void receivedMessage( quint32 instanceId, QByteArray message );

private:
    QNetworkAccessManager* _network;
    QSettings* _settings;
    Status _status;
    void _setStatus(Status, double progress);
    /**
     * @brief _setStatus without changing status.
     */
    void _setStatus(Status);
    void _setStatusProgressIndeterminate(Status);
    double _progress;
    bool _progressIndeterminate;
    QString _newFrontendVersion;
    QString _newBackendURL;
    QString _newBackendVersion;
    QProcess *_server;
    int _expectedStartupTime;
};

#endif // APPRESOURCEMANAGER_H
