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

#include "CSCIx229.h"  //  Doesn't include Print (glut dependency), use printF() instead

#define AXES 100.0 //  Length of axes
#define LD AXES*2  //  Radius of sun from origin
#define LEN 8192   //  Max length of text string

int fov = 55;      //  Field of view 
double asp = 1;    //  Aspect ratio
double dim = AXES; //  'Radius' of world
int axes = 1;      //  Display axes
int th = 0;        //  Azimuth of view angle
int ph = 15;       //  Elevation of view angle
int zh = 0;        //  Azimouth of light
int light = 1;     //  Lighting
double yl = 0.0;   //  Elevation of light
int move = 1;      //  Move light
double Ex = 1;     //  X-coordinate of eye
double Ey = 1;     //  Y-coordinate of eye
double Ez = 1;     //  Z-coordinate of eye
double Ox = 0;	   //  Look-at x
double Oy = 0;	   //  Look-at y
double Oz = 0;	   //  Look-at z
int X,Y;           //  Last mouse coordinates
int mouse = 0;      //  Move mode    
unsigned int rock[6];  //  Rock textures

SDL_Surface* screen;

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
		row(0,x,y+(i*dy*2),z+(i*dz*2),dx,dy,dz,th);
		row(1,x,y+(i*dy*2)-2.65,z+(i*dz*2)+.8,dx-.02,dy/4,dz/4,th);
	}
	//  Undo transformations
	glPopMatrix();
}

//  mode: 1 = brick, 2 = metal, 3 = concrete, 4 = DJ booth, else concrete
static void cube(const int mode, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformation
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	//glRotated(th,0,0,1);
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
		cube(0,x,y+(i*dy),z+(i*dz*2),dx,dy,dz,th);
	}
	//  Undo transformations
	glPopMatrix();
}
static void pathEdge(int num, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glRotated(th,0,0,1);

	for (int i = 0; i < num; i++)
	{ 
		cube(1,x,y+(i*dy*4),z+(i*dz*4),dx,dy,dz,th);
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
	//glRotated(ph,0,1,0);

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
	for (i=0; i<=slices; i++)  //  For loop runs opposite way to allow for face culling
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
	glRotated(th,0,0,1);
	glScaled(dx,dy,dz);

	//  Main stage floor
	for (int i = 0; i < 25; i++)
	{
		int x = i * 8;
		for (int j = 0; j < 25; j++)
		{
			int y = j * 8;
			cube(3, -95+x,-220+y,0, 4,4,2, 0);
		}
	}

	//  DJ Booth
	cube(4, 0,-60,7, 25,12.5,8, 0);

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
			cube(2, 100+x,-220+y,60, 3,3,3, 0);
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
			cube(2, -100-x,-220+y,60, 3,3,3, 0);
		}
	}

	//  Main stage roof
	for (int i = 0; i < 25; i++)
	{
		int x = i * 8;
		for (int j = 0; j < 25; j++)
		{
			int y = j * 8;
			cube(2, -95+x,-220+y,68, 4,4,5, 0);
		}
	}

	//  Undo transformations
	glPopMatrix();
}

//  Print the designated string at the specified coordinates
void printF(int x, int y, TTF_Font* font, const char *c, ...)
{
	char    buf[LEN];
	char*   ch=buf;
	va_list args;
	//  Turn the parameters into a character string
	va_start(args,c);
	vsnprintf(buf,LEN,c,args);
	va_end(args);

	//  Set color to white
	SDL_Color fColor;
	fColor.r = 255;
	fColor.g = 255;
	fColor.b = 255;

	SDL_Surface* fontSurface = TTF_RenderText_Solid(font, ch, fColor);
	SDL_Rect fontRect;
	fontRect.x = x;
	fontRect.y = y;

	SDL_BlitSurface(fontSurface, NULL, screen, &fontRect);
	SDL_Flip(screen);
}

static void Vertex(int th, int ph, const int tex)
{
	double x = -Sin(th)*Cos(ph);
	double y =  Cos(th)*Cos(ph);
	double z =          Sin(ph);
	glNormal3d(x,y,z);
	if (tex)
	{
		glTexCoord2d(th/360.0,ph/180.0+0.5);
	}
	glVertex3d(x,y,z);
}

static void ball(double x, double y, double z, double r)
{
	int th,ph;
	//  Save transformation
	glPushMatrix();
	//  Offset, scale and rotate
	glTranslated(x,y,z);
	glScaled(r,r,r);
	//  Bands of latitude
	glColor3f(1,1,0);
	for (ph=-90;ph<90;ph+=10)
	{
		glBegin(GL_QUAD_STRIP);
		for (th=0;th<=360;th+=20)
		{
			Vertex(th,ph,0);
			Vertex(th,ph+10,0);
		}
		glEnd();
	}
	//  Undo transofrmations
	glPopMatrix();
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
	//  Toggles the axes
	else if (keys[SDLK_a])
	{
		axes = 1 - axes;
	}
	//  Toggles light movement
	else if (keys[SDLK_s])
	{
		move = 1 - move;
	}
	//  Cycles light
	else if (keys[SDLK_l])
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
	//  Reset viewing angle
	else if (keys[SDLK_0])
	{
		th = Ox = Oy = Oz = 0;
		ph = 15;
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
	else if (keys[SDLK_3]) {
		dim -= 5.0;
	}
	//  Decrease dim
	else if (keys[SDLK_4]) {
		dim += 5.0;
	}
	//  Right arrow key - increase angle by 5 degrees
	else if (keys[SDLK_RIGHT]) {
		th += 5;
	}
	//  Left arrow key - decrease angle by 5 degrees
	else if (keys[SDLK_LEFT]) {
		th -= 5;
	}
	//  Up arrow key - increase elevation by 5 degrees
	else if (keys[SDLK_UP]) {
		ph += 5;
	}
	//  Down arrow key - decrease elevation by 5 degrees
	else if (keys[SDLK_DOWN]) {
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
		//  Set ambient, diffuse, specular components and position of light0
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

	//  Enable textures
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);

	//  Draw scene
	stage(0,0,0, 1,1,1, 0);
	stairs(240, -155,0,-.5, 5,5,.5, 0);
	stairs(240, 155,0,-.5, 5,5,.5, 0);
	pathEdge(30, -170,5,1, 10,10,2, 0);
	pathEdge(30, 170,5,1, 10,10,2, 0);
	stands(120, 0,0,0, 150,5,1, 0);
	

	//  Turn lighting and textures off
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	//  Draw axes
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
		/*//  Label axes
		glRasterPos3d(AXES,0.0,0.0);
		//Print("X");
		glRasterPos3d(0.0,AXES,0.0);
		//Print("Y");
		glRasterPos3d(0.0,0.0,AXES);
		//Print("Z");*/
	}

	glPopMatrix();

	/*//  Initialize font
	TTF_Init();
	TTF_Font* font = TTF_OpenFont("arial.ttf", 12);
	
	printF(5, 5, font, "hello world");
	//  Display parameters
	glWindowPos2i(5,5);
	//Print("Angle=%d,%d  Dim=%.1f  FOV=%d  Light=%s",th,ph,dim,fov,light?"On":"Off");
	if (light)
	{
		//Print("  Light=%d,%.1f",zh,yl);
	}*/

	//  Render the scene and make it visible
	ErrCheck("display");
	glFlush();
	SDL_GL_SwapBuffers();
}

int main(int argc, char *argv[])
{
	int run = 1;
	double t0 = 0;
	Mix_Music* music;

	//  Initialize SDL
	SDL_Init(SDL_INIT_VIDEO);
	//  Set size, resizable and double buffering
	screen = SDL_SetVideoMode(1280,720,0,SDL_OPENGL|SDL_RESIZABLE|SDL_DOUBLEBUF);
	if (!screen ) Fatal("Cannot set SDL video mode\n");
	//  Set window and icon labels
	SDL_WM_SetCaption("Steven Conflenti: CSCI 4229 Project", "Conflenti");
	//  Set screen size
	reshape(screen->w,screen->h);

	
	//  Load textures
	rock[0] = LoadTexBMP("brick2.bmp");
	rock[1] = LoadTexBMP("concrete2.bmp");
	rock[2] = LoadTexBMP("metal.bmp");
	rock[3] = LoadTexBMP("wood.bmp");
	rock[4] = LoadTexBMP("dj.bmp");
	rock[5] = LoadTexBMP("led.bmp");
	
	/*  Initialize audio
	if (Mix_OpenAudio(44100,AUDIO_S16SYS,2,4096)) Fatal("Cannot initialize audio\n");
	//  Load "The Wall"
	music = Mix_LoadMUS("thewall.ogg");
	if (!music) Fatal("Cannot load thewall.ogg\n");
	//  Play (looping)
	Mix_PlayMusic(music,-1);
	*/

	//  SDL event loop
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