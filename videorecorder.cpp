#include "videorecorder.h"
#include <QLocalSocket>
#include <QDataStream>
#include <QtConcurrentRun>
#include "opencv2/core/core.hpp"
#include "cameradevice.h"
#include "hovermenubutton.h"
#include "VideoRecorderGlobalDef.h"

VideoRecorder::VideoRecorder(QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), camera(this, 0), remoteServer(this),
    remoteWorkable(false), socketSignalMapper(this) {

    if (!initRemoteServer()) {
        return;
    }
    ui.setupUi(this);
    ui.imageView->deviceComboBox()->addItems(CameraDevice::getDeviceList());
    ui.imageView->settingButton()->addAction(ui.actionEstimateFPS);
    ui.imageView->settingButton()->addAction(ui.actionIPCClientCount);

    connect(&socketSignalMapper, SIGNAL(mapped(QObject *)),
            this, SLOT(onSocketReadyRead(QObject *))
            );

    connect(&camera, SIGNAL(imageUpdated(const cv::Mat *)),
            ui.imageView, SLOT(updateImage(const cv::Mat *))
            );

    connect(ui.imageView->controlPanel(), SIGNAL(startRecording(const QString &)),
            &camera, SLOT(startRecording(const QString &))
            );

    connect(ui.imageView->controlPanel(), SIGNAL(stopRecording()),
            &camera, SLOT(stopRecording())
            );

    connect(&camera, SIGNAL(updateRecordingState(bool)),
            ui.imageView->controlPanel()->recordButton(), 
            SLOT(setChecked(bool))
            );

    connect(&camera, SIGNAL(updateRecordingState(bool)),
            ui.imageView->deviceComboBox(), 
            SLOT(setDisabled(bool))
            );

    connect(ui.imageView->controlPanel(), SIGNAL(startPlaying(const QString &)),
            &camera, SLOT(startPlaying(const QString &))
            );

    connect(ui.imageView->controlPanel(), SIGNAL(pausePlaying(bool)),
            &camera, SLOT(pausePlaying(bool))
            );

    connect(ui.imageView->controlPanel(), SIGNAL(stopPlaying()),
            &camera, SLOT(stopPlaying())
            );

    connect(&camera, SIGNAL(recordingHasFinished()),
            ui.imageView->controlPanel(), SLOT(onCameraRecordingHasFinished())
            );

    connect(&camera, SIGNAL(playElapsedTimeUpdated(double)),
            ui.imageView->controlPanel(), SLOT(onPlayElapseTimeUpdated(double))
            );

    connect(&camera, SIGNAL(recordElapsedTimeUpdated(double)),
            ui.imageView->controlPanel(), SLOT(onPlayElapseTimeUpdated(double))
            );

    connect(&camera, SIGNAL(playProgressUpdated(int)), 
            ui.imageView->controlPanel()->playProgressBar(),
            SLOT(setValue(int))
            );

    connect(&camera, SIGNAL(videoHasLoaded(int)), 
            ui.imageView->controlPanel(),
            SLOT(onCameraVideoHasLoaded(int))
            );

    connect(ui.imageView->controlPanel()->playProgressBar(),
            SIGNAL(valueChanged(int)),
            &camera,
            SLOT(setCurrentPlayVideoFrame(int))
            );
        
    connect(&camera, SIGNAL(videoTotalTimeHasGot(double)),
            ui.imageView->controlPanel(),
            SLOT(onCameraVideoTimeHasGot(double))
            );

    connect(ui.imageView->deviceComboBox(), SIGNAL(currentIndexChanged(int)),
            &camera, SLOT(setCameraIdx(int))
            );

    connect(ui.imageView, SIGNAL(imageUpdated(const QImage &)),
            this, SLOT(onImageViewUpdated(const QImage &))
            );
}

VideoRecorder::~VideoRecorder() {
    if (remoteServer.isListening()) {
        remoteServer.close();
    }
}

bool VideoRecorder::isRemoteWorkable() const {
    return remoteWorkable;
}

void VideoRecorder::onNewSocketConnected() {
    auto *newConnection = remoteServer.nextPendingConnection();

    if (newConnection) {

        connect(newConnection, SIGNAL(disconnected()),
                newConnection, SLOT(deleteLater())
                );

        connect(newConnection, SIGNAL(disconnected()),
                this, SLOT(onSocketDisconnected())
                );

        connect(newConnection, SIGNAL(readyRead()),
                &socketSignalMapper, SLOT(map())
                );

        socketSignalMapper.setMapping(newConnection, newConnection);
        connectedSocketCount.ref();
        updateIPCClientCountUI();
    }
}

void VideoRecorder::onSocketDisconnected() {
    connectedSocketCount.deref();
    updateIPCClientCountUI();
}

void VideoRecorder::onSocketReadyRead(QObject *obj) {
    auto *socket = qobject_cast<QLocalSocket *>(obj);

    if (socket) {
        VideoRecorderIPCCommand cmd;
        QDataStream stream(socket);
        stream >> cmd;
        processIPCCommand(cmd);
    }
}

void VideoRecorder::on_actionEstimateFPS_triggered() {
    QtConcurrent::run(&camera, &CameraDevice::estimateFPS);
    //camera.estimateFPS();
}

void VideoRecorder::onImageViewUpdated(const QImage &img) {

}

bool VideoRecorder::initRemoteServer() {
    if (remoteWorkable = remoteServer.listen(QString(videoRecorderRemoteServerName))) {

        connect(&remoteServer, SIGNAL(newConnection()),
                this, SLOT(onNewSocketConnected())
                );
    }
    return remoteWorkable;
}

bool VideoRecorder::processIPCCommand(const VideoRecorderIPCCommand &cmd) {
    bool ret = true;

    ui.imageView->controlPanel()->show();

    switch (cmd.commandType()) {

        case VideoRecorderIPCCommand::playStart: {
            auto filePath = cmd.getPlayRecordFilePath();
            ui.imageView->controlPanel()->setRecordPlayFilePath(filePath);

            if (ui.imageView->controlPanel()->playButton()->isEnabled()) {
                ui.imageView->controlPanel()->playButton()->setChecked(true);
            }
            break;
        }

        case VideoRecorderIPCCommand::playStop: {
            if (ui.imageView->controlPanel()->stopButton()->isEnabled()) {
                ui.imageView->controlPanel()->stopButton()->click();
            }
            break;
        }

        case VideoRecorderIPCCommand::playPause: {
            if (ui.imageView->controlPanel()->playButton()->isEnabled()) {
                ui.imageView->controlPanel()->playButton()->setChecked(false);
            }
            break;
        }

        case VideoRecorderIPCCommand::playSeek: {
            auto seekPercent = cmd.getPlaySeekPos();
            auto totalFrameCount = camera.getPlayVideoTotalFrameCount();
            auto targetFramePos = static_cast<int>(seekPercent * totalFrameCount);
            ui.imageView->controlPanel()->playProgressBar()->setSliderDown(true);
            ui.imageView->controlPanel()->playProgressBar()->setValue(targetFramePos);
            ui.imageView->controlPanel()->playProgressBar()->setSliderDown(false);
            break;
        }

        case VideoRecorderIPCCommand::recordStart: {
            ui.imageView->controlPanel()->show();
            auto filePath = cmd.getPlayRecordFilePath();
            ui.imageView->controlPanel()->setRecordPlayFilePath(filePath);

            if (ui.imageView->controlPanel()->recordButton()->isEnabled()) {
                ui.imageView->controlPanel()->recordButton()->setChecked(true);
            }
            break;
        }

        case VideoRecorderIPCCommand::recordStop: {
            ui.imageView->controlPanel()->show();

            if (ui.imageView->controlPanel()->recordButton()->isEnabled()) {
                ui.imageView->controlPanel()->recordButton()->setChecked(false);
            }
            break;
        }

        case VideoRecorderIPCCommand::awakeApp: {
            activateWindow();
            showNormal();
            break;
        }

        case VideoRecorderIPCCommand::closeApp: {
            remoteServer.close();
            close();
        }

        default: {
            ret = false;
            break;
        }
    }
    return ret;
}

void VideoRecorder::updateIPCClientCountUI() {
    QString text = QString("IPC Client: %1").arg(connectedSocketCount);
    ui.actionIPCClientCount->setText(text);
}