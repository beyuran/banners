#include "mouselabel.h"

MouseLabel::MouseLabel(QWidget* parent):
    QLabel(parent)
{

}

void MouseLabel::mousePressEvent(QMouseEvent *ev)
{
    this->x = ev->x();
    this->y = ev->y();
    emit Mouse_Pressed();
}

void MouseLabel::mouseDoubleClickEvent(QMouseEvent *ev)
{
    emit Mouse_DoubleClick();
}

void MouseLabel::leaveEvent(QEvent *ev)
{
    emit Mouse_Left();
}

MouseLabel::~MouseLabel()
{

}

