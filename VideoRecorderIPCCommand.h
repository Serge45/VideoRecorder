#ifndef VIDEO_RECORDER_IPC_COMMAND_H_
#define VIDEO_RECORDER_IPC_COMMAND_H_

#include <stdint.h>
#include <QString>
#include <QDataStream>

class VideoRecorderIPCCommand
{
    friend QDataStream &operator<<(QDataStream &out, const VideoRecorderIPCCommand &cmd);
    friend QDataStream &operator>>(QDataStream &in, VideoRecorderIPCCommand &cmd);

public:
    enum CommandType : uint32_t {
        playStart   = 0x00000000,
        playPause   = 0x00000001,
        playStop    = 0x00000010,
        playSeek    = 0x00000100,
        recordStart = 0x00001000,
        recordStop  = 0x00010000,
        closeApp    = 0x00100000,
        awakeApp    = 0x10000000
    };

    VideoRecorderIPCCommand();
    VideoRecorderIPCCommand(CommandType cmdType);

public:
    CommandType commandType() const {
        return command;
    }
    void setPlayRecordFilePath(const QString &path);
    void setPlayStartPos(float pos);
    void setPlayStopPos(float pos);
    void setPlaySeekPos(float pos);
    QString getPlayRecordFilePath() const;
    float getPlayStartPos() const;
    float getPlayStopPos() const;
    float getPlaySeekPos() const;

private:
    CommandType command;
    float playStartPos;
    float playStopPos;
    float playSeekPos;
    QString playRecordFilePath;
};

QDataStream &operator<<(QDataStream &out, const VideoRecorderIPCCommand &cmd);
QDataStream &operator>>(QDataStream &in, VideoRecorderIPCCommand &cmd);
#endif