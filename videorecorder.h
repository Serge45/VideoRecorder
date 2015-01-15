#ifndef VIDEORECORDER_H
#define VIDEORECORDER_H

#include <QtGlobal>
#include <QLocalServer>
#include <QAtomicInt>
#include <QSignalMapper>

#if (QT_VERSION >= 0x050000)
#include <QtWidgets/qmainwindow.h>
#else
#include <QMainWindow>
#endif
#include "ui_videorecorder.h"
#include "cameradevice.h"
#include "VideoRecorderIPCCommand.h"

class QLocalSocket;

class VideoRecorder : public QMainWindow
{
    Q_OBJECT

public:
    VideoRecorder(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~VideoRecorder();

public:
    bool isRemoteWorkable() const;

private slots:
    void onNewSocketConnected();
    void onSocketDisconnected();
    void onSocketReadyRead(QObject *obj);
    void on_actionEstimateFPS_triggered();

private:
    bool initRemoteServer();
    bool processIPCCommand(const VideoRecorderIPCCommand &cmd);
    void updateIPCClientCountUI();

private:
    Ui::VideoRecorderClass ui;
    CameraDevice camera;
    QLocalServer remoteServer;
    bool remoteWorkable;
    QAtomicInt connectedSocketCount;
    QSignalMapper socketSignalMapper;
};

#endif // VIDEORECORDER_H
