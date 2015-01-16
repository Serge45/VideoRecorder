#include "cameradevice.h"
#include <QDebug>
#include <QDir>

#ifdef _WIN32
#include <Windows.h>
#include <dshow.h>
#pragma comment(lib, "strmiids")
#endif

qint64 fpsToTimeMs(int fps) {
    return static_cast<qint64>(1000. / fps + 0.01);
}

QList<QString> CameraDevice::deviceList = QList<QString>();

CameraDevice::CameraDevice(QObject *parent, int devIdx)
    : QObject(parent), deviceIdx(devIdx), fps(25.), highPrecisionTimer(this),
    recording(false), playing(false), playingPause(false), capturing(false)
{
    if (deviceList.empty()) {
        listDevices(true);
    }

    highPrecisionTimer.setInterval(fpsToTimeMs(fps));

    if (capturer.open(deviceIdx)) {
        startCapturing();
    }
}

CameraDevice::~CameraDevice() {
    stopCapturing();
}

void CameraDevice::setRecordFilePath(const QString &path) {
    recordFilePath = path;
}

int CameraDevice::listDevices(bool silent) {
#ifdef _WIN32
    return listDevicesWin32(silent);
#elif defined(__APPLE__)
    return listDevicesOSX(silent);
#endif
}

double CameraDevice::estimateFPS() {
    bool capturerState = capturing;

    if (capturing) {
        stopCapturing();
    }
    
    fpsEstimationTimer.restart();

    double ret = 10.;
    
    if (capturer.open(deviceIdx)) {
        int count = 0;
        capturer >> localBuffer;
        auto startTime = fpsEstimationTimer.elapsed();
        
        while (count < 50) {
            capturer >> localBuffer;
            ++count;
        }

        auto endTime = fpsEstimationTimer.elapsed();
        double estimateFPS = count * 1000;
        estimateFPS /= endTime - startTime;
        qDebug() << "fps: " << estimateFPS;
        ret = estimateFPS;
    }

    if (capturerState) {
        startCapturing();
    }

    return ret;
}

int CameraDevice::getPlayVideoTotalFrameCount() {
    if (capturer.isOpened()) {
        return static_cast<int>(capturer.get(::CV_CAP_PROP_FRAME_COUNT));
    }
    return 0;
}

double CameraDevice::getPlayVideoTotalTimeMs() {
    auto totalFrameCount = getPlayVideoTotalFrameCount();
    auto curPos = capturer.get(::CV_CAP_PROP_POS_MSEC);
    capturer.set(::CV_CAP_PROP_POS_FRAMES, totalFrameCount - 1);
    auto totalTimeMs = capturer.get(::CV_CAP_PROP_POS_MSEC);
    capturer.set(::CV_CAP_PROP_POS_FRAMES, curPos);
    return totalTimeMs;
}

//Copied and modified from OpenCV source codes.
int CameraDevice::listDevicesWin32(bool silent) {
#ifdef _WIN32
    //Try COM Library Intialization
    auto comInitOk = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    deviceList.clear();

    if(!silent) {
        printf("\nVIDEOINPUT SPY MODE!\n\n");
    }

    ICreateDevEnum *pDevEnum = NULL;
    IEnumMoniker *pEnum = NULL;
    int deviceCounter = 0;

    HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL,
        CLSCTX_INPROC_SERVER, IID_ICreateDevEnum,
        reinterpret_cast<void**>(&pDevEnum));

    if (SUCCEEDED(hr)) {
        // Create an enumerator for the video capture category.
        hr = pDevEnum->CreateClassEnumerator(
            CLSID_VideoInputDeviceCategory,
            &pEnum, 0);

        if (hr == S_OK) {
            if (!silent) {
                printf("SETUP: Looking For Capture Devices\n");
            }
            IMoniker *pMoniker = NULL;

            while (pEnum->Next(1, &pMoniker, NULL) == S_OK) {
                IPropertyBag *pPropBag;
                hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag,
                    (void**)(&pPropBag));

                if (FAILED(hr)) {
                    pMoniker->Release();
                    continue;// Skip this one, maybe the next one will work.
                }

                // Find the description or friendly name.
                VARIANT varName;
                VariantInit(&varName);
                hr = pPropBag->Read(L"Description", &varName, 0);

                if (FAILED(hr)) {
                    hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                }

                if (SUCCEEDED(hr)) {
                    hr = pPropBag->Read(L"FriendlyName", &varName, 0);
                    const auto maxLen = ::SysStringLen(varName.bstrVal);
                    deviceList.push_back(QString::fromWCharArray(varName.bstrVal, maxLen));

                    if (!silent) {
                        printf("SETUP: %i) %s \n",deviceCounter, deviceList[deviceCounter].toStdString().c_str());
                    }
                }

                pPropBag->Release();
                pPropBag = NULL;

                pMoniker->Release();
                pMoniker = NULL;

                deviceCounter++;
            }

            pDevEnum->Release();
            pDevEnum = NULL;

            pEnum->Release();
            pEnum = NULL;
        }

        if (!silent) {
            printf("SETUP: %i Device(s) found\n\n", deviceCounter);
        }
    }

    if (comInitOk != RPC_E_CHANGED_MODE) {
        CoUninitialize();
    }

    return deviceCounter;
#else
    return -1;
#endif
}

const QList<QString> &CameraDevice::getDeviceList() {
    return deviceList;
}

void CameraDevice::onUpdateTimerTimeout() {
    recorderRemainFrameCount.ref();

    if (playingPause && playing) {
        recorderRemainFrameCount.deref();
        return;
    }

    capturer >> localBuffer;

    if (recording && recorder.isOpened()) {
        qint64 recordElapsedTime = static_cast<qint64>(recorderTimeElapsedTimer.elapsed());
        recorder.write(localBuffer);
        emit recordElapsedTimeUpdated(recordElapsedTime);
    }
    emit imageUpdated(&localBuffer);
    recorderRemainFrameCount.deref();

    if (playing) {
        if (capturer.get(::CV_CAP_PROP_FRAME_COUNT) <= capturer.get(::CV_CAP_PROP_POS_FRAMES) + 1) {
            capturer.set(::CV_CAP_PROP_POS_FRAMES, 0);
        }
        emit playElapsedTimeUpdated(capturer.get(::CV_CAP_PROP_POS_MSEC));
        emit playProgressUpdated(static_cast<int>(capturer.get(::CV_CAP_PROP_POS_FRAMES)));
    }
}

void CameraDevice::startCapturing() {
    if (capturer.isOpened()) {
        capturing = true;
        highPrecisionTimer.setCallback(std::bind(&CameraDevice::onUpdateTimerTimeout, this));
        highPrecisionTimer.start();
    }
}

void CameraDevice::stopCapturing() {
    capturing = false;
    highPrecisionTimer.stop();
    stopRecording();
}

void CameraDevice::startRecording(const QString &dstPath) {
    if (capturer.isOpened()) {
        auto frameWidth = capturer.get(::CV_CAP_PROP_FRAME_WIDTH);
        auto frameHeight = capturer.get(::CV_CAP_PROP_FRAME_HEIGHT);
        auto format =
#ifdef _WIN32
        CV_FOURCC('D', 'I', 'V', 'X');
#elif defined(__APPLE__)
        CV_FOURCC('m', 'p', '4', 'v');
#endif
        recordFilePath = dstPath;

        QDir dir(recordFilePath);

        if (!dir.exists()) {
            dir.mkpath(dir.absolutePath());
        }

        if (recorder.open(recordFilePath.toStdString(), format, fps, cv::Size(frameWidth, frameHeight))) {
            recorderTimeElapsedTimer.restart();
            recording = true;
        }
        emit updateRecordingState(recording);
    }
}

void CameraDevice::stopRecording() {
    recording = false;

    if (recorder.isOpened()) {

        while (recorderRemainFrameCount) {
            //Busy waiting.
        };
        recorder.release();
        emit updateRecordingState(recording);
        emit recordingHasFinished();
    }
}

void CameraDevice::startPlaying(const QString &filePath) {
    if (playing) {
        return;
    }
    stopCapturing();
    recordFilePath = filePath;

    if (capturer.open(recordFilePath.toStdString())) {
        emit videoHasLoaded(getPlayVideoTotalFrameCount());
        emit videoTotalTimeHasGot(getPlayVideoTotalTimeMs());
        playing = true;
        startCapturing();
    }
}

void CameraDevice::stopPlaying() {
    stopCapturing();
    playing = false;
    emit playElapsedTimeUpdated(0.);

    if (capturer.open(deviceIdx)) {
        startCapturing();
    }
}

void CameraDevice::pausePlaying(bool pause) {
    playingPause = pause;
}

void CameraDevice::setCurrentPlayVideoFrame(int frame) {
    if (playing && playingPause && capturer.isOpened()) {
        capturer.set(::CV_CAP_PROP_POS_FRAMES, static_cast<double>(frame));
        capturer >> localBuffer;
        emit imageUpdated(&localBuffer);
        emit playElapsedTimeUpdated(capturer.get(::CV_CAP_PROP_POS_MSEC));
    }
}

void CameraDevice::setFPS(double f) {
    fps = f;
    highPrecisionTimer.setInterval(fpsToTimeMs(fps));
}

void CameraDevice::setCameraIdx(int idx) {
    deviceIdx = idx;

    if (!playing && !recording) {
        stopCapturing();
        startCapturing();
    }
}
