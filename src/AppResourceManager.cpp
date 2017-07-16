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

#include "AppResourceManager.h"
#include "util.h"

#include <QTimer> // for debugging
#include <QNetworkReply>
#include <QStandardPaths>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QResource>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QMessageBox>

#include "SingleApplication.h"
#include <QApplication>

#if defined(Q_OS_MACOS) && defined(Q_PROCESSOR_X86_64)
static QString PLATFORM_SLUG = "-osx";
#elif defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)
static QString PLATFORM_SLUG = "-linux-x86_64";
#elif defined(Q_OS_WIN32) && defined(Q_PROCESSOR_X86_32)
#error This component runs fine on Windows, but I still have to get lys to work.
#else
#error Unsupported platform.
#endif

static double UPDATING_FRONTEND_PROGRESS_MIN = 0.1;
static double UPDATING_FRONTEND_PROGRESS_MAX = 0.25;

static double UPDATING_BACKEND_PROGRESS_MIN = 0.3;
static double UPDATING_BACKEND_PROGRESS_MAX = 0.45;

static double STARTING_BACKEND_PROGRESS_MIN = 0.5;
static double STARTING_BACKEND_PROGRESS_MAX  = 0.95;

static QString FRONTEND_RELEASE_SHA_URL = "http://192.168.0.14:5000/react-app.rcc.sha";
static QString FRONTEND_RELEASE_GZ_URL = "http://192.168.0.14:5000/react-app.rcc.gz";
static QString BACKEND_RELEASES_URL = "https://api.github.com/repos/hacklily/hacklily-lilypond-dist/releases/latest";
static QString FRONTEND_VERSION_QSETTINGS_NAME = "frontend-version";
static QString BACKEND_VERSION_QSETTINGS_NAME = "backend-version";

AppResourceManager::AppResourceManager(QObject *parent)
    : QObject(parent)
    , _network(new QNetworkAccessManager(this))
    , _settings(new QSettings("Nettek", "hacklily-app-resource-manager", this))
    , _status(Unknown)
    , _progress(0.00)
    , _progressIndeterminate(true)
    , _server(NULL)
{
    qDebug() << "AppLocalDataLocation:" << _getAppDataDir();
    qDebug() << "TempDataLocation:" << _getTempDir();
    _fsm_checkingForFrontendUpdates_go();

    SingleApplication *app = qobject_cast<SingleApplication*>(QCoreApplication::instance());
    connect(app, &SingleApplication::receivedMessage, this, &AppResourceManager::receivedMessage);

}

AppResourceManager::~AppResourceManager() {
    if (_server) {
        _server->close();
        _server->terminate();
        _server->kill();
        _server->waitForFinished();
        QProcess stopProc;
        stopProc.start(getRendererPath() + QDir::separator() + "stop.sh");
        stopProc.waitForFinished();
    }
}

QString AppResourceManager::getLocalFilePath() {
    return _settings->value("localFilePath").toString();
}

void AppResourceManager::setLocalFilePath(QString localFilePath) {
    _settings->setValue("localFilePath", localFilePath);
    emit localFilePathChanged();
}

QString AppResourceManager::getStandaloneRoot() {
    QString standaloneRootOverride = QString::fromUtf8(qgetenv("HACKLILY_STANDALONE_ROOT"));
    if (standaloneRootOverride.length()) {
        return standaloneRootOverride;
    }
    return "qrc:/react-app";
}

QString AppResourceManager::getStandaloneSPA() {
    QString standaloneSPAOverride = QString::fromUtf8(qgetenv("HACKLILY_STANDALONE_SPA"));
    if (standaloneSPAOverride.length()) {
        return standaloneSPAOverride;
    }
    return getStandaloneRoot() + "/index.html";
}

QString AppResourceManager::getNonStandalone() {
    QString nonStandaloneOverride = QString::fromUtf8(qgetenv("HACKLILY_NON_STANDALONE"));
    if (nonStandaloneOverride.length()) {
        return nonStandaloneOverride;
    }
    return "https://hacklily.github.io";
}

void AppResourceManager::aboutQt() {
    QMessageBox::aboutQt(NULL);
}

void AppResourceManager::_fsm_checkingForFrontendUpdates_go() {
    Q_ASSERT(_status == Unknown);

    _setStatusProgressIndeterminate(CheckingForFrontendUpdates);
    QNetworkRequest req((QUrl(FRONTEND_RELEASE_SHA_URL)));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = _network->get(req);
    connect(
                reply,
                &QNetworkReply::finished,
                this,
                &AppResourceManager::_fsm_checkingForFrontendUpdates_gotVersions
    );
}

static QByteArray getData(QNetworkReply* reply) {
    if (reply->error()) {
        return "";
    }
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QByteArray result = reply->readAll();
    if (status < 200 || status >= 300) {
        return "";
    }
    return result;
}

void AppResourceManager::_fsm_checkingForFrontendUpdates_gotVersions() {
    Q_ASSERT(_status == CheckingForFrontendUpdates);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "NoFrontendUpdates";
        _fsm_otherError_go();
        return;
    }
    QByteArray result = getData(reply);
    reply->deleteLater();
    if (result.isEmpty()) {
        qDebug() << "Got empty reply while checking for frontend updates. Network error: " << reply->errorString();
        _fsm_updateError_go();
        return;
    }
    QStringList lines = QString::fromUtf8(result).split("\n");
    foreach(QString line, lines) {
        line = line.trimmed();
        QStringList parts = line.split(QRegExp("\\s+"));
        if (parts.size() == 2) {
            QString resource = parts[1];
            QString sha = parts[0];
            if (resource == "react-app.rcc.gz") {
                if (sha == _settings->value(FRONTEND_VERSION_QSETTINGS_NAME) &&
                        !QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "react-app.rcc").isEmpty()) {
                    _fsm_checkingForBackendUpdates_go();
                } else {
                    _newFrontendVersion = sha;
                    _fsm_updatingFrontend_go();
                }
            }
        }
    }
}

void AppResourceManager::_fsm_updatingFrontend_go() {
    Q_ASSERT(_status == CheckingForFrontendUpdates);

    _setStatus(UpdatingFrontend, UPDATING_FRONTEND_PROGRESS_MIN);
    QNetworkRequest req((QUrl(FRONTEND_RELEASE_GZ_URL)));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = _network->get(req);
    connect(
                reply,
                &QNetworkReply::finished,
                this,
                &AppResourceManager::_fsm_updatingFrontend_stored
    );
    connect(
                reply,
                &QNetworkReply::downloadProgress,
                this,
                &AppResourceManager::_fsm_updatingFrontend_downloadProgress
    );
}

void AppResourceManager::_fsm_updatingFrontend_downloadProgress(qint64 received, qint64 total) {
    Q_ASSERT(_status == UpdatingFrontend);
    double progressWidth = UPDATING_FRONTEND_PROGRESS_MAX - UPDATING_FRONTEND_PROGRESS_MIN;
    _setStatus(
        UpdatingFrontend,
        (static_cast<double>(received)) / (static_cast<double>(total)) * progressWidth + UPDATING_FRONTEND_PROGRESS_MIN
    );
}

void AppResourceManager::_fsm_updatingFrontend_stored() {
    Q_ASSERT(_status == UpdatingFrontend);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "NoFrontendUpdate";
        _fsm_otherError_go();
        return;
    }
    QByteArray result = getData(reply);
    reply->deleteLater();
    if (result.isEmpty()) {
        qDebug() << "Got empty reply while updating frontend. Network error: " << reply->errorString();
        _fsm_updateError_go();
        return;
    }

    QByteArray uncompressed = Util::gUncompress(result);
    if (uncompressed.isEmpty()) {
        _fsm_updateError_go();
        return;
    }
    QFile file(_getRCCPath());
    if (!file.open(QFile::WriteOnly)) {
        _fsm_updateError_go();
        return;
    }
    if (file.write(uncompressed) == -1) {
        qDebug() << "ErrorInstallingFrontend";
        _fsm_otherError_go(); // installation is probably broken now
        return;
    }
    file.close();
    _settings->setValue(FRONTEND_VERSION_QSETTINGS_NAME, _newFrontendVersion);
    _fsm_checkingForBackendUpdates_go();
}

void AppResourceManager::_fsm_checkingForBackendUpdates_go() {
    Q_ASSERT(_status == CheckingForFrontendUpdates || _status == UpdatingFrontend);

    _setStatusProgressIndeterminate(CheckingForBackendUpdates);
    QNetworkRequest req((QUrl(BACKEND_RELEASES_URL)));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = _network->get(req);
    connect(
                reply,
                &QNetworkReply::finished,
                this,
                &AppResourceManager::_fsm_checkingForBackendUpdates_gotVersions
    );
}

void AppResourceManager::_fsm_checkingForBackendUpdates_gotVersions() {
    Q_ASSERT(_status == CheckingForBackendUpdates);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "NoBackendUpdates";
        _fsm_otherError_go();
        return;
    }
    QByteArray result = getData(reply);
    reply->deleteLater();
    if (result.isEmpty()) {
        qDebug() << "Got empty reply while checking for backend updates. Network error: " << reply->errorString();
        _fsm_updateError_go();
        return;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(result, &error);
    if (error.error != QJsonParseError::NoError) {
        _fsm_updateError_go();
        return;
    }
    QJsonObject docObj = doc.object();
    QString tag = docObj["tag_name"].toString();
    if (tag == _settings->value(BACKEND_VERSION_QSETTINGS_NAME) &&
            !QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "hacklily-lilypond-dist.tar.gz").isEmpty()) {
        _fsm_startingBackend_go();
    } else {
        _newBackendVersion = tag;
        QJsonArray assets = docObj["assets"].toArray();
        foreach(QJsonValue asset, assets) {
            QString name = asset.toObject()["name"].toString();
            if (name.indexOf(PLATFORM_SLUG) != -1) {
              _newBackendURL = asset.toObject()["browser_download_url"].toString();
              _fsm_updatingBackend_go();
              return;
            }
        }
        _fsm_updateError_go();
    }
}

void AppResourceManager::_fsm_updatingBackend_go() {
    Q_ASSERT(_status == CheckingForBackendUpdates);

    qDebug() << "Updating backend" << _newBackendURL << _newBackendVersion;
    _setStatus(UpdatingBackend, UPDATING_BACKEND_PROGRESS_MIN);
    QNetworkRequest req((QUrl(_newBackendURL)));
    req.setAttribute(QNetworkRequest::FollowRedirectsAttribute, true);
    QNetworkReply* reply = _network->get(req);
    connect(
                reply,
                &QNetworkReply::finished,
                this,
                &AppResourceManager::_fsm_updatingBackend_stored
    );
    connect(
                reply,
                &QNetworkReply::downloadProgress,
                this,
                &AppResourceManager::_fsm_updatingBackend_downloadProgress
    );
    connect(
                reply,
                static_cast<void(QNetworkReply::*)(QNetworkReply::NetworkError)>(&QNetworkReply::error),
                this,
                &AppResourceManager::_fsm_updateError_go
    );
}

void AppResourceManager::_fsm_updatingBackend_downloadProgress(qint64 received, qint64 total) {
    Q_ASSERT(_status == UpdatingBackend);
    double progressWidth = UPDATING_BACKEND_PROGRESS_MAX - UPDATING_BACKEND_PROGRESS_MIN;
    _setStatus(
        UpdatingBackend,
        static_cast<double>(received) / static_cast<double>(total) * progressWidth + UPDATING_BACKEND_PROGRESS_MIN
    );
}

void AppResourceManager::_fsm_updatingBackend_stored() {
    Q_ASSERT(_status == UpdatingBackend);
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) {
        qDebug() << "NoBackendUpdate";
        _fsm_otherError_go();
        return;
    }
    QByteArray result = getData(reply);
    reply->deleteLater();
    if (result.isEmpty()) {
        qDebug() << "Got empty reply while updating backend. Network error: " << reply->errorString();
        _fsm_updateError_go();
        return;
    }

    QFile file(_getLilypondBundlePath());
    if (!file.open(QFile::WriteOnly)) {
        _fsm_updateError_go();
        return;
    }
    if (file.write(result) == -1) {
        qDebug() << "ErrorInstallingBackend";
        _fsm_otherError_go(); // installation is probably broken now
        return;
    }
    file.close();
    _settings->setValue(BACKEND_VERSION_QSETTINGS_NAME, _newBackendVersion);
    _fsm_startingBackend_go();
}

void AppResourceManager::_fsm_updateError_go() {
    Q_ASSERT(_status == CheckingForBackendUpdates || _status == UpdatingBackend || _status == CheckingForFrontendUpdates || _status == UpdatingFrontend);

    _setStatusProgressIndeterminate(UpdateError);

    if (!QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "react-app.rcc").isEmpty() &&
            !QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, "hacklily-lilypond-dist.tar.gz").isEmpty()) {
        _fsm_startingBackend_go();
        return;
    }
}

void AppResourceManager::_fsm_hostOutOfDate_go() {
    Q_ASSERT(_status == CheckingForBackendUpdates || _status == UpdatingBackend || _status == CheckingForFrontendUpdates || _status == UpdatingFrontend);

    _setStatusProgressIndeterminate(HostOutOfDate);
}

void AppResourceManager::_fsm_startingBackend_go() {
    Q_ASSERT(_status == CheckingForBackendUpdates || _status == UpdatingBackend || _status == UpdateError);

    _setStatus(StartingBackend, STARTING_BACKEND_PROGRESS_MIN);
    QProcess* proc = new QProcess(this);
    proc->setWorkingDirectory(_getTempDir());
    proc->start("tar", QStringList() <<  "-xf" << _getLilypondBundlePath());

    // falling back to legacy syntax because finished is an overload
    connect(proc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(_fsm_startingBackend_extracted(int,QProcess::ExitStatus)));
}

void AppResourceManager::_fsm_startingBackend_extracted(int exitCode, QProcess::ExitStatus status) {
    Q_ASSERT(_status == StartingBackend);

    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) {
        qDebug() << "Extraction";
        _fsm_otherError_go();
        return;
    }

    qDebug() << "tar log" << proc->readAllStandardError() << proc->readAllStandardError();
    proc->deleteLater();

    if (status != QProcess::NormalExit || exitCode != 0) {
        qDebug() << "Tar";
        _fsm_otherError_go(); // installation is probably broken now
        return;
    }

    if (QDir().exists(_getAppDataDir() + QDir::separator() + ".lilypond-fonts.cache-2")) {
        Util::copyPath(_getAppDataDir() + QDir::separator() + ".lilypond-fonts.cache-2", getRendererPath() + QDir::separator() + ".lilypond-fonts.cache-2");
        _expectedStartupTime = 1500;
    } else {
        _expectedStartupTime = 12000;
    }

    QProcess* stopProc = new QProcess(this);
    stopProc->start(getRendererPath() + QDir::separator() + "stop.sh");
    connect(stopProc, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(_fsm_startingBackend_stopped(int,QProcess::ExitStatus)));
}

void AppResourceManager::_fsm_startingBackend_stopped(int exitCode, QProcess::ExitStatus status) {
    Q_ASSERT(_status == StartingBackend);

    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) {
        qDebug() << "Stopped!";
        _fsm_otherError_go();
        return;
    }

    proc->deleteLater();
    if (status != QProcess::NormalExit || exitCode != 0) {
        qDebug() << "NotNormalExit";
        _fsm_otherError_go(); // installation is probably broken now
        return;
    }

    _server = new QProcess(this);
    _server->start(getRendererPath() + QDir::separator() + "start.sh");
    connect(_server, SIGNAL(readyReadStandardOutput()), this, SLOT(_fsm_startingBackend_started()));
    QTimer::singleShot(20, this, &AppResourceManager::_fsm_startingBackend_fakeProgress);
}

void AppResourceManager::_fsm_startingBackend_fakeProgress() {
    // Distracts the user so they don't freak out.
    if (_status != StartingBackend) {
        qDebug() << "FakeProgress";
        return;
    }

    double progressWidth = STARTING_BACKEND_PROGRESS_MAX - STARTING_BACKEND_PROGRESS_MIN;
    double increment = progressWidth / (_expectedStartupTime / 20);
    if (_progress + increment > STARTING_BACKEND_PROGRESS_MAX) {
        _setStatusProgressIndeterminate(StartingBackend);
    } else {
        _setStatus(StartingBackend, _progress + increment);
        QTimer::singleShot(20, this, &AppResourceManager::_fsm_startingBackend_fakeProgress);
    }
}

void AppResourceManager::_fsm_startingBackend_started() {
    Q_ASSERT(_status == StartingBackend);

    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) {
        qDebug() << "Started";
        _fsm_otherError_go();
        return;
    }

    QByteArray stdOut = proc->readAllStandardOutput();
    qDebug() << "Server" << stdOut;

    if (stdOut.contains("Listening on port")) {
        Util::copyPath(getRendererPath() + QDir::separator() + ".lilypond-fonts.cache-2", _getAppDataDir() + QDir::separator() + ".lilypond-fonts.cache-2");
        disconnect(_server, SIGNAL(readyReadStandardOutput()), this, SLOT(_fsm_startingBackend_started()));
        _fsm_ready_go();
    }
}

void AppResourceManager::_fsm_backendError_go() {
    _setStatusProgressIndeterminate(BackendError);
}

void AppResourceManager::_fsm_frontendResourceError_go() {
    qDebug() << "ResourceError";
    _fsm_otherError_go();
}

void AppResourceManager::_fsm_otherError_go() {
    _setStatusProgressIndeterminate(OtherError);
}

void AppResourceManager::_fsm_ready_go() {
    QResource::unregisterResource(_getRCCPath());
    QResource::registerResource(_getRCCPath());
    _setStatusProgressIndeterminate(Ready);
}

QString AppResourceManager::_getAppDataDir() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(path);
    return path;
}

QString AppResourceManager::_getTempDir() {
    QString path = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir().mkpath(path);
    return path;
}

QString AppResourceManager::_getRCCPath() {
    return _getAppDataDir() + QDir::separator() + "react-app.rcc";
}

QString AppResourceManager::_getLilypondBundlePath() {
    return _getAppDataDir() + QDir::separator() + "hacklily-lilypond-dist.tar.gz";
}

QString AppResourceManager::getRendererPath() {
    return _getTempDir() + QDir::separator() + "hacklily-lilypond-dist" + PLATFORM_SLUG;
}

void AppResourceManager::receivedMessage(quint32, QByteArray message) {
    qDebug() << "Recevied message" << message;
    if (message == "focus") {
        emit shouldFocus();
    }
}

void AppResourceManager::_setStatus(Status status, double progress) {
    if (status != _status) {
        qDebug() << "Status:" << status;
    }
    _status = status;
    _progress = progress;
    _progressIndeterminate = false;
    emit statusChanged();
    emit progressChanged();
    emit progressIndeterminateChanged();
}

void AppResourceManager::_setStatus(Status status) {
    if (status != _status) {
        qDebug() << "Status:" << status;
    }
    _status = status;
    emit statusChanged();
}

void AppResourceManager::_setStatusProgressIndeterminate(Status status) {
    if (status != _status) {
        qDebug() << "Status:" << status;
    }
    _status = status;
    _progressIndeterminate = true;
    emit statusChanged();
    emit progressIndeterminateChanged();
}
