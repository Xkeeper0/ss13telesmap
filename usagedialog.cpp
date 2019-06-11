#include "usagedialog.h"
#include "ui_usagedialog.h"

UsageDialog::UsageDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UsageDialog)
{
    ui->setupUi(this);
}

UsageDialog::~UsageDialog()
{
    delete ui;
}
