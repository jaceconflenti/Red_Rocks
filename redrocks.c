/*
 * Key bindings:
 * ESC		Exit
 * Spacebar Pause music
 * n/m  	Previous/Next song
 * s 		Toggle sky
 * d 		Toggle light movement
 * f 		Cycle light
 * [/] 		Move light
 * -/+ 		Change light elevation
 * 1/2		Decrease/Increase fov
 * 3/4 		Decrease/Increase dim
 * 5 		Default viewing angle
 * 6 		Alternate viewing angle
 */

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "CSCIx229.h"

#define LEN 8192        //  Max length of text string 

#define SONGS 2			//  Number of songs
const char *songs[SONGS] = {"Nyboda.mp3", "The_Unborn_Dancing.mp3"};  //  Songs downloaded from freemusicarchive.org
Mix_Music* music[SONGS];
int play = 0;			//  Play music
int track = 0;          //  Music track

#define AXES 1500.0     //  Length of axes
double dim = AXES;      //  'Radius' of world
int fov = 60;           //  Field of view 
double asp = 1;         //  Aspect ratio
int axes = 1;           //  Display axes
int th = 190;           //  Azimuth of view angle
int ph = 5;             //  Elevation of view angle
int zh = 0;             //  Azimouth of light
int Th = 180;			//  Azimuth of spotlight
int Ph = 30;			//  Elevation of spotlight
float sco = 180;        //  Spot cuttoff angle
float Exp = 0;          //  Spot exponent
int inf = 0;            //  Infinite distance light
int light = 1;          //  Lighting
double yl = 0.0;        //  Elevation of light
int move = 1;           //  Move light
double Ex = 1;          //  X-coordinate of eye
double Ey = 1;          //  Y-coordinate of eye
double Ez = 1;          //  Z-coordinate of eye
double Ox = -350;	    //  Look-at x
double Oy = 20;	        //  Look-at y
double Oz = 2000;	    //  Look-at z
int X,Y;                //  Last mouse coordinates
int mouse = 0;          //  Move mode    
unsigned int rock[6];   //  Rock/surface textures
unsigned int sky[2];    //  Sky textures
int shader = 0;         //  Shader program
int left, right, tree, dome, spotlight;  //  Belnder object display lists
int skySwitch = 1;      //  Sky switch

//  Base plane for scene
static void drawGround(double x, double y, double z, double th)
{	
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);

	glRotated(th,1,0,0);

	double vert = AXES * 2;

	glBegin(GL_QUADS);
	glColor3f(1,0.5,0.35);
	glNormal3f(0, 0, +1);
	glVertex3f(-vert, vert, 0);
	glVertex3f(-vert, -vert, 0);
	glVertex3f(vert, -vert, 0);
	glVertex3f(vert, vert, 0);
	glEnd();

	//  Undo transformations
	glPopMatrix();
}

//  Mode: 1 = left rock, else = right rock
static void drawRock(const int mode, double x, double y, double z, double dx, double dy, double dz, double th, double ph, double zh)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glRotated(th,1,0,0);
	glRotated(ph,0,1,0);
	glRotated(zh,0,0,1);
	glScaled(dx,dy,dz);

	glColor3f(1,0.5,0.35);  
	//glBindTexture(GL_TEXTURE_2D, rock[6]);  //  Stone

	if (mode == 1)
	{
		glCallList(left);
	}
	else
	{
		glCallList(right);
	}

	//  Undo transformations
	glPopMatrix();
}

static void drawTree(double x, double y, double z, double dx, double dy, double dz, double th, double ph, double zh)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glRotated(th,1,0,0);
	glRotated(ph,0,1,0);
	glRotated(zh,0,0,1);
	glScaled(dx,dy,dz);

	glCallList(tree);

	//  Undo transformations
	glPopMatrix();
}

//  Draws a geodesic dome
static void drawDome(double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glRotated(th,1,0,0);
	glScaled(dx,dy,dz);

	glCallList(dome);

	//  Undo transformations
	glPopMatrix();
}

//  Curved rectangular prism 
//  mode: 1 = wood, else = brick
static void row(const int mode, double x, double y, double z, double dx, double dy, double dz, double th)
{
	const double d = 5.0;  //  Degress per step
	const double slices = 180.0 / d;

	//  Save transformation
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	//glRotated(th,0,0,1);
	glScaled(dx,dy,dz);

	if (mode == 1)
	{
		glColor3f(0.7,0.55,0.43);  //  Light taupe
		glBindTexture(GL_TEXTURE_2D, rock[3]);  //  Wood
	}
	else
	{
		glColor3f(1.0,0.86,0.73);  //  Peach
		glBindTexture(GL_TEXTURE_2D, rock[0]);  //  Brick
	}
	
	//  Front
	for (int j=0; j<=slices; j++)
	{
		glBegin(GL_QUADS);
		glNormal3d(-Cos(j*d), -Sin(j*d), 0);
		glTexCoord2f(0,0); glVertex3d(Cos(j*d),Sin(j*d)-1,-1);
		glNormal3d(-Cos((j+1)*d), -Sin((j+1)*d), 0);
		glTexCoord2f(1,0); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)-1,-1);
		glNormal3d(-Cos((j+1)*d), -Sin((j+1)*d), 0);
		glTexCoord2f(1,1); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)-1,+1);
		glNormal3d(-Cos(j*d), -Sin(j*d), 0);
		glTexCoord2f(0,1); glVertex3d(Cos(j*d),Sin(j*d)-1,+1);
		glEnd();
	}
	//  Back
	for (int j=0; j<=slices; j++)
	{
		glBegin(GL_QUADS);
		glNormal3d(Cos(j*d), Sin(j*d), 0);
		glTexCoord2f(0,0); glVertex3d(Cos(j*d),Sin(j*d)+1,-1);
		glNormal3d(Cos((j+1)*d), Sin((j+1)*d), 0);
		glTexCoord2f(1,0); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)+1,-1);
		glNormal3d(Cos((j+1)*d), Sin((j+1)*d), 0);
		glTexCoord2f(1,1); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)+1,+1);
		glNormal3d(Cos(j*d), Sin(j*d), 0);
		glTexCoord2f(0,1); glVertex3d(Cos(j*d),Sin(j*d)+1,+1);
		glEnd();
	}
	//  Right
	glBegin(GL_QUADS);
	glNormal3f(+1, 0, 0);
	glTexCoord2f(1,0); glVertex3f(+1,-1,+1);
	glTexCoord2f(1,1); glVertex3f(+1,-1,-1);
	glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
	glTexCoord2f(0,0); glVertex3f(+1,+1,+1);
	glEnd();
	//  Left
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(1,1); glVertex3f(-1,-1,+1);
	glTexCoord2f(0,1); glVertex3f(-1,+1,+1);
	glTexCoord2f(0,0); glVertex3f(-1,+1,-1);
	glEnd();


	if (mode == 1)
	{
		glBindTexture(GL_TEXTURE_2D, rock[3]);  //  Wood
	}
	else 
	{
		glBindTexture(GL_TEXTURE_2D, rock[1]);  //  Concrete   
	}  

	//  Top
	glNormal3f(0, 0, +1);
	for (int j=0; j<=slices; j++)
	{		
		glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex3d(Cos(j*d),Sin(j*d)-1,+1);
		glTexCoord2f(1,0); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)-1,+1);
		glTexCoord2f(1,1); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)+1,+1);
		glTexCoord2f(0,1); glVertex3d(Cos(j*d),Sin(j*d)+1,+1);
		glEnd();
	}
	//  Bottom
	glNormal3f(0, 0, -1);
	for (int j=0; j<=slices; j++)
	{
		glBegin(GL_QUADS);
		glTexCoord2f(0,0); glVertex3d(Cos(j*d),Sin(j*d)-1,+1);
		glTexCoord2f(1,0); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)-1,+1);
		glTexCoord2f(1,1); glVertex3d(Cos((j+1)*d),Sin((j+1)*d)+1,+1);
		glTexCoord2f(0,1); glVertex3d(Cos(j*d),Sin(j*d)+1,+1);
		glEnd();
	}

	//  Undo transofrmations
	glPopMatrix();
}

static void stands(const int num, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glRotated(th,0,0,1);

	for (int i = 0; i < num; i++)
	{ 
		row(0,x,y+(i*dy*2),z+(i*dz*2),dx,dy,dz,th);  //  Brick
		row(1,x,y+(i*dy*2)-2.65,z+(i*dz*2)+.8,dx-.02,dy/4,dz/4,th);  //  Wood
	}
	//  Undo transformations
	glPopMatrix();
}


//  Credit for spotlight blender model goes to Atzibala 
//  http://www.blendswap.com/blends/view/75624
static void drawSpotlight(const int lightNum, double x, double y, double z, double dx, double dy, double dz, double r, double g, double b)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glScaled(dx,dy,dz);

	//glCallList(spotlight);
	ErrCheck("after spotlight");

	//  Undo transformations
	glPopMatrix();
}

//  mode: 1 = brick, 2 = metal, 3 = concrete, 4 = DJ booth, else concrete
static void cube(const int mode, double x, double y, double z, double dx, double dy, double dz)
{
	//  Save transformation
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glScaled(dx,dy,dz);

	if (mode == 2)
	{
		glColor3f(1.0,0.0,0.0);
		glBindTexture(GL_TEXTURE_2D, rock[2]);  //  Metal
	}
	else if (mode == 3)
	{
		glColor3f(0.73,0.73,0.73);
		glBindTexture(GL_TEXTURE_2D, rock[1]);
	}
	else if (mode == 4)
	{
		glBindTexture(GL_TEXTURE_2D, rock[4]);  //  DJ
	}
	else 
	{
		glColor3f(1.0,0.99,0.73);
		glBindTexture(GL_TEXTURE_2D, rock[1]);  //  Concrete
	}

	//  Top
	glBegin(GL_QUADS);
	glNormal3f(0,0,+1);
	glTexCoord2f(0,0); glVertex3f(-1,-1,+1);
	glTexCoord2f(1,0); glVertex3f(+1,-1,+1);
	glTexCoord2f(1,1); glVertex3f(+1,+1,+1);
	glTexCoord2f(0,1); glVertex3f(-1,+1,+1);
	glEnd();
	//  Bottom
	glBegin(GL_QUADS);
	glNormal3f(0,0,-1);
	glTexCoord2f(0,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(1,1); glVertex3f(-1,+1,-1);
	glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
	glEnd();

	if (mode == 1) 
	{
		glBindTexture(GL_TEXTURE_2D, rock[0]);  //  Brick 
	}
	else if (mode == 4)
	{
		glBindTexture(GL_TEXTURE_2D, rock[5]); //  LED
	}

	//  Right
	glBegin(GL_QUADS);
	glNormal3f(+1, 0, 0);
	glTexCoord2f(1,0); glVertex3f(+1,-1,+1);
	glTexCoord2f(1,1); glVertex3f(+1,-1,-1);
	glTexCoord2f(0,1); glVertex3f(+1,+1,-1);
	glTexCoord2f(0,0); glVertex3f(+1,+1,+1);
	glEnd();
	//  Left
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(1,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(1,1); glVertex3f(-1,-1,+1);
	glTexCoord2f(0,1); glVertex3f(-1,+1,+1);
	glTexCoord2f(0,0); glVertex3f(-1,+1,-1);
	glEnd();
	//  Back
	glBegin(GL_QUADS);
	glNormal3f( 0,+1, 0);
	glTexCoord2f(0,0); glVertex3f(-1,+1,+1);
	glTexCoord2f(1,0); glVertex3f(+1,+1,+1);
	glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
	glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
	glEnd();
	//  Front
	glBegin(GL_QUADS);
	glNormal3f( 0,-1, 0);
	glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(1,1); glVertex3f(+1,-1,+1);
	glTexCoord2f(0,1); glVertex3f(-1,-1,+1);
	glEnd();

	//  Undo transofrmations
	glPopMatrix();
}

static void stairs(int num, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glRotated(th,0,0,1);

	for (int i = 0; i < num; i++)
	{ 
		cube(0,x,y+(i*dy),z+(i*dz*2),dx,dy,dz);
	}
	//  Undo transformations
	glPopMatrix();
}

//  Brick rectangular boxes and trees
static void pathEdge(int num, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glRotated(th,0,0,1);

	int xTree = (x > 0) ? x+50 : x-50; 
	int x2Tree = (x > 0) ? x+80 : x-110;
	int x3Tree = (x > 0) ? x+105 : x-160; 

	for (int i = 0; i < num; i++)
	{ 
		if (((i%5) == 0) && (i != 0))
		{
			drawTree(xTree,y+(i*dy*4),z+(i*dz*4)-8,dx/5,dy/5,dz*.75,th+21,0,0);			
		}

		cube(1,x,y+(i*dy*4),z+(i*dz*4),dx,dy,dz); 
	}

	if (x < 0)
	{
		drawTree(x2Tree,y+(13*dy*4),z+(13*dz*4)-8,dx/8,dy/8,dz*.5,th+21,0,0);
		drawTree(x3Tree,y+(10*dy*4),z+(10*dz*4)-10,dx/3,dy/3,dz,th+21,0,0);
		drawTree(x3Tree,y+(23*dy*4),z+(23*dz*4)-10,dx/3,dy/3,dz,th+21,0,0);	
		drawTree(x-125,y+(18*dy*4),z+(18*dz*4)-8,dx/5,dy/5,dz*.75,th+21,0,0);		
	}
	else 
	{
		drawTree(x+35,y+(3*dy*4),z+(3*dz*4)-8,dx/8,dy/8,dz*.5,th+21,0,0);
		drawTree(x3Tree,y+(18*dy*4),z+(18*dz*4)-8,dx/8,dy/8,dz*.5,th+21,0,0);
		drawTree(x2Tree,y+(28*dy*4),z+(28*dz*4)-8,dx/8,dy/8,dz*.5,th+21,0,0);
	}

	//  Undo transformations
	glPopMatrix();
}

static void cylinder(const int _stacks, double x, double y, double z, double r, double h, double th)
{
	const double d = 10.0;  //  Degress per step
	const double stacks = _stacks;
	const double slices = 360.0 / d;
	const double zStep = h / stacks;

	int i, j;
	double z0, z1;

	//  Save transformation
	glPushMatrix();

	//  Offset
	glTranslated(x,y,z);
	glRotated(th,1,0,0);

	glColor3d(1,0,0);  //  Red
	glBindTexture(GL_TEXTURE_2D, rock[2]);  //  Metal

	//  Construct top cricle
	glNormal3d(0,0,1);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5, 0.5);
	glVertex3d(0,0,h);  //  Center point of circle
	for (i=slices; i>=0; i--)
	{
		glTexCoord2f(0.5 * Cos(i*d) + 0.5, 0.5 * Sin(i*d) + 0.5);
		glVertex3d(r * Cos(i*d), r * Sin(i*d), h);
	}
	glEnd();
	
	//  Edge
	z0 = 0.0;
	z1 = zStep;

	for (i = 1; i <= stacks; i++)
	{
		if (i == stacks)
		{
			z1 = h;
		}

		glBegin(GL_QUAD_STRIP);
		for (j=0; j<=slices; j++)
		{
			double u = (double) j / (double) slices;
			glNormal3d(Cos(j*d), Sin(j*d), 0);
			glTexCoord2f(u, 0.0); glVertex3d(r * cos(2*PI*u), r * sin(2*PI*u), z0);
			glTexCoord2f(u, 1.0); glVertex3d(r * cos(2*PI*u), r * sin(2*PI*u), z1);

		}
		glEnd();

		z0 = z1;
		z1 += zStep;
	}

	//  Construct bottom circle
	glNormal3d(0,0,-1);
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5, 0.5);
	glVertex3d(0,0,0);  //  Center point of circle
	for (i=0; i<=slices; i++)
	{
		glTexCoord2f(0.5 * Cos(i*d) + 0.5, 0.5 * Sin(i*d) + 0.5);
		glVertex3d(r * Cos(i*d), r * Sin(i*d), 0);
	}
	glEnd();

	//  Undo transformations
	glPopMatrix();
}

static void stage(double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glRotated(th,0,0,1); 
	glScaled(dx,dy,dz);

	//  Spotlights
	drawSpotlight(1, -120,-50,45, 5,5,5, 0.0,0.0,1.0);
	drawSpotlight(2, -80,-50,50, 5,5,5, 0.0,0.0,1.0);
	drawSpotlight(3, -40,-50,50, 5,5,5, 0.0,0.0,1.0);
	drawSpotlight(4, 0,-50,50, 5,5,5, 0.0,0.0,1.0);
	drawSpotlight(5, 40,-50,50, 5,5,5, 0.0,0.0,1.0);
	drawSpotlight(6, 80,-50,50, 5,5,5, 0.0,0.0,1.0);
	drawSpotlight(7, 120,-50,45, 5,5,5, 0.0,0.0,1.0);

	//  Main stage floor
	for (int i = 0; i < 25; i++)
	{
		int x = i * 8;
		for (int j = 0; j < 25; j++)
		{
			int y = j * 8;
			cube(3, -95+x,-220+y,0, 4,4,2);
		}
	}

	//  DJ Booth
	cube(4, 0,-60,7, 25,12.5,8); 

	//  Right sub-stage 
	cylinder(12, 140,-55,0, 2,60, 0);
	cylinder(12, 140,-85,0, 2,60, 0);
	cylinder(12, 110,-55,0, 2,60, 0);
	cylinder(12, 110,-85,0, 2,60, 0);
	cylinder(12, 110,-145,0, 2,60, 0);
	cylinder(12, 110,-200,0, 2,60, 0);
	
	//  Right sub-stage roof
	for (int i = 0; i < 9; i++)
	{
		int x = i * 6;
		for (int j = 0; j < 32; j++)
		{
			int y = j * 6;
			cube(2, 100+x,-220+y,60, 3,3,3);
		}
	}

	//  Left sub-stage
	cylinder(12, -140,-55,0, 2,60, 0);
	cylinder(12, -140,-85,0, 2,60, 0);
	cylinder(12, -110,-55,0, 2,60, 0);
	cylinder(12, -110,-85,0, 2,60, 0);
	cylinder(12, -110,-145,0, 2,60, 0);
	cylinder(12, -110,-200,0, 2,60, 0);

	//  Left sub-stage roof
	for (int i = 0; i < 9; i++)
	{
		int x = i * 6;
		for (int j = 0; j < 32; j++)
		{
			int y = j * 6;
			cube(2, -100-x,-220+y,60, 3,3,3);
		}
	}

	//  Main stage roof
	for (int i = 0; i < 25; i++)
	{
		int x = i * 8;
		for (int j = 0; j < 25; j++)
		{
			int y = j * 8;
			cube(2, -95+x,-220+y,68, 4,4,5);
		}
	}

	//  Undo transformations
	glPopMatrix();
}

//  Adapted from example 18 code
static void Vertex(int th,int ph)
{
	double x = -Sin(th)*Cos(ph);
	double y =  Cos(th)*Cos(ph);
	double z =          Sin(ph);
	glNormal3d(x,y,z);
	glVertex3d(x,y,z);
}

//  Adapted from example 13 code
static void ball(double x,double y,double z,double r)
{
	int th,ph;
	int inc = 10;

	//  Save transformation
	glPushMatrix();
	//  Offset, scale 
	glTranslated(x,y,z);
	glScaled(r,r,r);

	glColor3f(1.0,1.0,0.0);

	//  Bands of latitude
	for (ph=-90;ph<90;ph+=inc)
	{
		glBegin(GL_QUAD_STRIP);
		for (th=0;th<=360;th+=2*inc)
		{
			Vertex(th,ph);
			Vertex(th,ph+inc);
		}
		glEnd();
	}

	//  Undo transofrmations
	glPopMatrix();
}

static void drawScene()
{
	stage(-20,0,0, 1,1,1, -10); 
	stairs(240, -175,0,-.5, 5,5,.5, -10);
	stairs(240, 135,0,-.5, 5,5,.5, -10);
	pathEdge(30, -190,5,1, 10,10,2, -10);
	pathEdge(30, 150,5,1, 10,10,2, -10);
	stands(120, -20,0,0, 150,5,1, -10);
	drawRock(1, 38,525,100, 750,2250,1, 11,-2,60);            //  Stand patch
	drawRock(1, -60,1360,110.5, 900,900,1, 90,168.5,174.25);  //  Back patch
	drawRock(1, 315,1000,-50, 1250,1250,1250, 100,160,-4)  ;  //  Left rock
	drawRock(0, 626,-180,-100, 1000,1000,1000, 0,30,-4);      //  Right rock
	drawGround(0,500,0, 0);
}

static void drawSky()
{
	glUseProgram(shader);

	int id = glGetUniformLocation(shader, "blue");
	int id2 = glGetUniformLocation(shader, "star");
	
	if (id >= 0) glUniform1i(id, 0);
	if (id2 >= 0) glUniform1i(id2, 1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sky[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sky[1]);

	drawDome(0,750,-50, 1500,1500,1500, 90);

	glActiveTexture(GL_TEXTURE0);
	glUseProgram(0);
}

//  Shader functions adapted from Ex 27
char* ReadText(char *file)
{
   int   n;
   char* buffer;
   //  Open file
   FILE* f = fopen(file,"rt");
   if (!f) Fatal("Cannot open text file %s\n",file);
   //  Seek to end to determine size, then rewind
   fseek(f,0,SEEK_END);
   n = ftell(f);
   rewind(f);
   //  Allocate memory for the whole file
   buffer = (char*)malloc(n+1);
   if (!buffer) Fatal("Cannot allocate %d bytes for text file %s\n",n+1,file);
   //  Snarf the file
   if (fread(buffer,n,1,f)!=1) Fatal("Cannot read %d bytes for text file %s\n",n,file);
   buffer[n] = 0;
   //  Close and return
   fclose(f);
   return buffer;
}

void PrintShaderLog(int obj,char* file)
{
   int len=0;
   glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&len); 
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for shader log\n",len);
      glGetShaderInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s:\n%s\n",file,buffer);
      free(buffer);
   }
   glGetShaderiv(obj,GL_COMPILE_STATUS,&len);
   if (!len) Fatal("Error compiling %s\n",file);
}

void PrintProgramLog(int obj)
{
   int len=0;
   glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&len);
   if (len>1)
   {
      int n=0;
      char* buffer = (char *)malloc(len);
      if (!buffer) Fatal("Cannot allocate %d bytes of text for program log\n",len);
      glGetProgramInfoLog(obj,len,&n,buffer);
      fprintf(stderr,"%s\n",buffer);
   }
   glGetProgramiv(obj,GL_LINK_STATUS,&len);
   if (!len) Fatal("Error linking program\n");
}

int CreateShader(GLenum type,char* file)
{
   //  Create the shader
   int shader = glCreateShader(type);
   //  Load source code from file
   char* source = ReadText(file);
   glShaderSource(shader,1,(const char**)&source,NULL);
   free(source);
   //  Compile the shader
   fprintf(stderr,"Compile %s\n",file);
   glCompileShader(shader);
   //  Check for errors
   PrintShaderLog(shader,file);
   //  Return name
   return shader;
}

int CreateShaderProg(char* VertFile,char* FragFile)
{
   //  Create program
   int prog = glCreateProgram();
   //  Create and compile vertex shader
   int vert = CreateShader(GL_VERTEX_SHADER  ,VertFile);
   //  Create and compile fragment shader
   int frag = CreateShader(GL_FRAGMENT_SHADER,FragFile);
   //  Attach vertex shader
   glAttachShader(prog,vert);
   //  Attach fragment shader
   glAttachShader(prog,frag);
   //  Link program
   glLinkProgram(prog);
   //  Check for errors
   PrintProgramLog(prog);
   //  Return name
   return prog;
}

int key()
{
	Uint8* keys = SDL_GetKeyState(NULL);
	int shift = SDL_GetModState()&KMOD_SHIFT;

	//  Exit on ESC
	if (keys[SDLK_ESCAPE])
	{
		return 0;
	}	
	//  Toggles sky
	else if (keys[SDLK_s])
	{
		skySwitch = 1 - skySwitch;
	}
	//  Toggles light movement
	else if (keys[SDLK_d])
	{
		move = 1 - move;
	}
	//  Cycles light
	else if (keys[SDLK_f])
	{
		light = (light+1)%3;
	}

	else if (keys[SDLK_l] && shift)
	{
		light = (light+2)%3;
	}
	//  Moves light
	else if (keys[SDLK_LEFTBRACKET])
	{
		zh -= 1; 
	}
	else if (keys[SDLK_RIGHTBRACKET])
	{
		zh += 1; 
	}
	//  Change light elevation
	else if (keys[SDLK_KP_MINUS] || keys[SDLK_MINUS])
	{
		yl -= 5.0; 
	}
	else if (keys[SDLK_KP_PLUS] || keys[SDLK_PLUS])
	{
		yl += 5.0; 
	}
	//  Decreases fov
	else if (keys[SDLK_1])
	{
		fov--;
	}
	//  Increase fov
	else if (keys[SDLK_2])
	{
		fov++;
	}
	//  Decrease dim
	else if (keys[SDLK_3]) 
	{
		dim -= 5.0;
	}
	//  Increase dim
	else if (keys[SDLK_4]) 
	{
		dim += 5.0;
	}
	//  Default viewing angle
	else if (keys[SDLK_5])
	{
		th = 190;           
		ph = 5;
		Ox = -350;	   
		Oy = 20;	     
		Oz = 2000;
	}
	//  No sky viewing angle
	else if (keys[SDLK_6])
	{
		th = 10;           
		ph = 5;
		Ox = 350;	   
		Oy = 20;	     
		Oz = -2250;
	}
	//  Previous song
	else if (keys[SDLK_n]) 
	{
		if (track != 0)
		{
			track -= 1;
		}
		else
		{
			track = SONGS;
		}
		if (play)
		{
			//  Play current track
			Mix_PlayMusic(music[track],-1);
		}
	}
	//  Next song
	else if (keys[SDLK_m]) 
	{
		if (track+1 < SONGS)
		{
			track += 1;
		}
		else
		{
			track = 0;
		}
		if (play)
		{
			//  Play current track
			Mix_PlayMusic(music[track],-1);
		}
	}
	//  Toggle music
	else if (keys[SDLK_SPACE]) 
	{
		play = 1 - play;
		if (play)
		{
			//  Play current track
			Mix_PlayMusic(music[track],-1);
		}
	}
	//  Right arrow key - increase angle by 5 degrees
	else if (keys[SDLK_RIGHT]) 
	{
		th += 5;
	}
	//  Left arrow key - decrease angle by 5 degrees
	else if (keys[SDLK_LEFT]) 
	{
		th -= 5;
	}
	//  Up arrow key - increase elevation by 5 degrees
	else if (keys[SDLK_UP]) 
	{
		ph += 5;
	}
	//  Down arrow key - decrease elevation by 5 degrees
	else if (keys[SDLK_DOWN]) 
	{
		ph -= 5;
	}

	//  Keep angles to +/-360 degrees
	th %= 360;
	ph %= 360;
	zh %= 360;

	//  Reproject
	Project(fov,asp,dim);

	return 1;
}

void reshape(int width, int height)
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
	double Ex = Ox-2*dim*Sin(th)*Cos(ph);
	double Ey = Oy+2*dim        *Sin(ph);
	double Ez = Oz+2*dim*Cos(th)*Cos(ph);
	gluLookAt(Ex,Ey,Ez, Ox,Oy,Oz , 0,Cos(ph),0);

	glPushMatrix();
 
	glRotated(-90,1,0,0);

	//  Draw sky
	if (skySwitch) drawSky();

	//  Light switch
	if (light)
	{
		float F = (light==2) ? 1 : 0.3;
		//  Translate intensity to color vectors
		float Ambient[]   = {0.2*F,0.2*F,0.2*F,1.0};
		float Diffuse[]   = {0.5*F,0.5*F,0.5*F,1.0};
		float Specular[]  = {1.0*F,1.0*F,1.0*F,1.0};
		float yellow[] = {1.0,1.0,0.85,1.0};
		//  Light position
		float Position[]  = {AXES*Cos(zh),yl+750,AXES*Sin(zh),1.0};
		//  Draw light position as ball (still no lighting here)
		ball(Position[0],Position[1],Position[2], 5);
		//  Enbale normalization 
		glEnable(GL_NORMALIZE);
		//  Enable lighting
		glEnable(GL_LIGHTING);
		//  Two-sided lighting
      	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 1);
		//  glColor sets ambient and diffuse color materials
		glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
		glEnable(GL_COLOR_MATERIAL);
		//  Enable light 0
		glEnable(GL_LIGHT0);
		//  Set ambient, diffuse, specular components and position of light0
		glLightfv(GL_LIGHT0,GL_AMBIENT,Ambient);
		glLightfv(GL_LIGHT0,GL_DIFFUSE,Diffuse);
		glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
		glLightfv(GL_LIGHT0,GL_POSITION,Position);
		glMaterialf(GL_FRONT_AND_BACK,GL_SHININESS,32.0f);
		glMaterialfv(GL_FRONT_AND_BACK,GL_SPECULAR,yellow);
	}
	else
	{
		glDisable(GL_LIGHTING);
	}

	//  Enable textures
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	//  Draw scene 
	drawScene();

	//  Turn lighting and textures off
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glColor3f(1,1,1);
	if (axes)
	{
		glBegin(GL_LINES);
		glVertex3d(Ox,Oy,Oz);
		glVertex3d(AXES,Oy,Oz);
		glVertex3d(Ox,Oy,Oz);
		glVertex3d(Ox,AXES,Oz);
		glVertex3d(Ox,Oy,Oz);
		glVertex3d(Ox,Oy,AXES);
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

	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	SDL_GL_SwapBuffers();
}

int main(int argc, char *argv[])
{
	int run = 1;
	double t0 = 0;
	SDL_Surface* screen;

	//  Initialize SDL
	SDL_Init(SDL_INIT_VIDEO); 
	//  Set size, resizable and double buffering
	screen = SDL_SetVideoMode(600,600,0,SDL_OPENGL|SDL_RESIZABLE|SDL_DOUBLEBUF);
	if (!screen ) Fatal("Cannot set SDL video mode\n");
	//  Set window and icon labels
	SDL_WM_SetCaption("Steven Conflenti: CSCI 4229 Project", "Red Rocks Amphitheatre");
	//  Set screen size
	reshape(screen->w,screen->h);	
	
	//  Load textures 
	rock[0] = LoadTexBMP("brick.bmp");
	rock[1] = LoadTexBMP("concrete.bmp");
	rock[2] = LoadTexBMP("metal.bmp");
	rock[3] = LoadTexBMP("wood.bmp"); 
	rock[4] = LoadTexBMP("dj.bmp"); 
	rock[5] = LoadTexBMP("led.bmp"); 

	sky[0] = LoadTexBMP("blue.bmp");
	sky[1] = LoadTexBMP("star.bmp");
 
	//  Load objects
	left = LoadOBJ("left.obj");
	right = LoadOBJ("right.obj");
	tree = LoadOBJ("tree.obj");
	dome = LoadOBJ("dome.obj");
	spotlight = LoadOBJ("spotlight.obj");

	//  Create shader programs
	shader = CreateShaderProg("dome.vert","dome.frag");

	//  Initialize audio
	if (Mix_OpenAudio(44100,AUDIO_S16SYS,2,4096)) Fatal("Cannot initialize audio\n");
	for (int i = 0; i < SONGS; i++)
	{
		//  Load songs
		music[i] = Mix_LoadMUS(songs[i]);
		if (!music[i]) Fatal("Cannot load " + *songs[i]);
	}

	ErrCheck("init");
	while (run)
	{
		//  Elapsed time in seconds
		double t = SDL_GetTicks()/1000.0;
		//  Process all pending events
		SDL_Event event;
		while (SDL_PollEvent(&event)) 
		{
			switch (event.type)
			{
				case SDL_VIDEORESIZE:
					screen = SDL_SetVideoMode(event.resize.w,event.resize.h,0,SDL_OPENGL|SDL_RESIZABLE|SDL_DOUBLEBUF);
					reshape(screen->w,screen->h);
					break;
				case SDL_QUIT:
					run = 0;
					break;
				case SDL_KEYDOWN:
					run = key();
					t0 = t+0.5;  //  Wait .5s before repeating
					break;
				case SDL_MOUSEMOTION:
					if (mouse)
					{
						//  Left/right
						Ox += (X - (int) event.motion.x);
						//  Near/far or up/down
						if (mouse < 0)
						{
							Oy -= (Y - (int) event.motion.y);
						}
						else
						{
							Oz += (Y - (int) event.motion.y);
						}
						//  Remember coordinates
						X = (int) event.motion.x;
						Y = (int) event.motion.y;
					}
					break;
				case SDL_MOUSEBUTTONDOWN:
					mouse = (event.button.button == SDL_BUTTON_LEFT) ? 1 : -1;
					X = (int) event.button.x;
					Y = (int) event.button.y;
					break;
				case SDL_MOUSEBUTTONUP:
					mouse = 0;
					break;
				default:
					break;  //  Do nothing
			}
		}
		//  Repeat key every 50 ms
		if (t-t0>0.05)
		{
			run = key();
			t0 = t;
		}
		//  Display
		if (move) 
		{
			zh = fmod(90*t,360.0);
			Th = fmod(90*t,360.0);
		}
		display();
		//  Slow down display rate to about 100 fps by sleeping 5ms
		SDL_Delay(5);
	}
	//  Shut down SDL
	Mix_CloseAudio();
	SDL_Quit();
	return 0;
}