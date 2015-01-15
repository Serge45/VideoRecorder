#ifndef HIGHPRECISIONTIMER_H
#define HIGHPRECISIONTIMER_H

#include <functional>
#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QElapsedTimer>

typedef std::function<void()> HighPrecisionTimerCallbackFunctor;

class HighPrecisionTimer : public QObject
{
    Q_OBJECT

public:
    HighPrecisionTimer(QObject *parent = nullptr);
    ~HighPrecisionTimer();

public:
    void setInterval(qint64 msec);
    void start();
    void stop();
    bool isActivated() const;
    void setCallback(const HighPrecisionTimerCallbackFunctor &func);
    void resetCallback();

signals:
    void timeout();

private:
    void timerLoop();

private:
    QElapsedTimer timer;
    qint64 intervalMs;
    qint64 referenceTime;
    bool running;
    HighPrecisionTimerCallbackFunctor callback;
    QFuture<void> timerFuture;
    QFutureWatcher<void> timerFutureWatcher;
    
};

#endif // HIGHPRECISIONTIMER_H
