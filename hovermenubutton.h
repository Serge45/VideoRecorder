//
//  hovermenubutton.h
//  VideoRecorder
//
//  Created by Serge Lu on 2015/1/11.
//
//

#ifndef __VideoRecorder__hovermenubutton__
#define __VideoRecorder__hovermenubutton__

#include <QtGlobal>
#if (QT_VERSION >= 0x050000)
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qmenu.h>
#include <QtWidgets/qabstractbutton.h>
#else
#include <QAbstractButton>
#include <QMenu>
#include <QIcon>
#endif

class QAction;
class QEvent;
class QMoveEvent;
class QPaintEvent;

class HoverMenuButton : public QAbstractButton {
    Q_OBJECT
    
public:
    HoverMenuButton(QWidget *parent = nullptr);

public:
    void addAction(QAction *action);

public slots:
    void showMenu();
    void hideMenu();
    
protected:
    void enterEvent(QEvent *event);
    void leaveEvent(QEvent *event);
    void paintEvent(QPaintEvent *event);
    
private:
    QMenu menu;
    QImage icon;
    QColor backgroundColor;
};

#endif /* defined(__VideoRecorder__hovermenubutton__) */
