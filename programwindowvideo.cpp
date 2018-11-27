#include "programwindowvideo.h"

ProgramWindowVideo::ProgramWindowVideo(QStringList fileNames, QWidget *parent):
    ProgramWindow(parent),
    videoFileName(fileNames[0])
{
    if(!capture.open(videoFileName.toStdString()))
    {
        qDebug() << "Can't open video " << videoFileName;
        exit(-1);
    }

    frame_count = (int)capture.get(CV_CAP_PROP_FRAME_COUNT);
    printf("Frames count : %d\n", frame_count);
    on_nextButton_clicked();
    preloadFrames(0,120);

    ui->mainFrame->setFixedSize(curr_frame.cols + 2, curr_frame.rows + 23);

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
    resize(curr_frame.cols + 280, curr_frame.rows + 260);

    ui->createBannerButton->setEnabled(false);
    ui->detectButton->setEnabled(false);

    ui->minuteSb->setMinimum(0);
    if (frame_count < 120)
    {
        ui->minuteSb->setMaximum(1);
        ui->minuteSb->setEnabled(false);
    }else
        ui->minuteSb->setMaximum(frame_count / 120);

    ui->checkStartSb->setMaximum(frame_count - 1);
    ui->checkStartSb->setValue(0);
    ui->checkFinishSb->setMaximum(frame_count - 1);
    ui->checkFinishSb->setValue(frame_count - 1);
    ui->gotoFrameSb->setMaximum(frame_count - 1);
}

void ProgramWindowVideo::readCurrentFrame()
{
    capture.set(cv::CAP_PROP_POS_FRAMES, curr_frame_num);
    capture.read(curr_frame);
}

void ProgramWindowVideo::on_buildReportBtn_clicked()
{
    if (in_process)
    {
        break_process = true;
        printf("Soon process will be stopped. Please wait...\n");
        return;
    }
    in_process = true;
    break_process = false;
    ui->buildReportBtn->setText("Stop");
    QCoreApplication::processEvents();

    objectDetector->setMatchingParams(params);
    objectDetector->setDebugMode(ui->debugCheckBox->isChecked());
    printf("Start scanning...\n");
    timer.start();

    hitsNum = 0;
    QFile file(videoFileName+".txt");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    reportStream.setDevice(&file);
    initReport(true);

    int start = ui->checkStartSb->value();
    int finish = ui->checkFinishSb->value();
    cv::Mat frame;

    int b_size = 0;
    std::vector<cv::Mat> selected_images;
    std::vector<BannerInfo> selected_banners;

    for (int i = 0; i < ui->bannersListWidget->count(); i++)
    {
        QListWidgetItem *item = ui->bannersListWidget->item(i);
        if (item->checkState() == Qt::Checked)
        {
            selected_images.push_back(banners[i].image);
            selected_banners.push_back(banners[i]);
            b_size++;
        }
    }
    int size = (finish - start + 1) * b_size;
    int progress = 0;
    int count = 0;

    ui->progressBar->setValue(0);
    capture.set(cv::CAP_PROP_POS_FRAMES, start);
    int frameNum = start;

    if (selected_images.size() > 0)
    {
        objectDetector->computeQueryDescriptors(selected_images);

        while(capture.read(frame) && frameNum <= finish)
        {
            QCoreApplication::processEvents();
            if (frame.empty())
            {
                frameNum++;
                continue;
            }
            objectDetector->computeSceneDescriptors(frame);
            for (int i = 0; i < selected_banners.size(); i++)
            {
                try
                {
                    objectDetector->computeMatches(selected_images[i], frame, i);
                }
                catch(cv::Exception ex)
                {
                    printf("Exception on frame %d, banner = %s\n",frameNum, selected_banners[i].name.toStdString().c_str());
                    break;
                }
                std::vector<MatchInfo> detectorHits = objectDetector->getHits();
                std::vector<MagellanHit> magellanHits = getMagellanHits(banners[i], detectorHits, frameNum);
                writeReport(magellanHits);
                count++;
                progress = lround(100 * count / size);
            }
            if (break_process == true)
            {
                printf("Process is stopped on frame %d.\n", frameNum);
                break;
            }
            frameNum++;
            ui->progressBar->setValue(progress);
        }
    }else
    {
        printf("Banners list is empty.\n");
    }

    if (!break_process)
        printf("Process is succesfully finished.\n");
    printf("Elapsed time %0.3f s\n", timer.elapsed() / 1000.0);
    ui->progressBar->setValue(0);
    file.close();
    in_process = false;
    break_process = false;
    ui->buildReportBtn->setText("Start");
}

void ProgramWindowVideo::initReport(bool writeCoords)
{
    this->writeCoords = writeCoords;
    reportStream << "Program\t" << "Hit\t" << "Pictograph\t" << "Frame\t"
        << "Size\t" << "Visible\t" << "On-screen\t" << "Occlusion\t"
        << "Position\t" << "Quality\t" << "Origin\t" << "Location\t";
    if (writeCoords) reportStream << "Coords";
    reportStream << "\n";
}

void ProgramWindowVideo::writeReport(std::vector<MagellanHit> hits)
{
    for (int i = 0; i < hits.size(); i++)
    {
        reportStream << hits[i].program
            << "\t" << hits[i].hid
            << "\t" << hits[i].bannerInfo.name
            << "\t" << hits[i].frame
            << "\t" << hits[i].matchInfo.size
            << "\t" << hits[i].matchInfo.visible
            << "\t" << hits[i].matchInfo.onscreen
            << "\t" << hits[i].matchInfo.occlusion
            << "\t" << hits[i].matchInfo.position
            << "\t" << hits[i].matchInfo.quality
            << "\t" << hits[i].origin
            << "\t" << hits[i].bannerInfo.location;
        if (writeCoords)
            reportStream << "\t" << hits[i].matchInfo.corners[0].x
                << " " << hits[i].matchInfo.corners[0].y
                << " " << hits[i].matchInfo.corners[1].x
                << " " << hits[i].matchInfo.corners[1].y
                << " " << hits[i].matchInfo.corners[2].x
                << " " << hits[i].matchInfo.corners[2].y
                << " " << hits[i].matchInfo.corners[3].x
                << " " << hits[i].matchInfo.corners[3].y;
        reportStream << "\n";
    }
}

void ProgramWindowVideo::preloadFrames(int frameStart, int frameSize)
{
    ui->framesListWidget->clear();
    capture.set(cv::CAP_PROP_POS_FRAMES, frameStart);
    int frameAdded = frameStart ;
    while(frameAdded < frameStart + frameSize)
    {
        cv::Mat frame;
        capture.read(frame);
        if(frame.empty())
            break;
        cv::Mat icon;
        cv::cvtColor(frame,icon,CV_BGR2RGB);
        QImage qimage((uchar*)icon.data, icon.cols, icon.rows, icon.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(qimage);
        ui->framesListWidget->addItem(new QListWidgetItem(QIcon(pixmap),QString::number(frameAdded)));
        frameAdded++;
    }
    capture.set(cv::CAP_PROP_POS_FRAMES, frameStart);
}

std::vector<MagellanHit> ProgramWindowVideo::getMagellanHits(BannerInfo bannerInfo, std::vector<MatchInfo> matches, int frameNum)
{
    std::vector<MagellanHit> hits;
    for (MatchInfo matchInfo: matches)
    {
        MagellanHit mHit;
        hitsNum++;
        mHit.frame = frameNum;
        mHit.hid = hitsNum;
        mHit.matchInfo = matchInfo;
        mHit.bannerInfo = bannerInfo;
        hits.push_back(mHit);
    }
    return hits;
}

