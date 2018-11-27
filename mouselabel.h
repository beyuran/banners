#ifndef MOUSELABEL_H
#define MOUSELABEL_H

#include <QObject>
#include <QLabel>
#include <QMouseEvent>
#include <QEvent>
#include <QDebug>

class MouseLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MouseLabel(QWidget *parent = 0);

    void mousePressEvent(QMouseEvent* ev);
    void mouseDoubleClickEvent(QMouseEvent* ev);
    void leaveEvent(QEvent* ev);

    int x,y;
    bool pressed = false;

    ~MouseLabel();

signals:
    void Mouse_Pressed();
    void Mouse_Pos();
    void Mouse_Left();
    void Mouse_DoubleClick();
};

#endif // MOUSELABEL_H
