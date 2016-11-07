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
int ph = 0;        //  Elevation of view angle
int zh = 0;        //  Azimouth of light
int light = 1;     //  Lighting
double yl = 0.0;   //  Elevation of light
int move = 1;      //  Move light
double Ex = 1;     //  X-coordinate of eye
double Ey = 1;     //  Y-coordinate of eye
double Ez = 1;     //  Z-coordinate of eye
unsigned int rock[4];  //  Rock textures

SDL_Surface* screen;

//  Curved rectangular prism 
static void row(double x, double y, double z, double dx, double dy, double dz, double th)
{
	const double d = 2.5;  //  Degress per step
	const double slices = 180.0 / d;

	//  Save transformation
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	//glRotated(th,0,0,1);
	glScaled(dx,dy,dz);

	glColor3f(1.0,0.9,0.65);
	glBindTexture(GL_TEXTURE_2D, rock[1]);  //  Sandstone 128
	
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
	glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
	glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
	glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
	glEnd();
	//  Left
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
	glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
	glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
	glEnd();

	glBindTexture(GL_TEXTURE_2D, rock[3]);  //  Concrete     

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

static void stands(int num, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glRotated(th,0,0,1);

	for (int i = 0; i < num; i++)
	{ 
		row(x,y+(i*dy*2),z+(i*dz*2),dx,dy,dz,th);
	}
	//  Undo transformations
	glPopMatrix();
}

//  Cubeish
static void stair(double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformation
	glPushMatrix();
	//  Offset
	glTranslated(x,y,z);
	//glRotated(th,0,0,1);
	glScaled(dx,dy,dz);

	glColor3f(1.0,0.45,0.0);
	glBindTexture(GL_TEXTURE_2D, rock[3]);  //  Concrete

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

	glBindTexture(GL_TEXTURE_2D, rock[2]);  //  Sandstone 256

	//  Right
	glBegin(GL_QUADS);
	glNormal3f(+1, 0, 0);
	glTexCoord2f(0,0); glVertex3f(+1,-1,+1);
	glTexCoord2f(1,0); glVertex3f(+1,-1,-1);
	glTexCoord2f(1,1); glVertex3f(+1,+1,-1);
	glTexCoord2f(0,1); glVertex3f(+1,+1,+1);
	glEnd();
	//  Left
	glBegin(GL_QUADS);
	glNormal3f(-1, 0, 0);
	glTexCoord2f(0,0); glVertex3f(-1,-1,-1);
	glTexCoord2f(1,0); glVertex3f(-1,-1,+1);
	glTexCoord2f(1,1); glVertex3f(-1,+1,+1);
	glTexCoord2f(0,1); glVertex3f(-1,+1,-1);
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
		stair(x,y+(i*dy),z+(i*dz*2),dx,dy,dz,th);
	}
	//  Undo transformations
	glPopMatrix();
}
static void bigStairs(int num, double x, double y, double z, double dx, double dy, double dz, double th)
{
	//  Save transformations
	glPushMatrix();
	//  Offset
	glRotated(th,0,0,1);

	for (int i = 0; i < num; i++)
	{ 
		stair(x,y+(i*dy*4),z+(i*dz*4),dx,dy,dz,th);
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
	else if (keys[SDLK_0])
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
		th = ph = 0;
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
	stairs(120, -155,0,-.5, 5,5,.5, 0);
	stairs(120, 155,0,-.5, 5,5,.5, 0);
	bigStairs(15, -170,5,1, 10,10,2, 0);
	bigStairs(15, 170,5,1, 10,10,2, 0);
	stands(60, 0,0,0, 150,5,1, 0);

	//  Turn lighting and textures off
	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

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
	rock[0] = LoadTexBMP("brick.bmp");
	rock[1] = LoadTexBMP("sandstone.bmp");
	rock[2] = LoadTexBMP("sandstone2.bmp");
	rock[3] = LoadTexBMP("concrete.bmp");

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
					t0 = t+0.5;  //  Wait 1/2 s before repeating
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