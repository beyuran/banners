#ifndef BANNERDIALOG_H
#define BANNERDIALOG_H

#include <QDialog>
#include "structures.h"

namespace Ui {
class BannerDialog;
}

class BannerDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit BannerDialog(BannerInfo *info, QWidget *parent = 0);
    ~BannerDialog();
    
private slots:
    void on_buttonBox_accepted();

private:
    Ui::BannerDialog *ui;
    BannerInfo *info;
};

#endif // BANNERDIALOG_H
