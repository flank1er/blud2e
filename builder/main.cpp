#include "mainwindow.h"


int main(int argc, char *argv[])
{
    glutInit(&argc, argv);

    QApplication a(argc, argv);
    MainWindow w;

    if (!(QGLFormat::openGLVersionFlags() & QGLFormat::OpenGL_Version_2_1))
    {
       QMessageBox::critical(0, "Message", "Your platform does not support OpenGL 2.1");
       exit(EXIT_FAILURE);
    }

    w.show();
    return a.exec();

}
