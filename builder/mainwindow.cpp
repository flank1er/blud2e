#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "aboutdlg.h"
#include "glwidget.h"


inline QString sm(int sectors, int walls, int sprites)
{
    QString ret="<font color=blue>";
    ret +=QString::number(sectors);
    ret +="</font> / <font color=blue>";
    ret +=QString::number(walls);
    ret +="</font> / <font color=blue>";
    ret +=QString::number(sprites);
    ret +="<font>";

    return ret;
}

HexSpinBox::HexSpinBox(QWidget *parent) : QSpinBox(parent)
{
    setRange(0, 255);
    validator = new QRegExpValidator(QRegExp("[0-9A-Fa-f]{1,8}"), this);
}


QValidator::State HexSpinBox::validate(QString &text, int &pos) const
{
    return validator->validate(text, pos);
}

QString HexSpinBox::textFromValue(int value) const
{
    return QString::number(value, 16).toUpper();
}

int HexSpinBox::valueFromText(const QString &text) const
{
    return text.toInt(0, 16);
}


void MY_LABEL::setGrayText(const QString &t, const int &n)
{
    QString myText="<font size=6 color=gray>";
    myText+=t;
    myText+=": </font><font color=green size=6>";
    myText+=QString::number(n);
    myText+="</font>";
    QLabel::setText(myText);
}

void MY_LABEL::setWhiteText(const QString &t, const int &n)
{
    QString myText="<span style='font-size:11pt; color:white;'>";
    myText+=t;
    myText+=": </span><font color=green>";
    myText+=QString::number(n);
    myText+="</font></span>";
    QLabel::setText(myText);
}

void MY_LABEL::setSimpleRedText(const QString &t)
{
    QString myText="<span style='font-size:11pt; color:red;'>";
    myText+=t;
    myText+="</span>";
    QLabel::setText(myText);
}


void MY_LABEL::setSimpleBlueText(const QString &t)
{
    QString myText="<span style='font-size:11pt; color:cyan;'>";
    myText+=t;
    myText+="</span>";
    QLabel::setText(myText);
}

void MY_LABEL::setSimpleGrayText(const QString &t)
{
    QString myText="<span style='font-size:11pt; color:gray;'>";
    myText+=t;
    myText+="</span>";
    QLabel::setText(myText);
}

bool fileExists(const std::string& filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
};


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QLabel* gridLabel=new QLabel(this);
    ui->statusBar->addPermanentWidget(gridLabel);
    gridLabel->setText(gridText(7));

    statLabel =new QLabel(this);
    ui->statusBar->addPermanentWidget(statLabel);
    statLabel->setText(sm(0,0,0));



    this->setWindowTitle("B2E Builder");

    ui->stackedWidget->setMinimumWidth(170);
    ui->stackedWidget->setMaximumWidth(200);
    ui->toolBox->setItemText(0, "Sector");
    ui->toolBox->setItemText(1, "Floor");
    ui->toolBox->setItemText(2, "Ceiling");


    //QVBoxLayout* v1=new QVBoxLayout();
    MY_LABEL* l9=new MY_LABEL();
    l9->setSimpleGrayText("= SECTOR =======");
    ui->verticalLayout_2->addWidget(l9);


    MY_LABEL* l1=new MY_LABEL();
    MY_LABEL* l2=new MY_LABEL();
    MY_LABEL* l3=new MY_LABEL();

    //ui->verticalLayout_2->alignment( );
    QSpacerItem* vsp1 = new QSpacerItem(0, 500, QSizePolicy::Minimum, QSizePolicy::Expanding);
      ui->verticalLayout_2->addItem(vsp1);

    ui->verticalLayout_2->addWidget(l1);
    ui->verticalLayout_2->addWidget(l2);
    ui->verticalLayout_2->addWidget(l3);

    l1->setWhiteText("first wall", 0);
    l2->setWhiteText("number of wall", 0);
    l3->setWhiteText("first sprite", 0);

    QHBoxLayout* h1=new QHBoxLayout();
    MY_LABEL* l4=new MY_LABEL();
    QSpinBox* sb1=new QSpinBox();
    QSpacerItem* hsp1 = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    sb1->setMinimum(0);
    sb1->setMaximum(65535);

    h1->addWidget(l4);
    h1->addWidget(sb1);
    h1->addItem(hsp1);
    ui->verticalLayout_2->addLayout(h1);
    l4->setSimpleRedText("lotag: ");
    sb1->setMinimumWidth(70);

    QHBoxLayout* h2=new QHBoxLayout();
    MY_LABEL* l5=new MY_LABEL();
    QSpinBox* sb2=new QSpinBox();
    QSpacerItem* hsp2 = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    sb2->setMinimum(0);
    sb2->setMaximum(65535);
    h2->addWidget(l5);
    h2->addWidget(sb2);
    h2->addItem(hsp2);
    ui->verticalLayout_2->addLayout(h2);
    l5->setSimpleBlueText("hitag: ");
    sb2->setMinimumWidth(70);

    QHBoxLayout* h3=new QHBoxLayout();
    MY_LABEL* l6=new MY_LABEL();
    QSpinBox* sb3=new QSpinBox();
    QSpacerItem* hsp3 = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    sb3->setMinimum(0);
    sb3->setMaximum(65535);
    h3->addWidget(l6);
    h3->addWidget(sb3);
    h3->addItem(hsp3);
    ui->verticalLayout_2->addLayout(h3);
    l6->setSimpleGrayText("extra: ");
    sb3->setMinimumWidth(70);

    MY_LABEL* l7=new MY_LABEL();
    l7->setWhiteText("visability", 0);
    ui->verticalLayout_2->addWidget(l7);

    MY_LABEL* l8=new MY_LABEL();
    l8->setSimpleGrayText("= CEILING =======");
    ui->verticalLayout_2->addWidget(l8);

    QHBoxLayout* h4=new QHBoxLayout();
    MY_LABEL* l10=new MY_LABEL();
    HexSpinBox* sb4=new HexSpinBox();
    QSpacerItem* hsp4 = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    //sb4->setMinimum(0);
    //sb3->setMaximum(65535);
    h4->addWidget(l10);
    h4->addWidget(sb4);
    h4->addItem(hsp4);
    ui->verticalLayout_2->addLayout(h4);
    l10->setSimpleGrayText("flags (hex): ");
    sb4->setMinimumWidth(45);

    connect(ui->widget, SIGNAL(sentStatus(QString)), ui->statusBar, SLOT(showMessage(QString)));
    connect(ui->widget, SIGNAL(gridSize(QString)), gridLabel, SLOT(setText(QString)));
    connect(this, SIGNAL(KeyPressed(QString)), ui->widget, SLOT(KeyHandler(QString)));
    connect(this, SIGNAL(RebuildMap(QString)), ui->widget, SLOT(chng_map(QString)));
    //connect(&ui->widget->map, SIGNAL(mChanged(QString)), statLabel, SLOT(setText(QString)));
    //connect(&ui->widget->map, SIGNAL(mInit(bool)), ui->widget, SLOT(if_changed_map(bool)));

    std::string tex_con_file="defs.con";
    std::string sound_con_file="sounds.con";
    std::string original_sound="sounds_old.con";
    std::string pic_file="pic_table.con";

    if (!fileExists(tex_con_file) || !fileExists(sound_con_file) || !fileExists(original_sound) || !fileExists(pic_file))
    {
        QMessageBox::critical(0, "Error", "missing files: sounds.con or sounds_old.con or defs.con or pic_table.con");
        exit(EXIT_FAILURE);
    };


    if  ((ui->widget->map.sTable.open(original_sound ,sound_con_file, tex_con_file) < 0) ||
        (ui->widget->map.openPicsTable(pic_file, ui->widget->map.targa) < 0 ))
    {
        QMessageBox::critical(0, "Error", "con't open files: sounds.con or sounds_old.con or defs.con or pic_table.con");
        exit(EXIT_FAILURE);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_Ablout_triggered()
{
    aboutDlg mDialog;
    mDialog.exec();
}

void MainWindow::on_action_Quit_triggered()
{
    QMessageBox msgBox(this);
    msgBox.setWindowTitle(tr("Quit"));
    msgBox.setText(tr("<center><font size=18 color=red> Are you sure? </font></center></size>"));
    msgBox.setIconPixmap(QPixmap(":/images/exit.png"));
    msgBox.setWindowIcon(QIcon(":/images/exit.png"));
    QAbstractButton *quitButton = msgBox.addButton(tr("Quit"), QMessageBox::ActionRole);
    msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == quitButton)
    {
        close();
    }

}

void MainWindow::on_action_Import_triggered()
{
    if (!ui->widget->map.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Attention!"));
        msgBox.setText(tr("<font size=18 color=red>Your map not empty! All data will destroyed!</font>"));
        msgBox.setIconPixmap(QPixmap(":/images/exclamation.png"));
        msgBox.setWindowIcon(QIcon(":/images/ex.png"));
        QAbstractButton *openButton = msgBox.addButton(tr("Open"), QMessageBox::ActionRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == openButton)
        {
            ui->widget->map.rm(); // format c:

        } else
        {
            return;
        }
    }

    QString inputFile=QFileDialog::getOpenFileName(this, tr("Import Blood's' MAP"), "", "Maps (*.map)");
    if (inputFile.isEmpty()) return;

    QString referMessage;
    std::string refer;
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Reading map file");
    //msgBox.setMinimumWidth(400); //don't work(

    QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    //layout->setSizeConstraint(QLayout::SetFixedSize);
    //msgBox.setFixedSize(550,300);
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    const char * filemap = inputFile.toStdString().c_str();
    char* fn=(char*)filemap;

    if (ui->widget->map.read(fn, refer) >= 0)
    {

        msgBox.setText(tr("<center><font size=18 color=green> Success! </font></center>"));
        msgBox.setIconPixmap(QPixmap(":/images/ok_button.png"));
        msgBox.setWindowIcon(QIcon(":/images/ok.png"));
        QString stat=QString::number(ui->widget->map.getSectors()); stat+="/";
        stat +=QString::number(ui->widget->map.getWalls()); stat+="/";
        stat +=QString::number(ui->widget->map.getSprites()); stat+="/0";
        ////////////////////////////////
        // conversion area
        ////////////////////////////////
        ui->widget->map.processing(refer, 0.75f);
        referMessage=QString::fromStdString(refer);
        // change window title
        QStringList parts = inputFile.split("/");
        QString lastBit = parts.at(parts.size()-1);
        QString title="B2E Builder - " +lastBit;
        this->setWindowTitle(title);

        QString statusText=sm(ui->widget->map.getSectors(),ui->widget->map.getWalls(), ui->widget->map.getSprites());
        statLabel->setText(statusText);
        emit RebuildMap("all");
    } else
    {
        msgBox.setText(tr("<H1><center><font color=red>Failure!</font></center></H1>"));
        msgBox.setIconPixmap(QPixmap(":/images/no_button.png"));
        msgBox.setWindowIcon(QIcon(":/images/no.png"));
    };
    msgBox.setDetailedText(referMessage);
    msgBox.exec();
}

void MainWindow::on_action_Open_triggered()
{

    if (!ui->widget->map.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Attention!"));
        msgBox.setText(tr("<font size=18 color=red>Your map not empty! All data will destroyed!</font>"));
        msgBox.setIconPixmap(QPixmap(":/images/exclamation.png"));
        msgBox.setWindowIcon(QIcon(":/images/ex.png"));
        QAbstractButton *openButton = msgBox.addButton(tr("Open"), QMessageBox::ActionRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == openButton)
        {
            ui->widget->map.rm(); // format c:

            QString statusText=sm(ui->widget->map.getSectors(),ui->widget->map.getWalls(), ui->widget->map.getSprites());
            statLabel->setText(statusText);
            emit RebuildMap("all");

        } else
        {
            return;
        }
    }

    QString inputFile=QFileDialog::getOpenFileName(this, tr("Open File"), "", "Maps (*.map)");
    if (inputFile.isEmpty()) return;


    // change window title
    QStringList parts = inputFile.split("/");
    QString lastBit = parts.at(parts.size()-1);
    QString title="B2E Builder - " +lastBit;
    this->setWindowTitle(title);

    QString referMessage;
    std::string refer;
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Reading map file");
    //msgBox.setMinimumWidth(400); //don't work(

    QSpacerItem* horizontalSpacer = new QSpacerItem(500, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    //layout->setSizeConstraint(QGridLayout::SetFixedSize);
    //msgBox.setFixedSize(550,300);
    layout->addItem(horizontalSpacer, layout->rowCount(), 0, 1, layout->columnCount());

    const char * filemap = inputFile.toStdString().c_str();

    char* fn=(char*)filemap;

    if (ui->widget->map.read(fn, refer) >= 0)
    {
        msgBox.setText(tr("<center><font size=18 color=green> Success! </font></center>"));
        msgBox.setIconPixmap(QPixmap(":/images/ok_button.png"));
        msgBox.setWindowIcon(QIcon(":/images/ok.png"));
        QString stat=QString::number(ui->widget->map.getSectors()); stat+="/";
        stat +=QString::number(ui->widget->map.getWalls()); stat+="/";
        stat +=QString::number(ui->widget->map.getSprites()); stat+="/0";

        // change window title
        QStringList parts = inputFile.split("/");
        QString lastBit = parts.at(parts.size()-1);
        QString title="B2E Builder - " +lastBit;
        this->setWindowTitle(title);

        ui->widget->map.prepare();
        referMessage=QString::fromStdString(refer);

        QString statusText=sm(ui->widget->map.getSectors(),ui->widget->map.getWalls(), ui->widget->map.getSprites());
        statLabel->setText(statusText);
        emit RebuildMap("all");

        //statLabel->setText(stat);
    } else
    {
        msgBox.setText(tr("<H1><center><font color=red>Failure!</font></center></H1>"));
        msgBox.setIconPixmap(QPixmap(":/images/no_button.png"));
        msgBox.setWindowIcon(QIcon(":/images/no.png"));
    };
    msgBox.setDetailedText(referMessage);
    msgBox.exec();

}


void MainWindow::on_action_Info_triggered()
{

}

void MainWindow::on_action_Save_triggered()
{
    QString saveFileName;

    if (!ui->widget->map.isEmpty())
    {
        saveFileName = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Maps (*.map)"));
        //qDebug() << saveFileName;
        if (saveFileName.isEmpty())
             return;
    } else
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(tr("<font size=18>You can't' save an empty file!</font>"));
        msgBox.setIconPixmap(QPixmap(":/images/exclamation.png"));
        msgBox.setWindowIcon(QIcon(":/images/ex.png"));
        msgBox.exec();
        return;
    }

      QMessageBox msgBox(this);

      const char * filemap = saveFileName.toStdString().c_str();
      char* fn=(char*)filemap;

      if (ui->widget->map.write(fn) >= 0)
      {
          // change window title
          QStringList parts = saveFileName.split("/");
          QString lastBit = parts.at(parts.size()-1);
          QString title="B2E Builder - " +lastBit;
          this->setWindowTitle(title);

          msgBox.setText(tr("<center><font size=18 color=green> Success! </font></center>"));
          msgBox.setIconPixmap(QPixmap(":/images/ok_button.png"));
          msgBox.setWindowIcon(QIcon(":/images/ok.png"));

      } else
      {
          msgBox.setText(tr("<center><font color=red size=18>Failure!</font></center>"));
          msgBox.setIconPixmap(QPixmap(":/images/no_button.png"));
          msgBox.setWindowIcon(QIcon(":/images/no.png"));
      };

      msgBox.exec();
}

void MainWindow::on_action_New_triggered()
{
    if (!ui->widget->map.isEmpty())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Attention!"));
        msgBox.setText(tr("<font size=18 color=red>Your map not empty! All data will destroyed!</font>"));
        msgBox.setIconPixmap(QPixmap(":/images/exclamation.png"));
        msgBox.setWindowIcon(QIcon(":/images/ex.png"));
        QAbstractButton *newButton = msgBox.addButton(tr("New"), QMessageBox::ActionRole);
        msgBox.addButton(tr("Cancel"), QMessageBox::ActionRole);

        msgBox.exec();

        if (msgBox.clickedButton() == newButton)
        {
            ui->widget->map.rm(); // format c:
            this->setWindowTitle("B2E Builder"); // clear window title

            QString statusText=sm(ui->widget->map.getSectors(),ui->widget->map.getWalls(), ui->widget->map.getSprites());
            statLabel->setText(statusText);
            emit RebuildMap("all");
        }
    }

}


void MainWindow::on_actionHide_ToolBar_triggered(bool checked)
{
    if (!checked)
    {
        ui->mainToolBar->setVisible(true);
        ui->actionHide_ToolBar->setIcon(QIcon(":/images/ok.png"));

    } else
    {
        ui->mainToolBar->setVisible(false);
        ui->actionHide_ToolBar->setIcon(QIcon(":/images/no.png"));
    };

}

void MainWindow::on_actionOpenGL_info_triggered()
{
    QString msg="GL_VERSION: ";
    msg +=QString::fromUtf8((const char*)glGetString(GL_VERSION));
    msg +="\nGL_VENDOR: ";
    msg +=QString::fromUtf8((const char*)glGetString(GL_VENDOR));
    msg +="\nGL_RENDERER: ";
    msg +=QString::fromUtf8((const char*)glGetString(GL_RENDERER));
    msg +="\nGLSL_VERSION : ";
    msg +=QString::fromUtf8((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    QString msgExt=msg;msgExt +="\nGL_EXTENSIONS: ";
    msgExt +=QString::fromUtf8((const char*)glGetString(GL_EXTENSIONS));

    QMessageBox msgBox(this);
    msgBox.setWindowTitle("OpenGL Info");
    msgBox.setIconPixmap(QPixmap(":/images/OpenGL.png"));

    QGridLayout* layout = (QGridLayout*)msgBox.layout();
    layout->setSizeConstraint(QLayout::SetFixedSize);
    msgBox.setFixedSize(550,300);


    msgBox.setText(msg);
    msgBox.setDetailedText(msgExt);
    msgBox.exec();

}

void MainWindow::on_actionAbout_Qt_triggered()
{
    QMessageBox::aboutQt(0,"AboutQt");
}

void MainWindow::on_actionDock_panel_triggered()
{

}

void MainWindow::on_actionDock_panel_triggered(bool checked)
{

    if (!checked)
    {
        ui->stackedWidget->setVisible(true);
        ui->actionDock_panel->setIcon(QIcon(":/images/ok.png"));

    } else
    {
        ui->stackedWidget->setVisible(false);
        ui->actionDock_panel->setIcon(QIcon(":/images/no.png"));
    };

}

void MainWindow::on_action_Export_triggered()
{
    QString saveFileName;

    if (!ui->widget->map.isEmpty())
    {
        saveFileName = QFileDialog::getSaveFileName(this, tr("Export File"), "", tr("Maps (*.obj)"));
        //qDebug() << saveFileName;
        if (saveFileName.isEmpty())
             return;
    } else
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Error"));
        msgBox.setText(tr("<font size=18>You can't' export an empty file!</font>"));
        msgBox.setIconPixmap(QPixmap(":/images/exclamation.png"));
        msgBox.setWindowIcon(QIcon(":/images/ex.png"));
        msgBox.exec();
        return;
    }

      QMessageBox msgBox(this);
      QString refer;

      const char * filemap = saveFileName.toStdString().c_str();
      char* fn=(char*)filemap;

      if (ui->widget->map.extract(150, fn) >= 0)
      {
          msgBox.setText(tr("<center><font size=18 color=green> Success! </font></center>"));
          msgBox.setIconPixmap(QPixmap(":/images/ok_button.png"));
          msgBox.setWindowIcon(QIcon(":/images/ok.png"));

      } else
      {
          msgBox.setText(tr("<center><font color=red size=18>Failure!</font></center>"));
          msgBox.setIconPixmap(QPixmap(":/images/no_button.png"));
          msgBox.setWindowIcon(QIcon(":/images/no.png"));
      };

      msgBox.exec();
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_A:
            emit KeyPressed("zoom_plus");
            break;
        case Qt::Key_Z:
            emit KeyPressed("zoom_minus");
            break;
        case Qt::Key_4:
            emit KeyPressed("left");
            break;
        case Qt::Key_6:
            emit KeyPressed("right");
            break;
        case Qt::Key_8:
            emit KeyPressed("up");
            break;
        case Qt::Key_2:
            emit KeyPressed("down");
            break;
        case Qt::Key_Escape:
            on_action_Quit_triggered();
            break;
        case Qt::Key_G:
            if (event->modifiers() && Qt::AltModifier)
                emit KeyPressed("grid_increment");
            else if (event->modifiers() && Qt::ControlModifier)
                emit KeyPressed("grid_decrement");
            break;

        default:
            QWidget::keyPressEvent(event);
    }

}
