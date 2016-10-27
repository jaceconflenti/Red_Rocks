/*
 * Key bindings:
 * ESC		Exit
 * a/A 		Toggle axes
 * s/S 		Toggle light movement
 * l/L 		Cycle light
 * [/] 		Move light
 * -/+ 		Change light elevation
 * 0 		Reset viewing angle
 * 1/2		Decrease/Increase fov
 * 3/4 		Decrease/Increase dim  
 */

#include "CSCIx229.h"

#define AXES 100.0 //  Length of axes
#define LD 90.0    //  Radius of sun

int fov = 55;      //  Field of view 
double asp = 1;    //  Aspect ratio
double dim = 100.0;//  Size of world
int axes = 1;      //  Display axes
int th = 0;        //  Azimuth of view angle
int ph = 0;        //  Elevation of view angle
int zh = 0;        //  Azimouth of light
int light = 1;     //  Lighting
double yl = 0.0;   //  Elevation of light
int move = 1;      //  Move light
double Ex = 1;     //  X-coordinate of eye
double Ey = 1;     //  Y-coordinate of eye
double Ez = 1;     //  Z-coordinate of eye

static void ball(double x,double y,double z,double r)
{
   //  Save transformation
   glPushMatrix();
   //  Offset, scale and rotate
   glTranslated(x,y,z);
   glScaled(r,r,r);
   //  White ball
   glColor3f(1,1,0);
   glutSolidSphere(1.0,16,16);
   //  Undo transofrmations
   glPopMatrix();
}

void idle()
{
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   glutPostRedisplay();
}

void key(unsigned char ch, int x, int y)
{
	//  Exit on ESC
	if (ch == 27)
	{
		exit(0);
	}	
	//  Toggles the axes
	else if (ch == 'a' || ch == 'A')
	{
		axes = 1 - axes;
	}
	//  Toggles light movement
	else if (ch == 's' || ch == 'S')
	{
		move = 1 - move;
	}
	//  Cycles light
	else if (ch == 'l')
	{
		light = (light+1)%3;
	}
	else if (ch == 'L')
	{
		light = (light+2)%3;
	}
	//  Moves light
	else if (ch == '[')
	{
		zh -= 1; 
	}
	else if (ch == ']')
	{
		zh += 1; 
	}
	//  Change light elevation
	else if (ch == '-')
	{
		yl -= 5.0; 
	}
	else if (ch == '+')
	{
		yl += 5.0; 
	}
	//  Reset viewing angle
	else if (ch == '0')
	{
		th = ph = 0;
	}
	//  Decreases fov
	else if (ch == '1')
	{
		fov--;
	}
	//  Increase fov
	else if (ch == '2')
	{
		fov++;
	}
	//  Decrease dim
	else if (ch == '3') {
		dim -= 5.0;
	}
	//  Decrease dim
	else if (ch == '4') {
		dim += 5.0;
	}

	//  Reproject
	Project(fov,asp,dim);

	//  Animate if requested
	glutIdleFunc(move?idle:NULL);

	//  Tell GLUT it is necessary to redisplay the scene
	glutPostRedisplay();
}

void special(int key, int x, int y)
{
	//  Right arrow key - increase angle by 5 degrees
	if (key == GLUT_KEY_RIGHT) {
		th += 5;
	}
	//  Left arrow key - decrease angle by 5 degrees
	else if (key == GLUT_KEY_LEFT) {
		th -= 5;
	}
	//  Up arrow key - increase elevation by 5 degrees
	else if (key == GLUT_KEY_UP) {
		ph += 5;
	}
	//  Down arrow key - decrease elevation by 5 degrees
	else if (key == GLUT_KEY_DOWN) {
		ph -= 5;
	}

	//  Keep angles to +/-360 degrees
	th %= 360;
	ph %= 360;

	//  Update projection
	Project(fov,asp,dim);

	//  Tell GLUT it is necessary to redisplay the scene
	glutPostRedisplay();
}

void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(fov,asp,dim);
}

void display()
{
	//  Clear screen and Z-buffer
	glClearColor(0,0.3,0.7,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	//  Enable Z-buffering in OpenGL
    glEnable(GL_DEPTH_TEST);
	//  Reset transformations
	glLoadIdentity();

	//  Calulate eye coordinates 
	double Ex = -2*dim*Sin(th)*Cos(ph);
	double Ey = +2*dim        *Sin(ph);
	double Ez = +2*dim*Cos(th)*Cos(ph);
	gluLookAt(Ex,Ey,Ez, 0,0,0 , 0,Cos(ph),0);

	glPushMatrix();

	glRotated(-90,1,0,0);

	//  Draw sky

	//  Light switch
	if (light)
	{
		float F = (light==2) ? 1 : 0.3;
		//  Translate intensity to color vectors
		float Ambient[]   = {0.3*F,0.3*F,0.3*F,1.0};
		float Diffuse[]   = {0.5*F,0.5*F,0.5*F,1.0};
		float Specular[]  = {1.0*F,1.0*F,1.0*F,1.0};
		float white[] = {1.0,1.0,1.0,1.0};
		//  Light position
		float Position[]  = {LD*Cos(zh),yl,LD*Sin(zh),1.0};
		//  Draw light position as ball (still no lighting here)
		ball(Position[0],Position[1],Position[2],5);
		//  Enbale normalization 
		glEnable(GL_NORMALIZE);
		//  Enable lighting
		glEnable(GL_LIGHTING);
		//  glColor sets ambient and diffuse color materials
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		//  Enable light 0
		glEnable(GL_LIGHT0);
		//  Set ambient, diffuse, specular components and position of light 0
		glLightfv(GL_LIGHT0,GL_AMBIENT,Ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,Diffuse);
		glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
		glLightfv(GL_LIGHT0,GL_POSITION,Position);
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,32.0f);
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,white);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}

	//  Draw scene
	

	//  Turn lighting off
	glDisable(GL_LIGHTING);

	//  Draw axes
	glColor3f(1,1,1);
	if (axes)
	{
		glBegin(GL_LINES);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(AXES,0.0,0.0);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(0.0,AXES,0.0);
		glVertex3d(0.0,0.0,0.0);
		glVertex3d(0.0,0.0,AXES);
		glEnd();
		//  Label axes
		glRasterPos3d(AXES,0.0,0.0);
		Print("X");
		glRasterPos3d(0.0,AXES,0.0);
		Print("Y");
		glRasterPos3d(0.0,0.0,AXES);
		Print("Z");
	}

	glPopMatrix();

	//  Display parameters
	glWindowPos2i(5,5);
	Print("Angle=%d,%d  Dim=%.1f  FOV=%d  Light=%s",th,ph,dim,fov,light?"On":"Off");
	if (light)
	{
		Print("  Light=%d,%.1f",zh,yl);
	}

	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	glutSwapBuffers();	
}

int main(int argc, char *argv[])
{
	//  Initialize GLUT
	glutInit(&argc,argv);
	//  Request double buffered true color window with Z-buffer
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	//  Sets initial window size	
	glutInitWindowSize(1280, 720);	
	//  Create window
	glutCreateWindow("Project: Steven Conflenti");
	
	//  Register display, reshape, key, and idle callbacks
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(key);
   	glutIdleFunc(idle);

   	//  Load textures
   	

	//  Pass control to GLUT for events
	ErrCheck("init");
	glutMainLoop();
	
	//  Return to OS
	return 0;
}