#include "aboutdlg.h"
#include "ui_aboutdlg.h"

aboutDlg::aboutDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::aboutDlg)
{
    ui->setupUi(this);
}

aboutDlg::~aboutDlg()
{
    delete ui;
}
