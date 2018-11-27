#include "bannerdialog.h"
#include "ui_bannerdialog.h"

BannerDialog::BannerDialog(BannerInfo *info, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BannerDialog)
{
    ui->setupUi(this);

    this->info = info;
    ui->nameEdit->setText(info->name);
    ui->locationEdit->setText(info->location);
}

BannerDialog::~BannerDialog()
{
    delete ui;
}

void BannerDialog::on_buttonBox_accepted()
{
    info->name = ui->nameEdit->text();
    info->location = ui->locationEdit->text();
}
