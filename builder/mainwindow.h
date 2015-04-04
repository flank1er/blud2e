#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>
#include <QSpinBox>
#include <QLabel>
#include <QApplication>
#include <QGLWidget>
#include <QMessageBox>
#include <QPushButton>

#include "QFileDialog"
#include <QDebug>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QStackedWidget>
#include <QStackedLayout>
#include <QToolBox>
#include <QSpinBox>

#include <thread>
#include <cassert>
#include <iostream>
#include <sys/stat.h>

#include <GL/glut.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

signals:
        void RebuildMap(QString data);
        void KeyPressed(QString key);

private slots:
    void on_action_Ablout_triggered();

    void on_action_Quit_triggered();

    void on_action_Import_triggered();

    void on_action_Open_triggered();

    void on_action_Info_triggered();


    void on_action_Save_triggered();

    void on_action_New_triggered();


    void on_actionHide_ToolBar_triggered(bool checked);

    void on_actionOpenGL_info_triggered();

    void on_actionAbout_Qt_triggered();

    void on_actionDock_panel_triggered();

    void on_actionDock_panel_triggered(bool checked);

    void on_action_Export_triggered();

private:
    Ui::MainWindow *ui;
    QLabel *statLabel;
protected:
   void keyPressEvent(QKeyEvent* event) Q_DECL_OVERRIDE;
};

class MY_LABEL : public QLabel
{
    Q_OBJECT
public:
        void setGrayText(const QString&, const int&);
        void setWhiteText(const QString&, const int&);
        void setSimpleRedText(const QString &t);
        void setSimpleBlueText(const QString &t);
        void setSimpleGrayText(const QString &t);
private:
};


class HexSpinBox : public QSpinBox
{
    Q_OBJECT

public:
        HexSpinBox(QWidget *parent = 0);

        // ...
private:
    QRegExpValidator *validator;

protected:
    QValidator::State validate(QString &text, int &pos) const;
    int valueFromText(const QString &text) const;
    QString textFromValue(int value) const;
};


#endif // MAINWINDOW_H


/*

class HexSpinBox : public QSpinBox
{
public:
    HexSpinBox(bool only16Bits, QWidget *parent = 0) : QSpinBox(parent), m_only16Bits(only16Bits)
    {
        setPrefix("0x");
        setDisplayIntegerBase(16);
        if (only16Bits)
            setRange(0, 0xFFFF);
        else
            setRange(INT_MIN, INT_MAX);
    }
    unsigned int hexValue() const
    {
        return u(value());
    }
    void setHexValue(unsigned int value)
    {
        setValue(i(value));
    }
protected:
    QString textFromValue(int value) const
    {
        return QString::number(u(value), 16).toUpper();
    }
    int valueFromText(const QString &text) const
    {
        return i(text.toUInt(0, 16));
    }
    QValidator::State validate(QString &input, int &pos) const
    {
        QString copy(input);
        if (copy.startsWith("0x"))
            copy.remove(0, 2);
        pos -= copy.size() - copy.trimmed().size();
        copy = copy.trimmed();
        if (copy.isEmpty())
            return QValidator::Intermediate;
        input = QString("0x") + copy.toUpper();
        bool okay;
        unsigned int val = copy.toUInt(&okay, 16);
        if (!okay || (m_only16Bits && val > 0xFFFF))
            return QValidator::Invalid;
        return QValidator::Acceptable;
    }

private:
    bool m_only16Bits;
    inline unsigned int u(int i) const
    {
        return *reinterpret_cast<unsigned int *>(&i);
    }
    inline int i(unsigned int u) const
    {
        return *reinterpret_cast<int *>(&u);
    }

};

 *
 */
