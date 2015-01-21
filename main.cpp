#include "videorecorder.h"

#include <QtGlobal>

#ifndef _WIN32
#include <QLocalSocket>
#include <QDataStream>
#endif
#if (QT_VERSION >= 0x050000)
#include <QtGui/qguiapplication.h>
#else
#include <QtGui/QApplication>
#endif
#include "VideoRecorderGlobalDef.h"
#include "VideoRecorderIPCCommand.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

#ifndef _WIN32
    QLocalSocket socket;
    socket.connectToServer(QString(videoRecorderRemoteServerName));

    if (socket.isOpen()) {
        VideoRecorderIPCCommand cmd(VideoRecorderIPCCommand::awakeApp);
        QDataStream data(&socket);
        data << cmd;
        socket.waitForBytesWritten();
        socket.disconnectFromServer();
        return -1;
    }
#else
    auto hr = CreateMutexExW(NULL, L"MP7600VideoRecorderMutex", CREATE_MUTEX_INITIAL_OWNER, DELETE);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        return -1;
    }
#endif

    VideoRecorder w;

    w.show();
    a.exec();

#ifdef _WIN32
    if (hr) {
        ReleaseMutex(hr);
    }
#endif
}
