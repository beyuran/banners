#include "parametersdialog.h"
#include "ui_parametersdialog.h"
#include <QStandardItemModel>

ParametersDialog::ParametersDialog(Parameters *params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ParametersDialog)
{
    ui->setupUi(this);

    this->params = params;
    ui->minMatchCountSb->setValue(params->minMatchCount);
    ui->nnMatchRatioSb->setValue(params->nnMatchRatio);
    ui->ransacThrsSb->setValue(params->ransacThrs);
    ui->partPolicySb->setValue(params->partPolicy);
    ui->thrsCorrSb->setValue(params->thrsCorrection);
    ui->binPreprocCb->setChecked(params->binPreproc);
    ui->matcherTypeCb->insertItems(0, MatcherTypes);
    ui->matcherTypeCb->setCurrentIndex(params->mTypeIndex);
    ui->featureTypeSb->insertItems(0, FeatureTypes);
    ui->featureTypeSb->setCurrentIndex(params->fTypeIndex);
    ui->scaleSb->setValue(params->scale);

    ui->sift_contrThrsSb->setValue(params->sift_contrastThrs);
    ui->sift_edgeThrsSb->setValue(params->sift_edgeThrs);

    ui->surf_hessThrsSb->setValue(params->surf_hessThrs);

    ui->orb_scaleFactorSb->setValue(params->orb_scaleFactor);
    ui->orb_nLevelsSb->setValue(params->orb_nLevels);
    ui->orb_edgeThrsSb->setValue(params->orb_edgeThrs);
    ui->orb_wtakSb->setValue(params->orb_wtak);
    ui->orb_patchSizeSb->setValue(params->orb_patchSize);

    ui->simple_thrsSb->setValue(params->simple_matchThrs);
}

ParametersDialog::~ParametersDialog()
{
    delete ui;
}

void ParametersDialog::on_buttonBox_accepted()
{
    params->mTypeIndex = ui->matcherTypeCb->currentIndex();
    params->fTypeIndex = ui->featureTypeSb->currentIndex();
    params->minMatchCount = ui->minMatchCountSb->value();
    params->nnMatchRatio = ui->nnMatchRatioSb->value();
    params->ransacThrs = ui->ransacThrsSb->value();
    params->partPolicy = ui->partPolicySb->value();
    params->thrsCorrection = ui->thrsCorrSb->value();
    params->binPreproc = ui->binPreprocCb->isChecked();
    params->scale = ui->scaleSb->value();

    params->sift_contrastThrs = ui->sift_contrThrsSb->value();
    params->sift_edgeThrs = ui->sift_edgeThrsSb->value();

    params->surf_hessThrs = ui->surf_hessThrsSb->value();

    params->orb_scaleFactor = ui->orb_scaleFactorSb->value();
    params->orb_nLevels = ui->orb_nLevelsSb->value();
    params->orb_edgeThrs = ui->orb_edgeThrsSb->value();
    params->orb_wtak = ui->orb_wtakSb->value();
    params->orb_patchSize = ui->orb_patchSizeSb->value();

    params->simple_matchThrs = ui->simple_thrsSb->value();
}

void ParametersDialog::on_buttonBox_rejected()
{

}

void ParametersDialog::on_featureTypeSb_currentIndexChanged(int index)
{
    ui->sift_contrThrsSb->setVisible(index == 0);
    ui->sift_edgeThrsSb->setVisible(index == 0);
    ui->sift_contrThrsLb->setVisible(index == 0);
    ui->sift_edgeThrsLb->setVisible(index == 0);

    ui->surf_hessThrsSb->setVisible(index == 1);
    ui->surf_hessThrsLb->setVisible(index == 1);

    ui->orb_scaleFactorSb->setVisible(index == 2);
    ui->orb_scaleFactorLb->setVisible(index == 2);
    ui->orb_nLevelsSb->setVisible(index == 2);
    ui->orb_edgeThrsSb->setVisible(index == 2);
    ui->orb_wtakSb->setVisible(index == 2);
    ui->orb_patchSizeSb->setVisible(index == 2);
    ui->orb_nLevelsLb->setVisible(index == 2);
    ui->orb_edgeThrsLb->setVisible(index == 2);
    ui->orb_wtakLb->setVisible(index == 2);
    ui->orb_patchSizeLb->setVisible(index == 2);

    ui->simple_thrsLb->setVisible(index == 3);
    ui->simple_thrsSb->setVisible(index == 3);

    if (index == 0 || index == 1)
        ui->matcherTypeCb->setCurrentIndex(0);
    else
        ui->matcherTypeCb->setCurrentIndex(3);
    qobject_cast< QStandardItemModel * >( ui->matcherTypeCb->model() )->item(0)->setEnabled(index == 0 || index == 1);
    qobject_cast< QStandardItemModel * >( ui->matcherTypeCb->model() )->item(3)->setEnabled(index == 2);
    qobject_cast< QStandardItemModel * >( ui->matcherTypeCb->model() )->item(4)->setEnabled(index == 2);
}
