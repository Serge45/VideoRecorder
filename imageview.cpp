#include "imageview.h"
#include <QPainter>
#include <QMouseEvent>
#include "cvmatandqimage.h"
#include "hovermenubutton.h"

ImageView::ImageView(QWidget *parent)
    : QWidget(parent) {
    ui.setupUi(this);
    ui.controlPanel->show();
    installEventFilter(this);

#ifdef _DEBUG
    ui.settingButton->setVisible(true);
#else
    ui.settingButton->setVisible(false);
#endif
}

ImageView::~ImageView() {

}

ControlPanel *ImageView::controlPanel() const {
    return ui.controlPanel;
}

QComboBox *ImageView::deviceComboBox() const {
    return ui.deviceListComboBox;
}

HoverMenuButton *ImageView::settingButton() const {
    return ui.settingButton;
}

const QImage &ImageView::image() const {
    return localImage;
}

void ImageView::updateImage(const cv::Mat *img) {
    if (img->data && img->size().area()) {
        localImage = QtOcv::mat2Image_shared(*img).rgbSwapped();
        update();
        emit imageUpdated(localImage);
    }
}

bool ImageView::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::MouseMove) {
        if (obj->isWidgetType()) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

            if (mouseEvent->pos().y() > (height() - ui.controlPanel->height())) {
                ui.controlPanel->show();
            } else {
                ui.controlPanel->hideWithAnimation();
            }
            return true;
        }
        return QWidget::eventFilter(obj, event);
    } else {
        return QWidget::eventFilter(obj, event);
    }
}

void ImageView::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.fillRect(rect(), Qt::black);
    p.drawImage(rect(), localImage, decideDrawingArea());
}

void ImageView::focusOutEvent(QFocusEvent *event) {
    ui.settingButton->hideMenu();
}

QRect ImageView::decideDrawingArea() const {
    auto imgRatio = static_cast<qreal>(localImage.width()) / localImage.height();
    auto viewRatio = static_cast<qreal>(width()) / height();

    auto result = localImage.rect();

    if (imgRatio > viewRatio) {
        result.setHeight(result.width() / viewRatio);
    } else {
        result.setWidth(result.height() * viewRatio);
    }

    result.moveCenter(localImage.rect().center());
    return result;
}
