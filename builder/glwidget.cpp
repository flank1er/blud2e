#define GL_GLEXT_PROTOTYPES

#include "mainwindow.h"
#include "glwidget.h"

GLuint vertexshader,fragmentshader,shaderprogram;
GLuint diffuse;
GLuint vertexbuffer;

///////////////////////////////////////
const GLfloat grid_color[] ={0.2f, 0.2f, 0.2f};
const GLfloat sector_color[] ={1.f, 1.f, 1.f};
const GLfloat inner_color[] ={1.f, 0.f, 0.f};
const GLfloat sprite_color[] ={0.f, 0.f, 1.f};

// вывод текста на холст OpenGL посредством GLUT
void draw_string_bitmap(void *font, const char* string) {
        while (*string)
                glutBitmapCharacter(font, *string++);
}

//int GLWidget::getExtensionSupported(QString ext)
//{
//    QString lst=QString::fromUtf8((const char*)glGetString(GL_EXTENSIONS));
//    return lst.lastIndexOf(ext, -1);
//};


void GLWidget::program_errors(const GLint program)
{
    GLint length ;
    GLchar * log ;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length) ;
    log = new GLchar[length+1] ;
    glGetProgramInfoLog(program, length, &length, log) ;
    qDebug() << "Compile Error, Log Below\n" << log;
    delete [] log;
}

void GLWidget::shader_errors(const GLint shader) {
    GLint length ;
    GLchar * log ;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length) ;
    log = new GLchar[length+1] ;
    glGetShaderInfoLog(shader, length, &length, log) ;
    qDebug() << "Compile Error, Log Below\n" << log ;
    delete [] log ;
}

GLuint GLWidget::init_shaders (GLenum type, const char *filename)
{
    // Using GLSL shaders, OpenGL book, page 679

    GLuint shader = glCreateShader(type) ;
    GLint compiled ;
    std::string str= textFileRead (filename) ;
    GLchar * cstr = new GLchar[str.size()+1] ;
    const GLchar * cstr2 = cstr ; // Weirdness to get a const char
    strcpy(cstr,str.c_str()) ;
    glShaderSource (shader, 1, &cstr2, NULL) ;
    glCompileShader (shader) ;
    glGetShaderiv (shader, GL_COMPILE_STATUS, &compiled) ;
    if (!compiled) {
        shader_errors (shader) ;
        throw 3 ;
    }
    return shader;
}

GLuint GLWidget::init_program (GLuint vertexshader, GLuint fragmentshader)
{
    GLuint program = glCreateProgram() ;
    GLint linked ;
    glAttachShader(program, vertexshader) ;
    glAttachShader(program, fragmentshader) ;
    glLinkProgram(program) ;
    glGetProgramiv(program, GL_LINK_STATUS, &linked) ;
    if (linked) glUseProgram(program) ;
    else {
        program_errors(program) ;
        throw 4 ;
    }
    return program ;
}

GLWidget::GLWidget(QWidget *parent) :
    QGLWidget(QGLFormat(), parent)
{
    QGLFormat frmt;
    frmt.setDoubleBuffer(false);
    frmt.setAlpha(false);
    //frmt.setSwapInterval(1);
    setFormat(frmt);
    //qDebug() << frmt.doubleBuffer();

    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));
    timer.start(17);
}

namespace eng {
    GLdouble x0=0;
    GLdouble y0=3000;

    //float scale=25.f;
    int xmin, xmax, ymin, ymax;
    int x_grid, y_grid;
    int x_grid_min, y_grid_min;
    int x_pixel, y_pixel;

    int width, height;

    int gSc     = 7;
    int max     = 131072;
    int min     = -131072;
    float scale = 25.f;

    std::vector<GLfloat> points;
    std::vector<GLfloat> g;
    std::vector<GLfloat> white;
    std::vector<GLfloat> green;
    GLfloat* ar;
    GLfloat* wl;
    GLfloat* gr;
    GLfloat* pnt;
}

void GLWidget::initializeGL()
{
    map.rm();

    vertexshader = init_shaders(GL_VERTEX_SHADER,"shaders/light.vert.glsl");
    fragmentshader = init_shaders(GL_FRAGMENT_SHADER,"shaders/light.frag.glsl");
    shaderprogram = init_program(vertexshader,fragmentshader);

    int step=2<<eng::gSc;

    // creating grid
    for (int i=eng::min; i< eng::max; i+=step )
    {
        eng::g.push_back((GLfloat)eng::min);
        eng::g.push_back((GLfloat)i);
        eng::g.push_back((GLfloat)eng::max);
        eng::g.push_back((GLfloat)i);
        eng::g.push_back((GLfloat)i);
        eng::g.push_back((GLfloat)eng::max);
        eng::g.push_back((GLfloat)i);
        eng::g.push_back((GLfloat)eng::min);
    }
    eng::ar=eng::g.data();

    eng::wl=NULL;
    eng::gr=NULL;
    eng::pnt=NULL;
}

void GLWidget::paintGL()
{
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform4fv(diffuse,1,grid_color);
    glLineWidth((GLfloat)1.f);

    glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, eng::ar);
        glDrawArrays(GL_LINES,0,(int)(eng::g.size()/2));

        glUniform4fv(diffuse,1,sector_color);
        glVertexPointer(2, GL_FLOAT, 0, eng::wl);
        glDrawArrays(GL_LINES,0,(int)(eng::white.size()/2));

        glUniform4fv(diffuse,1,inner_color);
        glVertexPointer(2, GL_FLOAT, 0, eng::gr);
        glDrawArrays(GL_LINES,0,(int)(eng::green.size()/2));

        glUniform4fv(diffuse, 1, sector_color);
        glPointSize(3.f);
        glVertexPointer(2, GL_FLOAT, 0, eng::wl);
        glDrawArrays(GL_POINTS, 0, (int)eng::white.size());
        glVertexPointer(2, GL_FLOAT, 0, eng::gr);
        glDrawArrays(GL_POINTS, 0, (int)eng::green.size());

        glUniform4fv(diffuse, 1, sprite_color);
        glPointSize(5.f);
        glVertexPointer(2, GL_FLOAT, 0, eng::pnt);
        glDrawArrays(GL_POINTS, 0, (int)eng::points.size());


    glDisableClientState(GL_VERTEX_ARRAY);

    ///   L A B E L
    //glRasterPos2f(600, 50);
    //std::string s="fps: ";
    //const char *cstr = s.c_str();
    //draw_string_bitmap(GLUT_BITMAP_HELVETICA_18, cstr);

}

void GLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    eng::width=w;
    eng::height=h;
    eng::xmin=eng::x0-(w/2.f * eng::scale);
    eng::xmax=eng::x0+(w/2.f * eng::scale);
    eng::ymin=eng::y0-(h/2.f * eng::scale);
    eng::ymax=eng::y0+(h/2.f * eng::scale);

    gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin); // right, left, bottom, top
   //gluOrtho2D(0.f, w*eng::scale, h*eng::scale, 0.f); // right, left, bottom, top
    //qDebug() << "width: " << w<< " height: " << h;
}

void GLWidget::init_map(bool)
 {

    map.getWhiteOutline(eng::white);
    eng::wl=eng::white.data();
    map.getRedOutline(eng::green);
    eng::gr=eng::green.data();
 }

void GLWidget::chng_map(QString)
{
    map.getWhiteOutline(eng::white);
    eng::wl=eng::white.data();
    map.getRedOutline(eng::green);
    eng::gr=eng::green.data();
    map.getPointsOutline(eng::points);
    eng::pnt=eng::points.data();
}



void GLWidget::KeyHandler(QString cmd)
{
 //   qDebug() << cmd;
    if (cmd == "zoom_plus")
    {
        glViewport(0, 0, (GLsizei)eng::width, (GLsizei)eng::height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        eng::scale -=5;
        eng::xmin=eng::x0-(eng::width/2.f * eng::scale);
        eng::xmax=eng::x0+(eng::width/2.f * eng::scale);
        eng::ymin=eng::y0-(eng::height/2.f * eng::scale);
        eng::ymax=eng::y0+(eng::height/2.f * eng::scale);

        gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin);
    } else if (cmd == "zoom_minus")
    {
        glViewport(0, 0, (GLsizei)eng::width, (GLsizei)eng::height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        eng::scale +=5;
        eng::xmin=eng::x0-(eng::width/2.f * eng::scale);
        eng::xmax=eng::x0+(eng::width/2.f * eng::scale);
        eng::ymin=eng::y0-(eng::height/2.f * eng::scale);
        eng::ymax=eng::y0+(eng::height/2.f * eng::scale);

        gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin);
    } else if (cmd == "left")
    {
        glViewport(0, 0, (GLsizei)eng::width, (GLsizei)eng::height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        eng::x0 -=2<<eng::gSc;
        eng::xmin=eng::x0-(eng::width/2.f * eng::scale);
        eng::xmax=eng::x0+(eng::width/2.f * eng::scale);
        eng::ymin=eng::y0-(eng::height/2.f * eng::scale);
        eng::ymax=eng::y0+(eng::height/2.f * eng::scale);

        gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin);
    } else if (cmd == "right")
    {
        glViewport(0, 0, (GLsizei)eng::width, (GLsizei)eng::height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        eng::x0 +=2<<eng::gSc;
        eng::xmin=eng::x0-(eng::width/2.f * eng::scale);
        eng::xmax=eng::x0+(eng::width/2.f * eng::scale);
        eng::ymin=eng::y0-(eng::height/2.f * eng::scale);
        eng::ymax=eng::y0+(eng::height/2.f * eng::scale);

        gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin);
    }else if (cmd == "up")
    {
        glViewport(0, 0, (GLsizei)eng::width, (GLsizei)eng::height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        eng::y0 -=2<<eng::gSc;
        eng::xmin=eng::x0-(eng::width/2.f * eng::scale);
        eng::xmax=eng::x0+(eng::width/2.f * eng::scale);
        eng::ymin=eng::y0-(eng::height/2.f * eng::scale);
        eng::ymax=eng::y0+(eng::height/2.f * eng::scale);

        gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin);
    } else if (cmd == "down")
    {
        glViewport(0, 0, (GLsizei)eng::width, (GLsizei)eng::height);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        eng::y0 +=2<<eng::gSc;
        eng::xmin=eng::x0-(eng::width/2.f * eng::scale);
        eng::xmax=eng::x0+(eng::width/2.f * eng::scale);
        eng::ymin=eng::y0-(eng::height/2.f * eng::scale);
        eng::ymax=eng::y0+(eng::height/2.f * eng::scale);

        gluOrtho2D(eng::xmin, eng::xmax, eng::ymax, eng::ymin);
    } else if (cmd == "grid_increment")
    {
        if (eng::gSc > 4)  eng::gSc--; else eng::gSc=9;

        // rebuild grid
        int step=2<<eng::gSc;
        eng::g.erase(eng::g.begin(), eng::g.end());
        for (int i=eng::min; i< eng::max; i+=step )
        {
            eng::g.push_back((GLfloat)eng::min);
            eng::g.push_back((GLfloat)i);
            eng::g.push_back((GLfloat)eng::max);
            eng::g.push_back((GLfloat)i);
            eng::g.push_back((GLfloat)i);
            eng::g.push_back((GLfloat)eng::max);
            eng::g.push_back((GLfloat)i);
            eng::g.push_back((GLfloat)eng::min);
        }
        eng::ar=eng::g.data();

        emit gridSize(gridText(eng::gSc));
        //qDebug() << "grid increment" << eng::gSc;
    } else if (cmd == "grid_decrement")
    {
        qDebug() << "gred decrement!!";
    }
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    switch(event->key())
    {
        case Qt::Key_4:
            qDebug() << "!!!";
            break;

        default:
            QWidget::keyPressEvent(event);
    }

}
