#define pi 3.141592653589793238462643383279

varying vec3 vertex;

void main()
{
    vertex      = gl_Vertex.xyz;
    gl_Position = ftransform();
}
