//
//  hovermenubutton.cpp
//  VideoRecorder
//
//  Created by Serge Lu on 2015/1/11.
//
//

#include "hovermenubutton.h"
#include <QEvent>
#include <QMoveEvent>
#include <QAction>
#include <QPainter>

HoverMenuButton::HoverMenuButton(QWidget *parent)
: QAbstractButton(parent), menu(this) {
    backgroundColor = QColor(0,0,0,20);
    icon = QImage(":/ICON/Resources/settings.ico");
    menu.setAttribute(Qt::WA_TranslucentBackground);
    menu.setStyleSheet("QMenu {background:transparent; background-color:rgba(0,0,0,40%); color:white;} \
                       QMenu::item:selected {background-color:rgba(180,0,0,80);}");
    menu.hide();
}

void HoverMenuButton::addAction(QAction *action) {
    menu.addAction(action);
}

void HoverMenuButton::showMenu() {
    if (!menu.isVisible()) {
        auto p = pos();
        p += QPoint(0, height() - 1);
        menu.exec(mapToGlobal(p));
    }
}

void HoverMenuButton::hideMenu() {
    menu.hide();
}
 
void HoverMenuButton::enterEvent(QEvent *event) {
    showMenu();
}

void HoverMenuButton::leaveEvent(QEvent *event) {
}

void HoverMenuButton::paintEvent(QPaintEvent *event) {
    QPainter p(this);
    p.fillRect(rect(), backgroundColor);
    p.drawImage(rect(), icon);
}