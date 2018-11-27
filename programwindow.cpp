#include "programwindow.h"
#include "ui_programwindow.h"
#include <QDebug>
#include <QMouseEvent>
#include <QFileDialog>
#include "serializer.h"
#include "parametersdialog.h"
#include "bannerdialog.h"
#include "mainwindow.h"

ProgramWindow::ProgramWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ProgramWindow)
{
    ui->setupUi(this);

    workDir = ((MainWindow*)parent)->getWorkDir();

    params = new Parameters();

    connect(ui->mainFrame->imageLabel, SIGNAL(Mouse_Pressed()),this, SLOT(addPoint()));

    objectDetector = new ObjectDetector();

    curr_min = 0;
    curr_frame_num = -1;
    in_process = false;
    break_process = false;
    initScale = 1.0;
}

ProgramWindow::~ProgramWindow()
{
    delete ui;
}

void ProgramWindow::drawFrame(cv::Mat frame, bool reset)
{
    if (!frame.empty())
    {
        cv::Mat img;
        frame.copyTo(img);
        cv::cvtColor(img,img,CV_BGR2RGB);
        QImage qimg((uchar*)img.data, img.cols, img.rows, img.step, QImage::Format_RGB888);
        if (reset)
            ui->mainFrame->setImage(qimg, initScale);
        else
            ui->mainFrame->setImage(qimg, 0.0);
        ui->mainFrame->show();
    }
    else
    {
        // Show empty frame
    }
    if (reset)
        points.clear();
}

void ProgramWindow::addPoint()
{
    if (points.size() == 0)
        curr_frame.copyTo(edit_frame);

    if (points.size() == 4)
    {
        curr_frame.copyTo(edit_frame);
        points.clear();
    }

    cv::Point p;
    p.x = ui->mainFrame->getPosX();
    p.y = ui->mainFrame->getPosY();
    points.push_back(p);
    cv::circle( edit_frame, p, 1.0, cv::Scalar( 0, 0, 255 ), 2, 8 );
    if (points.size() > 1)
    {
        cv::line(edit_frame, points[points.size()-1], points[points.size()-2], cv::Scalar( 110, 220, 0 ), 1, 8 );
    }
    if (points.size() == 4)
    {
        cv::line(edit_frame, points[points.size()-1], points[0], cv::Scalar( 110, 220, 0 ), 1, 8 );
    }

    drawFrame(edit_frame, false);
    ui->createBannerButton->setEnabled(points.size() == 4);
}

void ProgramWindow::on_createBannerButton_clicked()
{
    if (points.size() >= 4)
    {
        BannerInfo banner;
        banner.image = objectDetector->addQueryImage(curr_frame, points);
        BannerDialog bannerDialog(&banner, this);
        if(bannerDialog.exec() == QDialog::Accepted)
        {
            addBanner(banner);
        }
    }
    drawFrame(curr_frame, true);
}

void ProgramWindow::on_detectButton_clicked()
{
    ui->progressBar->setValue(0);
    timer.start();
    int b_size = 0;
    int progress = 0;
    int count = 0;

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
    objectDetector->setDebugMode(ui->debugCheckBox->isChecked(),
                                     ui->drawingCheckBox->isChecked());
    objectDetector->resetDrawScene(curr_frame);
    // To fit matching process into memory for large frames
    objectDetector->autoDetectScaling(curr_frame);
    objectDetector->setMatchingParams(params);

    if (selected_images.size() > 0)
    {
        objectDetector->computeQueryDescriptors(selected_images);
        objectDetector->computeSceneDescriptors(curr_frame);
        for (int i = 0; i < selected_banners.size(); i++)
        {
            try
            {
                objectDetector->computeMatches(selected_images[i], curr_frame, i);
            }
            catch(cv::Exception ex)
            {
                printf("Exception on banner = %s\n", selected_banners[i].name.toStdString().c_str());
                break;
            }
            count++;
            progress = lround(100 * count / b_size);
            ui->progressBar->setValue(progress);
            QCoreApplication::processEvents();
        }
    }
    drawFrame(objectDetector->getResultScene(), true);
    ui->progressBar->setValue(0);
    printf("Done, elapsed time %d ms\n", timer.elapsed());
}

void ProgramWindow::on_paramsDialogBtn_clicked()
{
    ParametersDialog pDialog(params, this);
    if(pDialog.exec() == QDialog::Accepted)
    {
        //printf("Accepted\n");
    }
}

void ProgramWindow::on_saveButton_clicked()
{
    QString selfilter = tr("Archive file (*.bnr)");
    QString path = QFileDialog::getSaveFileName(this, QString("Archive file"), workDir, tr("Archive file (*.bnr)"), &selfilter);
    if (path.isEmpty())
        return;

    std::vector<BannerInfo> to_save;
    for (int i = 0; i < ui->bannersListWidget->count(); i++)
    {
        QListWidgetItem *item = ui->bannersListWidget->item(i);
        if (item->checkState() == Qt::Checked)
        {
            to_save.push_back(banners[i]);
        }
    }
    Serializer::saveMats(to_save, path.toStdString());
}

void ProgramWindow::on_loadButton_clicked()
{
    QString selfilter = tr("Archive file (*.bnr)");
    QStringList paths = QFileDialog::getOpenFileNames(this, QString("Archive file"), workDir, tr("Archive file (*.bnr);;Image file (*.jpg *.png)"), &selfilter);

    if (paths.isEmpty())
        return;

    for (int i = 0; i < paths.size(); i++)
    {
        QFileInfo finfo(paths[i]);
        if (finfo.suffix().toLower() == "bnr")
        {
            std::vector<BannerInfo> to_load;
            Serializer::loadMats(to_load, paths[i].toStdString());
            for (auto banner : to_load)
            {
                addBanner(banner);
            }
        }else
        {
            BannerInfo banner;
            banner.name =  finfo.completeBaseName();
            banner.image = cv::imread(paths[i].toStdString(), CV_LOAD_IMAGE_COLOR);
            addBanner(banner);
        }
    }
}

void ProgramWindow::addBanner(BannerInfo banner)
{
    banners.push_back(banner);

    cv::Mat bannerRGB;
    cv::cvtColor(banner.image, bannerRGB, CV_BGR2RGB);
    QImage qimage((uchar*)bannerRGB.data, bannerRGB.cols, bannerRGB.rows, bannerRGB.step, QImage::Format_RGB888);
    QPixmap pixmap = QPixmap::fromImage(qimage);
    QListWidgetItem *item = new QListWidgetItem(QIcon(pixmap), banner.name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(Qt::Checked);
    ui->bannersListWidget->addItem(item);
    ui->createBannerButton->setEnabled(false);
    ui->detectButton->setEnabled(true);
}

void ProgramWindow::on_deleteButton_clicked()
{
    std::vector<QListWidgetItem*> remove_icons;
    std::vector<std::vector<BannerInfo>::iterator> remove_images;

    for (int i = 0; i < ui->bannersListWidget->count(); i++)
    {
        QListWidgetItem *item = ui->bannersListWidget->item(i);
        if (item->checkState() == Qt::Checked)
        {
            remove_icons.push_back(item);
            remove_images.push_back(banners.begin() + i);
        }
    }

    for (auto it: remove_icons)
        delete it;

    for (auto it: remove_images)
        banners.erase(it);
}

void ProgramWindow::on_checkAllBox_toggled(bool checked)
{
    for (int i = 0; i < ui->bannersListWidget->count(); i++)
    {
        QListWidgetItem *item = ui->bannersListWidget->item(i);
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}

void ProgramWindow::on_bannersListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    int index = ui->bannersListWidget->row(item);
    BannerDialog bannerDialog(&banners[index], this);
    if(bannerDialog.exec() == QDialog::Accepted)
    {
        item->setText(banners[index].name);
    }
}

void ProgramWindow::on_gotoFrameSb_valueChanged(int arg1)
{
    int curr_min = arg1 /120;
    if (curr_min != ui->minuteSb->value())
        preloadFrames(curr_min * 120, 120);
    ui->minuteSb->setValue(arg1 /120);
    curr_frame_num = arg1 - 1;
    on_nextButton_clicked();
}

void ProgramWindow::on_minuteSb_sliderReleased()
{
    if (curr_min == ui->minuteSb->value())
        return;
    curr_min = ui->minuteSb->value();
    curr_frame_num = 120 * curr_min + ui->framesListWidget->currentRow();
    preloadFrames(120 * curr_min, 120);
    curr_frame_num = curr_frame_num - 1;
    on_nextButton_clicked();
}

void ProgramWindow::on_framesListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    curr_frame_num = ui->framesListWidget->currentIndex().row() + ui->minuteSb->value() * 120;
    readCurrentFrame();
    drawFrame(curr_frame, true);
    ui->timeEdit->setTime(QTime(0,0,0).addMSecs(curr_frame_num * 500));
    ui->nextButton->setEnabled(curr_frame_num < frame_count - 1);
    ui->prevButton->setEnabled(curr_frame_num > 0);
}

void ProgramWindow::on_prevButton_clicked()
{
    curr_frame_num--;
    if (curr_frame_num < 0)
        curr_frame_num = 0;
    readCurrentFrame();
    drawFrame(curr_frame, true);
    ui->timeEdit->setTime(QTime(0,0,0).addMSecs(curr_frame_num * 500));
    ui->minuteSb->setValue(curr_frame_num / 120);
    ui->framesListWidget->setCurrentRow(curr_frame_num - ui->minuteSb->value() * 120);
    on_minuteSb_sliderReleased();
    ui->nextButton->setEnabled(curr_frame_num < frame_count - 1);
    ui->prevButton->setEnabled(curr_frame_num > 0);
}

void ProgramWindow::on_nextButton_clicked()
{
    curr_frame_num++;
    readCurrentFrame();
    drawFrame(curr_frame, true);
    ui->timeEdit->setTime(QTime(0,0,0).addMSecs(curr_frame_num * 500));
    ui->minuteSb->setValue(curr_frame_num / 120);
    ui->framesListWidget->setCurrentRow(curr_frame_num - ui->minuteSb->value() * 120);
    on_minuteSb_sliderReleased();
    ui->nextButton->setEnabled(curr_frame_num < frame_count - 1);
    ui->prevButton->setEnabled(curr_frame_num > 0);
}

void ProgramWindow::preloadFrames(int frameStart, int frameSize)
{
    ui->framesListWidget->clear();

    int temp = curr_frame_num;
    int frameStop = std::min(frameStart + frameSize, frame_count);

    for (curr_frame_num = frameStart; curr_frame_num < frameStop; curr_frame_num++)
    {
        readCurrentFrame();
        if (curr_frame.empty())
            break;

        cv::Mat icon;
        cv::cvtColor(curr_frame,icon,CV_BGR2RGB);
        QImage qimage((uchar*)icon.data, icon.cols, icon.rows, icon.step, QImage::Format_RGB888);
        QPixmap pixmap = QPixmap::fromImage(qimage);
        ui->framesListWidget->addItem(new QListWidgetItem(QIcon(pixmap),QString::number(curr_frame_num)));
    }
    curr_frame_num = temp;
    readCurrentFrame();
}

void ProgramWindow::updateSliders()
{
    ui->timeEdit->setTime(QTime(0,0,0).addMSecs(curr_frame_num * 500));

    ui->minuteSb->blockSignals(true);

    ui->minuteSb->setValue(curr_frame_num / 120);
    ui->framesListWidget->setCurrentRow(curr_frame_num - ui->minuteSb->value() * 120);
    on_minuteSb_sliderReleased();

    ui->minuteSb->blockSignals(false);

    ui->nextButton->setEnabled(curr_frame_num < frame_count - 1);
    ui->prevButton->setEnabled(curr_frame_num > 0);
}

void ProgramWindow::on_buildReportBtn_clicked()
{
}
