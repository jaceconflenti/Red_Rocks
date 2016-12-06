/*
# Red Rocks Amphitheatre 
## Steven Jace Conflenti
### CSCI 4229 Fall 16

## Run with make clean; make; ./redrocks 

## The best way to view the program is to let the camera cycle on it's own (every four seconds). You can also stop the automatic camera movement with the '`' (tilde) key and view the scene manually. Click and drag with the left mouse button to move in the X-Z plane, or click and drag with the right mouse button to move in the X-Y plane. You can use the arrow keys to further rotate the camera; however, note that the left and right keys are sensitive.

### Key bindings:
 * ESC		Exit
 * Spacebar Pause music
 * n/m  	Previous/Next song
 * a        Toggle performance mode
 * s 		Toggle sky
 * d 		Toggle light movement
 * f 		Cycle light intensity
 * [/] 		Move light
 * -/+ 		Change light elevation
 * `    	Toggles automatic camera cycling
 * 1		Viewing angle 1
 * 2 		Viewing angle 2
 * 3 		Viewing angle 3
 * 4 		Viewing angle 4
 * 5 		Viewing angle 5
 * 6 		Viewing angle 6
 * 7/8		Decrease/Increase fov
 * 9/0 		Decrease/Increase dim

### Things I Spent Lots of Time On
* Skydome - I spent way too much time trying to create a far too complex sky. Eventually I simplified it, but even the simplified version took me a long time to figure out.
* Stands - I created all elements of the stands by hand. They are very detailed (curved, complex, textured, etc.)
* Rock formations - Modeled them by hand in blender.
* Performance issues - Now the only thing that really saps performance are the trees (also have performance mode).
* Hand creating other objects - Almost everything you see in the scene (besides the spotlights and trees) I created by hand.
* Converting to and getting things working with the right version of SDL - luckily I did this relatively early on or it really would have been painful.
*/

#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "CSCIx229.h"

#define AXES 1500.0     //  Length of axes
#define LEN 8192        //  Max length of text string 
#define SONGS 4			//  Number of songs -1 (start with song 0)
const char *songs[SONGS+1] = {"Breakthrough.mp3","Why_Not.mp3","Renegade.mp3","Transformation.mp3","The_Unborn_Dancing.mp3"};  
Mix_Music* music[SONGS+1];  //  Songs legally downloaded from freemusicarchive.org
int play = 1;			//  Play music
int track = 0;          //  Music track
double dim = AXES;      //  Size of world
int fov = 60;           //  Field of view 
double asp = 1;         //  Aspect ratio
int th = 190;           //  Azimuth of view angle
int ph = 5;             //  Elevation of view angle
int zh = 0;             //  Azimouth of light
int Th,Ph,Zh,Th1,Ph1,Zh1 = 0;  //  Azimuths of spotlights
int light = 1;          //  Lighting
double yl = 0.0;        //  Elevation of light
int move = 1;           //  Move light
int cycle = 1;          //  Cycle camera angles
double Ex = 1;          //  X-coordinate of eye
double Ey = 1;          //  Y-coordinate of eye
double Ez = 1;          //  Z-coordinate of eye
double Ox = -350;	    //  Look-at x
double Oy = 20;	        //  Look-at y
double Oz = 2000;	    //  Look-at z
int X,Y;                //  Last mouse coordinates
int mouse = 0;          //  Move mode   
unsigned int material[7];  //  Rock/surface textures
unsigned int sky[4];    //  Sky textures
int shader = 0;         //  Shader program
int skySwitch = 1;      //  Sky switch
int quality = 1;    	//  0 = Better performance mode
float mix1 = 0.0;  		//  Uniform ratio of day sky : night sky
float amb = 1.25;       //  Influences ambient light
int left,right,tree,dome,spotlight;  //  Belnder object display lists

//  Base plane for scene
static void drawGround(double x, double y, double z, double th)
{	
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);

	glRotated(th,1,0,0);

	double vert = AXES * 1.5;

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
	//glBindTexture(GL_TEXTURE_2D, material[6]);  //  Stone

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

//  Geodesic dome
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

static void cylinder(const int mode, const int _stacks, double x, double y, double z, double r, double h)
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

	
	if (mode == 1) 
	{
		glColor3d(1,0,0);							//  Red
		glBindTexture(GL_TEXTURE_2D, material[6]);  //  Laser
	}
	else if (mode == 2) 
	{
		glColor3d(0,1,0);							//  Green					
		glBindTexture(GL_TEXTURE_2D, material[6]);  //  Laser
	}
	else if (mode == 3) 
	{
		glColor3d(0,0,1);							//  Blue
		glBindTexture(GL_TEXTURE_2D, material[6]);  //  Laser
	}
	else if (mode == 4) 
	{
		glColor3d(1,1,0);							//  Yellow
		glBindTexture(GL_TEXTURE_2D, material[6]);  //  Laser
	}
	else if (mode == 5) 
	{
		glColor3d(1,0,1);							//  Violet
		glBindTexture(GL_TEXTURE_2D, material[6]);  //  Laser
	}
	else if (mode == 6) 
	{
		glColor3d(0,1,1);							//  Cyan
		glBindTexture(GL_TEXTURE_2D, material[6]);  //  Laser
	}
	else 
	{
		glColor3d(1,0,0);  							//  Red
		glBindTexture(GL_TEXTURE_2D, material[2]);  //  Metal
	}

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
		glBindTexture(GL_TEXTURE_2D, material[3]);  //  Wood
	}
	else
	{
		glColor3f(1.0,0.86,0.73);  //  Peach
		glBindTexture(GL_TEXTURE_2D, material[0]);  //  Brick
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
		glBindTexture(GL_TEXTURE_2D, material[3]);  //  Wood
	}
	else 
	{
		glBindTexture(GL_TEXTURE_2D, material[1]);  //  Concrete   
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
static void drawSpotlight(double x, double y, double z, double dx, double dy, double dz)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glScaled(dx,dy,dz);

	glColor3f(0.31,0.33,0.35);
	glCallList(spotlight);

	//  Undo transformations
	glPopMatrix();
}

//  color: 1 = red, 2 = green, 3 = blue, 4 = yellow, 5 = violet, 6 = cyan
static void drawLaser(const int color, double x, double y, double z, double r, double h, double _th, double _ph)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glRotated(_th,1,0,0);
	glRotated(_ph,0,1,0);

	cylinder(color, (quality) ? r:10, 0,0,0, r,h);

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
		glBindTexture(GL_TEXTURE_2D, material[2]);  //  Metal
	}
	else if (mode == 3)
	{
		glColor3f(0.73,0.73,0.73);
		glBindTexture(GL_TEXTURE_2D, material[1]);
	}
	else if (mode == 4)
	{
		glBindTexture(GL_TEXTURE_2D, material[4]);  //  DJ
	}
	else 
	{
		glColor3f(1.0,0.99,0.73);
		glBindTexture(GL_TEXTURE_2D, material[1]);  //  Concrete
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
		glBindTexture(GL_TEXTURE_2D, material[0]);  //  Brick 
	}
	else if (mode == 4)
	{
		glBindTexture(GL_TEXTURE_2D, material[5]); //  LED
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
		if (((i%5) == 0) && (i != 0) && quality)
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

static void stage(double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	glRotated(th,0,0,1); 
	glScaled(dx,dy,dz);

	//  Spotlights
	drawSpotlight(-120,-25,45, 5,5,5); 
	drawSpotlight(-80,-25,50, 5,5,5);
	drawSpotlight(-40,-25,50, 5,5,5);
	drawSpotlight(0,-25,50, 5,5,5);
	drawSpotlight(40,-25,50, 5,5,5);
	drawSpotlight(80,-25,50, 5,5,5);
	drawSpotlight(120,-25,45, 5,5,5);

	//  Lasers
	drawLaser(2, -120,-15,43.5, 1.5,2000, Zh1,Th+90); drawLaser(3, -120,-42,43.5, 1.5,2000, Ph,Ph1+90); 
	drawLaser(4, -80,-15,48.5, 1.5,2000, Th1,Th1+90); drawLaser(5, -80,-42,48.5, 1.5,2000, Zh1,Th1+90);
	drawLaser(6, -40,-15,48.5, 1.5,2000, Ph,Ph+90); drawLaser(1, -40,-42,48.5, 1.5,2000, Zh1,Zh1+90); 
	drawLaser(5, 0,-15,48.5, 1.5,2000, Th1,Ph1+90); drawLaser(6, 0,-42,48.5, 1.5,2000, Ph,Th1+90);
	drawLaser(5, 40,-15,48.5, 1.5,2000, Ph1,Ph1+90); drawLaser(2, 40,-42,48.5, 1.5,2000, Zh1,Zh1+90); 
	drawLaser(6, 80,-15,48.5, 1.5,2000, Th1,Th1+90); drawLaser(4, 80,-42,48.5, 1.5,2000, Zh,Ph+90); 
	drawLaser(1, 120,-15,43.5, 1.5,2000, Ph1,Th+90); drawLaser(3, 120,-42,43.5, 1.5,2000, Zh,Th+90);

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
	cylinder(0, 12, 140,-55,0, 2,60);
	cylinder(0, 12, 140,-85,0, 2,60);
	cylinder(0, 12, 110,-55,0, 2,60);
	cylinder(0, 12, 110,-85,0, 2,60);
	cylinder(0, 12, 110,-145,0, 2,60);
	cylinder(0, 12, 110,-200,0, 2,60);
	
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
	cylinder(0, 12, -140,-55,0, 2,60);
	cylinder(0, 12, -140,-85,0, 2,60);
	cylinder(0, 12, -110,-55,0, 2,60);
	cylinder(0, 12, -110,-85,0, 2,60);
	cylinder(0, 12, -110,-145,0, 2,60);
	cylinder(0, 12, -110,-200,0, 2,60);

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

static void sun(double x,double y,double z,double r)
{
	//  Save transformation
	glPushMatrix();
	//  Offset, scale 
	glTranslated(x,y,z);

	GLUquadric *qobj = gluNewQuadric(); 
	gluQuadricTexture(qobj,GL_TRUE);
	glBindTexture(GL_TEXTURE_2D,sky[3]);
	gluSphere(qobj,r,25,25);
	gluDeleteQuadric(qobj); 

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
	int id1 = glGetUniformLocation(shader, "star");
	int id2 = glGetUniformLocation(shader, "cloud");
	float id3 = glGetUniformLocation(shader, "mix1");
	
	if (id >= 0) glUniform1i(id, 0);
	if (id1 >= 0) glUniform1i(id1, 1);
	if (id2 >= 0) glUniform1i(id2, 2);
	if (id3 >= 0) glUniform1f(id3, mix1);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sky[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sky[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, sky[2]);

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

int key(const int num)
{
	Uint8* keys = SDL_GetKeyState(NULL);
	int shift = SDL_GetModState()&KMOD_SHIFT;

	//  Exit on ESC
	if (keys[SDLK_ESCAPE])
	{
		return 0;
	}	
	//  Toggles performance mode
	else if (keys[SDLK_a])
	{
		quality = 1 - quality;
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
	//  Toggles camera cycling
	else if (keys[SDLK_BACKQUOTE])
	{
		cycle = 1 - cycle;
	}
	//  Viewing angle 1
	else if (keys[SDLK_1] || num == 1)
	{
		th = 190;           
		ph = 5;
		Ox = -350;	   
		Oy = 20;	     
		Oz = 2000;
	}
	//  Viewing angle 2
	else if (keys[SDLK_2] || num == 2)
	{
		th = -5;           
		ph = 5;
		Ox = 0;	   
		Oy = 0;	     
		Oz = -2500;
	}
	//  Viewing angle 3
	else if (keys[SDLK_3] || num == 3)
	{
	 	th = 180;           
		ph = 20;
		Ox = 150;	   
		Oy = 0;	     
		Oz = 975;
	}
	//  Viewing angle 4
	else if (keys[SDLK_4] || num == 4)
	{
		th = 15;           
		ph = 5;
		Ox = 350;	   
		Oy = 20;	     
		Oz = -2250;
	}
	//  Viewing angle 5
	else if (keys[SDLK_5] || num == 5)
	{
		th = -165;           
		ph = 15;
		Ox = 0;	   
		Oy = 0;	     
		Oz = 1250;	
	}
	//  Viewing angle 6
	else if (keys[SDLK_6] || num == 6)
	{
		th = 180;           
		ph = 10;
		Ox = -100;	   
		Oy = -200;	     
		Oz = 2500;
	}
	//  Decreases fov
	else if (keys[SDLK_7])
	{
		fov--;
	}
	//  Increase fov
	else if (keys[SDLK_8])
	{
		fov++;
	}
	//  Decrease dim
	else if (keys[SDLK_9]) 
	{
		dim -= 5.0;
	}
	//  Increase dim
	else if (keys[SDLK_0]) 
	{
		dim += 5.0;
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
		else
		{
			Mix_HaltMusic();
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
		if (ph > 20) ph = 20;
	}
	//  Down arrow key - decrease elevation by 5 degrees
	else if (keys[SDLK_DOWN]) 
	{
		ph -= 5;
		if (ph < 5) ph = 5;
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
		float Ambient[]   = {0.2*F*amb,0.2*F*amb,0.2*F*amb,1.0};
		float Diffuse[]   = {0.5*F,0.5*F,0.5*F,1.0};
		float Specular[]  = {1.0*F,1.0*F,1.0*F,1.0};
		float yellow[] = {1.0,1.0,0.85,1.0};
		//  Light position
		float Position[]  = {0.8*AXES*Cos(zh),yl+750,0.8*AXES*Sin(zh),1.0};
		//  Draw light position as sun (still no lighting here)
		sun(Position[0],Position[1],Position[2], 30);
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

	glPopMatrix();

	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	SDL_GL_SwapBuffers();
}

int main(int argc, char *argv[])
{
	int run = 1; 
	int cameraAngle = 1;
	double time0 = 0;
	double tKey0 = 0;
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
	material[0] = LoadTexBMP("brick.bmp");
	material[1] = LoadTexBMP("concrete.bmp");
	material[2] = LoadTexBMP("metal.bmp");
	material[3] = LoadTexBMP("wood.bmp"); 
	material[4] = LoadTexBMP("dj.bmp"); 
	material[5] = LoadTexBMP("led.bmp");
	material[6] = LoadTexBMP("laser.bmp"); 
	sky[0] = LoadTexBMP("sky.bmp");
	sky[1] = LoadTexBMP("bigstar.bmp");
	sky[2] = LoadTexBMP("cloud.bmp");
	sky[3] = LoadTexBMP("sun.bmp");
	
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
	for (int i = 0; i <= SONGS; i++)
	{
		//  Load songs
		music[i] = Mix_LoadMUS(songs[i]);
		if (!music[i]) Fatal("Cannot load " + *songs[i]);
	}
	//  Play current track
	Mix_PlayMusic(music[track],-1);

	ErrCheck("init");
	while (run)
	{
		//  Elapsed time in seconds
		double tSlow = SDL_GetTicks()/10000.0;
		double time = SDL_GetTicks()/1000.0;
		double tKey = SDL_GetTicks()/1000.0;
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
					run = key(0);
					tKey0 = tKey+0.5;  //  Wait .5s before repeating
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
		if (tKey-tKey0>0.05)
		{
			run = key(0);
			tKey0 = tKey;
		}
		//  Cycle camera angle every 4 seconds
		if(time-time0>4.0 && cycle)
		{
			cameraAngle++;
			cameraAngle %= 6;
			run = key(cameraAngle);
			time0 = time;
		}
		//  Display
		if (move) 
		{
			//  Slow the sky cycle time
			zh = fmod(90*tSlow,360.0);
			float percent = (float) zh / 360.0;
			//  Set the mix ratio
			if (percent < 0.5) mix1 = 0.01;
			else mix1 = 0.95;
			if (percent > 0.95 || percent < 0.05) mix1 = 0.5;
			if ((percent > 0.45 && percent < 0.55)) mix1 = 0.5;
			//  Adjust ambient light
			amb = 1.25 - percent;
			if (amb < 0.5) amb = 0.2;
			if (amb > 1.0) amb = 1.0;

			//  Laser angles
			Th = fmod((90*time),360.0);
			Ph = fmod((90*time)+30,360.0);
			Zh = fmod((90*time)+60,360.0);
			Th1 = fmod((90*time),180.0);
			Ph1 = fmod((90*time)+45,180.0);
			Zh1 = fmod((90*time)+90,180.0);
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