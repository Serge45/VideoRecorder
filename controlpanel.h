#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QtGlobal>
#if (QT_VERSION >= 0x050000)
#include <QtWidgets/qwidget.h>
#else
#include <QWidget>
#endif
#include <QPropertyAnimation>
#include "ui_controlpanel.h"

#ifdef _WIN32
#include <Windows.h>
#include <ShObjIdl.h>
#endif

class QShowEvent;
class QHideEvent;
class QPaintEvent;
class QSlider;
class QPushButton;

class ControlPanel : public QWidget
{
    Q_OBJECT

public:
    ControlPanel(QWidget *parent = 0);
    ~ControlPanel();

public:
    void hideWithAnimation();
    void showWithAnimation();

    QPushButton *playButton() const;
    QPushButton *recordButton() const;
    QPushButton *stopButton() const;
    QSlider *playProgressBar() const;

public:
    void setRecordPlayFilePath(const QString &path);

signals:
    void startRecording(const QString &);
    void stopRecording();
    void startPlaying(const QString &);
    void stopPlaying();
    void pausePlaying(bool);

protected:
    void paintEvent(QPaintEvent *event);
    void showEvent(QShowEvent *event);
    void enterEvent(QEvent *event);

#ifdef _WIN32
    bool winEvent(MSG *message, long *result);
    void initTaskBar();
    ITaskbarList3 *taskBar;
    THUMBBUTTON taskButtons[3];
#endif

private slots:
    void on_recordButton_toggled(bool checked);
    void on_playButton_toggled(bool checked);
    void on_stopButton_clicked();
    void on_recordFilePathEdit_textChanged(const QString &str);
    void on_playProgressBar_sliderPressed();
    void on_playProgressBar_sliderReleased();
    void onShowHideAnimationFinished();
    void onCameraRecordingHasFinished();
    void onPlayElapseTimeUpdated(double msec);
    void onCameraVideoHasLoaded(int frameCount);
    void onCameraVideoTimeHasGot(double timeMs);

private:
    bool recordFilePathValid(const QString &path);

private:
    Ui::ControlPanel ui;
    QPropertyAnimation showHideAnimation;
};

#endif // CONTROLPANEL_H
