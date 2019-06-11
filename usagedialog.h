#ifndef USAGEDIALOG_H
#define USAGEDIALOG_H

#include <QDialog>

namespace Ui {
class UsageDialog;
}

class UsageDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit UsageDialog(QWidget *parent = 0);
    ~UsageDialog();

private:
    Ui::UsageDialog *ui;
};

#endif // USAGEDIALOG_H
