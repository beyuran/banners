#ifndef PROGRAMWINDOWIMAGE_H
#define PROGRAMWINDOWIMAGE_H

#include "programwindow.h"

class ProgramWindowImage : public ProgramWindow
{
    Q_OBJECT
public:
    explicit ProgramWindowImage(QStringList fileNames, QWidget *parent);
private:
    void initReport(){}
    void writeReport(){}
    void readCurrentFrame();
private slots:
    void on_buildReportBtn_clicked(){}
private:
    QStringList imageNames;
};

#endif // PROGRAMWINDOWIMAGE_H

