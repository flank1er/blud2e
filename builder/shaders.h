#include <iostream>
#include <string>

#ifndef INCLUDESHADERS
#define INCLUDESHADERS

std::string textFileRead (const char * filename) ;
void programerrors (const GLint program) ;
void shadererrors (const GLint shader) ;
GLuint initshaders (GLenum type, const char * filename) ;
GLuint initprogram (GLuint vertexshader, GLuint fragmentshader) ;

#endif // SHADERS_H
