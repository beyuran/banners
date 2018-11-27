#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include "programwindow.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    QString getWorkDir(){ return appState.workDir;}
    ~MainWindow();
    
private slots:
    void on_openButton_clicked();

    void on_runButton_clicked();

    void on_openButton_2_clicked();

    void on_video_rb_toggled(bool checked);

private:
    ProgramWindow *pWindow;
    Ui::MainWindow *ui;
    QStringListModel *listModel;
    AppState appState;

    void readState();
    void writeState();
};

#endif // MAINWINDOW_H
