#ifndef IMAGEVIEW_H
#define IMAGEVIEW_H

#include <QtGlobal>
#if (QT_VERSION >= 0x050000)
#include <QtWidgets/qwidget.h>
#else
#include <QWidget>
#endif
#include "ui_imageview.h"
#include "opencv2/core/core.hpp"

class QFocusEvent;
class QPaintEvent;
class ControlPanel;
class HoverMenuButton;

class ImageView : public QWidget
{
    Q_OBJECT

public:
    ImageView(QWidget *parent = 0);
    ~ImageView();

public:
    ControlPanel *controlPanel();
    QComboBox *deviceComboBox();
    HoverMenuButton *settingButton();

public slots:
    void updateImage(const cv::Mat *img);

protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void paintEvent(QPaintEvent *event);
    void focusOutEvent(QFocusEvent *event);

private:
    QRect decideDrawingArea() const;

private:
    Ui::ImageView ui;
    QImage localImage;
};

#endif // IMAGEVIEW_H
