#ifndef CAMERADEVICE_H
#define CAMERADEVICE_H

#include <QObject>
#include <QAtomicInt>
#include <QList>
#include <QString>
#include <QElapsedTimer>
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "highprecisiontimer.h"

class CameraDevice : public QObject
{
    Q_OBJECT

public:
    CameraDevice(QObject *parent, int devIdx = 0);
    ~CameraDevice();

public:
    void setRecordFilePath(const QString &path);
    double estimateFPS();
    int getPlayVideoTotalFrameCount();
    double getPlayVideoTotalTimeMs();

public:
    static int listDevices(bool silent = true);
    static const QList<QString> &getDeviceList();

private:
    static int listDevicesWin32(bool silent);
    static int listDevicesOSX(bool silent);

public slots:
    void startCapturing();
    void stopCapturing();
    void startRecording(const QString &dstPath);
    void stopRecording();
    void startPlaying(const QString &filePath);
    void stopPlaying();
    void pausePlaying(bool pause);
    void setCurrentPlayVideoFrame(int frame);
    void setFPS(double f);
    void setCameraIdx(int idx);

signals:
    void imageUpdated(const cv::Mat *img);
    void updateRecordingState(bool onOff);
    void recordingHasFinished();
    void playElapsedTimeUpdated(double msec);
    void recordElapsedTimeUpdated(double msec);
    void playProgressUpdated(int frameIdx);
    void videoHasLoaded(int totalFrameCount);
    void videoTotalTimeHasGot(double timeMs);

private slots:
    void onUpdateTimerTimeout();

private:
    static QList<QString> deviceList;

private:
    cv::Mat localBuffer;
    cv::VideoCapture capturer;
    cv::VideoWriter recorder;
    int deviceIdx;
    double fps;
    HighPrecisionTimer highPrecisionTimer;
    bool recording;
    bool playing;
    bool playingPause;
    bool capturing;
    QAtomicInt recorderRemainFrameCount;
    QString recordFilePath;
    QString readFilePath;
    QElapsedTimer recorderTimeElapsedTimer;
    QElapsedTimer fpsEstimationTimer;
};

#endif // CAMERADEVICE_H
