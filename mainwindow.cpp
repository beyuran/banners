#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "programwindowvideo.h"
#include "programwindowimage.h"

#include <QFileDialog>

#define APP_STATE_FILE ".local_state"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    listModel = new QStringListModel(ui->pics_lv);
    readState();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{
    QString selfilter = tr("Video files (*.avi *.mp4)");
    QString path = QFileDialog::getOpenFileName(this, QString("Media files"), appState.workDir, tr("Video files (*.avi *.mp4);;"), &selfilter);
    ui->video_le->setText(path);
    appState.fileNames.clear();
    appState.fileNames.append(ui->video_le->text());
}

void MainWindow::on_openButton_2_clicked()
{
    QString selfilter = tr("Images (*.jpg *.png)");
    appState.fileNames = QFileDialog::getOpenFileNames(this, QString("Media files"), appState.workDir, tr("Images (*.jpg *.png);;"), &selfilter);
    listModel->setStringList(appState.fileNames);
    ui->pics_lv->setModel(listModel);
}

void MainWindow::on_runButton_clicked()
{
    writeState();
    if (ui->video_rb->isChecked())
        pWindow = new ProgramWindowVideo(appState.fileNames, this);
    else
        pWindow = new ProgramWindowImage(appState.fileNames, this);
    pWindow->setAttribute( Qt::WA_DeleteOnClose );
    pWindow->show();
}

void MainWindow::on_video_rb_toggled(bool checked)
{
    ui->video_le->setEnabled(checked);
    ui->openButton->setEnabled(checked);
    ui->pics_lv->setEnabled(!checked);
    ui->openButton_2->setEnabled(!checked);
}

void MainWindow::readState()
{
    QFile file(APP_STATE_FILE);
    bool isOpened = file.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!isOpened)
        return;
    QTextStream in;
    in.setDevice(&file);

    appState.workDir = in.readLine().trimmed();
    appState.isVideo = bool(in.readLine().toInt());
    appState.fileNames = in.readLine().split(",", QString::SkipEmptyParts);
    file.close();

    // Apply state
    if (appState.isVideo)
    {
        if (appState.fileNames.size() > 0)
            ui->video_le->setText(appState.fileNames[0]);
    }
    else
    {
        listModel->setStringList(appState.fileNames);
        ui->pics_lv->setModel(listModel);
    }
    on_video_rb_toggled(appState.isVideo);
    ui->pics_rb->setChecked(!appState.isVideo);
}

void MainWindow::writeState()
{
    QFile file(APP_STATE_FILE);
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out;
    out.setDevice(&file);

    out << appState.workDir << endl;
    out << int(ui->video_rb->isChecked()) << endl;
    for(int i = 0; i < appState.fileNames.size(); i++)
        out << appState.fileNames.at(i) << ",";
    out << endl;

    file.close();
}
