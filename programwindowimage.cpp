#include "programwindowimage.h"

//#define IM_WIDTH 720
//#define IM_HEIGHT 576
#define IM_WIDTH 1000
#define IM_HEIGHT 650

ProgramWindowImage::ProgramWindowImage(QStringList fileNames, QWidget *parent):
    ProgramWindow(parent),
    imageNames(fileNames)
{
    frame_count = imageNames.size();
    printf("Images count : %d\n", frame_count);

    on_nextButton_clicked();
    preloadFrames(0,120);

    int imWidth = std::min(IM_WIDTH, curr_frame.cols);
    int imHeight = std::min(IM_HEIGHT, curr_frame.rows);
    ui->mainFrame->setFixedSize(imWidth + 2, imHeight + 23);

    ui->bannersListWidget->setViewMode(QListWidget::IconMode);
    ui->bannersListWidget->setIconSize(QSize(200,200));

    ui->framesListWidget->setViewMode(QListWidget::IconMode);
    ui->framesListWidget->setIconSize(QSize(100,80));
    ui->framesListWidget->setFixedHeight(120);
    ui->framesListWidget->setWrapping(false);
    ui->framesListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Right way:
    //setCentralWidget(ui->gridLayoutWidget);
    //adjustSize();
    //but it cause "black widget init" bug, so:
    resize(imWidth + 280, imHeight + 260);

    ui->createBannerButton->setEnabled(false);
    ui->detectButton->setEnabled(false);

    ui->minuteSb->setMinimum(0);
    if (frame_count < 120)
    {
        ui->minuteSb->setMaximum(1);
        ui->minuteSb->setEnabled(false);
    }else
        ui->minuteSb->setMaximum(frame_count / 120);
    ui->timeEdit->setVisible(false);

    ui->checkStartSb->setMaximum(frame_count - 1);
    ui->checkStartSb->setValue(0);
    ui->checkFinishSb->setMaximum(frame_count - 1);
    ui->checkFinishSb->setValue(frame_count - 1);
    ui->gotoFrameSb->setMaximum(frame_count - 1);

    params->partPolicy = 1;
    params->fTypeIndex = 3;
}

void ProgramWindowImage::readCurrentFrame()
{
    if (curr_frame_num < frame_count && curr_frame_num >= 0)
    {
        curr_frame = cv::imread(imageNames[curr_frame_num].toStdString());

        int imWidth = std::min(IM_WIDTH, curr_frame.cols);
        int imHeight = std::min(IM_HEIGHT, curr_frame.rows);
        initScale = double(imWidth) / curr_frame.cols;
    }
    else
        std::cout << "WARNING: assert(0 <= curr_frame_num < frame_count)\n";
}

