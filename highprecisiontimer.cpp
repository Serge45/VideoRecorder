#include "highprecisiontimer.h"
#include <QtConcurrentRun>

#ifdef _WIN32
#include <Windows.h>
#define sleepMs(X) Sleep(X)
#else
#include <unistd.h>
#define sleepMs(X) usleep(1000 * (X))
#endif

HighPrecisionTimer::HighPrecisionTimer(QObject *parent)
    : QObject(parent), intervalMs(100000), running(false),
    referenceTime(0) {
    callback = [](){};
}

HighPrecisionTimer::~HighPrecisionTimer() {
    stop();
}

void HighPrecisionTimer::setInterval(qint64 usec) {
    intervalMs = usec;
}

void HighPrecisionTimer::start() {
    if (!running) {
        running = true;
        timerFuture = QtConcurrent::run(this, &HighPrecisionTimer::timerLoop);
        timerFutureWatcher.setFuture(timerFuture);
    }
}

void HighPrecisionTimer::stop() {
    running = false;
    timerFutureWatcher.waitForFinished();
}

void HighPrecisionTimer::timerLoop() {
    timer.restart();
    referenceTime = timer.elapsed();
    qint64 leftOver = 0;

    while (running) {
        if ((leftOver = (timer.elapsed() - referenceTime)) >= intervalMs) {
            referenceTime = timer.elapsed();
            callback();
            emit timeout();
        } else {
            sleepMs(intervalMs - leftOver - 1);
        }
    }
}

bool HighPrecisionTimer::isActivated() const {
    return running;
}

void HighPrecisionTimer::setCallback(const HighPrecisionTimerCallbackFunctor &func) {
    callback = func;
}

void HighPrecisionTimer::resetCallback() {
    callback = [](){};
}
