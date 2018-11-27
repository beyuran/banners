#ifndef PARAMETERSDIALOG_H
#define PARAMETERSDIALOG_H

#include <QDialog>
#include <QStringList>
#include "structures.h"

namespace Ui {
class ParametersDialog;
}

class ParametersDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ParametersDialog(Parameters *params_, QWidget *parent = 0);
    ~ParametersDialog();
    
private slots:
    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_featureTypeSb_currentIndexChanged(int index);

private:
    Ui::ParametersDialog *ui;
    Parameters *params;
};

#endif // PARAMETERSDIALOG_H
