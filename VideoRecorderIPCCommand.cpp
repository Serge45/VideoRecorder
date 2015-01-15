#include "VideoRecorderIPCCommand.h"

VideoRecorderIPCCommand::VideoRecorderIPCCommand()
    : command(recordStart), playStartPos(0.f), playStopPos(1.f),
    playSeekPos(0.f) {

}

VideoRecorderIPCCommand::VideoRecorderIPCCommand(CommandType cmdType)
    : command(cmdType), playStartPos(0.f), playStopPos(1.f), playSeekPos(0.f) {

}

void VideoRecorderIPCCommand::setPlayRecordFilePath(const QString &path) {
    playRecordFilePath = path;
}

void VideoRecorderIPCCommand::setPlayStartPos(float pos) {
    playStartPos = std::min<float>(std::max<float>(pos, 0.f), 1.f);
}

void VideoRecorderIPCCommand::setPlayStopPos(float pos) {
    playStartPos = std::max<float>(std::min<float>(pos, 1.f), 0);
}

void VideoRecorderIPCCommand::setPlaySeekPos(float pos) {
    playSeekPos = std::max<float>(std::min<float>(pos, 1.f), 0.f);
}

QString VideoRecorderIPCCommand::getPlayRecordFilePath() const {
    return playRecordFilePath;
}

float VideoRecorderIPCCommand::getPlayStartPos() const {
    return playStartPos;
}

float VideoRecorderIPCCommand::getPlayStopPos() const {
    return playStopPos;
}

float VideoRecorderIPCCommand::getPlaySeekPos() const {
    return playSeekPos;
}

QDataStream &operator<<(QDataStream &out, const VideoRecorderIPCCommand &cmd) {
    out << static_cast<uint32_t>(cmd.command)
        << cmd.playSeekPos
        << cmd.playStartPos
        << cmd.playStopPos
        << cmd.playRecordFilePath;
    return out;
}

QDataStream &operator>>(QDataStream &in, VideoRecorderIPCCommand &cmd) {
    uint32_t cmdTypeInt;
    in >> cmdTypeInt;

    cmd.command = static_cast<VideoRecorderIPCCommand::CommandType>(cmdTypeInt);

    in >> cmd.playSeekPos
       >> cmd.playStartPos
       >> cmd.playStopPos
       >> cmd.playRecordFilePath;
    
    return in;
}
