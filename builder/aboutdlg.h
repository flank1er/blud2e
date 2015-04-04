#ifndef ABOUTDLG_H
#define ABOUTDLG_H

#include <QDialog>

namespace Ui {
class aboutDlg;
}

class aboutDlg : public QDialog
{
    Q_OBJECT
    
public:
    explicit aboutDlg(QWidget *parent = 0);
    ~aboutDlg();
    
private:
    Ui::aboutDlg *ui;
};

#endif // ABOUTDLG_H
