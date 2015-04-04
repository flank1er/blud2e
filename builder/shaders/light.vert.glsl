# version 120

varying vec4 color ;
varying vec3 mynormal ;
varying vec4 myvertex ;

void main() {
    gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex ;
    color = gl_Color ;
    mynormal = gl_Normal ;
    myvertex = gl_Vertex ;

}
