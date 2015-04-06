#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QGLWidget>
#include <QString>
#include <QTimer>
#include <QDebug>
#include <QObject>
#include <string>

//#include <GL/glew.h>
#include "GL/freeglut.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <../blud2e/blud2e.h>

class B2E : public blud2e
{
private:

public:


private:
};

class GLWidget : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLWidget(QWidget *parent = 0);
    
    void initializeGL();
    void paintGL();
    void resizeGL(int w, int h);

    B2E map;
public slots:
    void chng_map(QString);
    void KeyHandler(QString);

signals:
    //void sectorsChanged(const QString);
    void gridSize(QString value);
    void sentStatus(QString value);

private:
    QTimer timer;
    GLuint init_shaders (GLenum type, const char *filename);
    void program_errors (const GLint program);
    void shader_errors (const GLint shader);
    GLuint init_program (GLuint vertexshader, GLuint fragmentshader);

protected:
    void keyPressEvent(QKeyEvent *event);
    //void mousePressEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
};

inline QString gridText(int grid)
{
    QString ret="<font color=red>";
    ret +=QString::number(grid);
    ret +="</font> / <font color=red>";
    ret +=QString::number(2<<grid);
    ret +="</font> |";

    return ret;
};

#endif // GLWIDGET_H
