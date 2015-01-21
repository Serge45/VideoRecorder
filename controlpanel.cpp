#include "controlpanel.h"
#include <cmath>
#include <QPainter>
#include <QFileInfo>

#if defined(_WIN32)
#pragma comment(lib, "ole32")
#endif

QString msToStandardTimeString(double t) {
    double sec = t / 1000.;
    double secFrac = 0.;

    secFrac = modf(sec, &sec);

    int secInt = static_cast<int>(sec + 0.0001);

    int displayHour = secInt / 3600;
    int displayMin = (secInt % 3600) / 60;
    double displaySec = (secInt % 60) + secFrac;
    return QString("%1:%2:%3").arg(displayHour)
                              .arg(displayMin)
                              .arg(QString::number(displaySec, 'f', 2));
}

ControlPanel::ControlPanel(QWidget *parent)
    : QWidget(parent), showHideAnimation(this, "size") {
    ui.setupUi(this);
    setMouseTracking(true);
    showHideAnimation.setTargetObject(this);
    showHideAnimation.setDuration(300);

    connect(&showHideAnimation, SIGNAL(finished()),
            this, SLOT(onShowHideAnimationFinished())
            );
    ui.playButton->setEnabled(recordFilePathValid(ui.recordFilePathEdit->text()));

}

ControlPanel::~ControlPanel() {

}

void ControlPanel::hideWithAnimation() {
    showHideAnimation.stop();
    showHideAnimation.setStartValue(size());
    showHideAnimation.setEndValue(QSize(0, height()));
    showHideAnimation.setEasingCurve(QEasingCurve::OutQuad);
    showHideAnimation.start();
}

void ControlPanel::showWithAnimation() {
    int targetWidth = width();

    if (parent()->isWidgetType()) {
        targetWidth = static_cast<QWidget *>(parent())->width();
    }
    auto startSize = showHideAnimation.currentValue().toSize();
    showHideAnimation.stop();
    showHideAnimation.setStartValue(startSize);
    showHideAnimation.setEndValue(QSize(targetWidth, height()));
    showHideAnimation.setEasingCurve(QEasingCurve::OutCubic);
    showHideAnimation.start();
}

QPushButton *ControlPanel::playButton() const {
    return ui.playButton;
}

QPushButton *ControlPanel::recordButton() const {
    return ui.recordButton;
}

QPushButton *ControlPanel::stopButton() const {
    return ui.stopButton;
}

QSlider *ControlPanel::playProgressBar() const {
    return ui.playProgressBar;
}

void ControlPanel::setRecordPlayFilePath(const QString &path) {
    ui.recordFilePathEdit->setText(path);
}

void ControlPanel::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.fillRect(rect(), QColor(0, 0, 0, 80));
}

void ControlPanel::showEvent(QShowEvent *event) {
    showWithAnimation();
}

void ControlPanel::enterEvent(QEvent *event) {
    showWithAnimation();
}

#ifdef _WIN32
bool ControlPanel::winEvent(MSG *message, long *result) {
    static UINT taskBarCreated = WM_NULL;

    if (taskBarCreated == WM_NULL) {
        taskBarCreated = RegisterWindowMessageW(L"TaskbarButtonCreated");
    }

    if (message->message == taskBarCreated) {
        initTaskBar();
    }

    return false;
}

void ControlPanel::initTaskBar() {
    auto hr = CoCreateInstance(CLSID_TaskbarList, NULL, ::CLSCTX_INPROC_SERVER, ::IID_ITaskbarList3,
                               (void **)(&taskBar)
                               );

    if (hr == S_OK) {
        hr = taskBar->HrInit();

        if (hr != S_OK) {
            taskBar->Release();
            taskBar = NULL;
            return;
        }
    }

    QString taskButtonStrings[3] = {"Record", "Stop", "Play"};

    UINT IDTB_FIRST = 3000;

    for (int i = 0; i < 3; ++i) {
        wcscpy(taskButtons[i].szTip, taskButtonStrings[i].toStdWString().c_str());

        taskButtons[i].iId = IDTB_FIRST + i;
        taskButtons[i].dwMask = (THUMBBUTTONMASK)(THB_BITMAP | THB_FLAGS | THB_TOOLTIP);
        taskButtons[i].dwFlags = (THUMBBUTTONFLAGS)(THBF_ENABLED);
    }

    //taskBar->ThumbBarAddButtons(winId(), 3, taskButtons);
}
#endif

void ControlPanel::on_recordButton_toggled(bool checked) {
    ui.recordFilePathEdit->setDisabled(checked);

    if (checked) {
        ui.playButton->setEnabled(false);
        ui.stopButton->setEnabled(false);
        emit startRecording(ui.recordFilePathEdit->text());
    } else {
        emit stopRecording();
    }
}

void ControlPanel::on_playButton_toggled(bool checked) {
    ui.recordButton->setDisabled(true);
    ui.recordFilePathEdit->setDisabled(true);

    if (checked) {
        ui.stopButton->setEnabled(true);
        emit startPlaying(ui.recordFilePathEdit->text());
    }

    emit pausePlaying(!checked);
}

void ControlPanel::on_stopButton_clicked() {
    ui.playButton->setChecked(false);
    ui.stopButton->setDisabled(true);
    ui.recordButton->setEnabled(true);
    ui.recordFilePathEdit->setEnabled(true);
    emit stopPlaying();
}

void ControlPanel::on_recordFilePathEdit_textChanged(const QString &str) {
    ui.playButton->setEnabled(recordFilePathValid(str));
}

void ControlPanel::on_playProgressBar_sliderPressed() {
    emit pausePlaying(true);
}

void ControlPanel::on_playProgressBar_sliderReleased() {
    emit pausePlaying(!ui.playButton->isChecked());
}

void ControlPanel::onShowHideAnimationFinished() {
    if (width() < 1) {
        hide();
    }
}

void ControlPanel::onCameraRecordingHasFinished() {
    ui.playButton->setEnabled(recordFilePathValid(ui.recordFilePathEdit->text()));
}

void ControlPanel::onPlayElapseTimeUpdated(double msec) {
    //Prevent blinking.
    if (showHideAnimation.state() == QAbstractAnimation::Running) {
        return;
    }

    ui.timeElapsedLabel->setText(msToStandardTimeString(msec));

}

void ControlPanel::onCameraVideoHasLoaded(int frameCount) {
    ui.playProgressBar->setMaximum(frameCount - 1);
    ui.playProgressBar->setSingleStep(std::max<int>(frameCount / 50, 1));
    ui.playProgressBar->setPageStep(std::max<int>(frameCount / 10, 10));
}

void ControlPanel::onCameraVideoTimeHasGot(double timeMs) {
    ui.totalPlayTimeLabel->setText(msToStandardTimeString(timeMs));
}

bool ControlPanel::recordFilePathValid(const QString &path) {
    QFileInfo fileInfo(path);
    auto suffix = fileInfo.suffix().toUpper();

    return (fileInfo.exists() && 
            fileInfo.isFile() &&
            fileInfo.size() &&
            (suffix == "MPG" || suffix == "AVI")
            );
}
