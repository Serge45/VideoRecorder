#include "videorecorder.h"

#include <QtGlobal>
#include <QLocalSocket>
#include <QDataStream>
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

    VideoRecorder w;

    w.show();
    return a.exec();
}
