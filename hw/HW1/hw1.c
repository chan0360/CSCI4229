/*
 *  Homework 1 - Lorenz Attractor
 *  Greg McQuie
 *  DUE: 6/7/2014
 *  CSCI 4229 - Computer Graphics
 *  Description: Visualizes a Lorenz system in 3 dimensions from time=0 to time=50.
 *  Controls :  Left Arrow   -  Rotate left
 *		Right Arrow  -  Rotate right
 *		Up Arrow     -  Rotate up
 *		Down Arrow   -  Rotate down
 *		s            -  Decrease Lorenz parameter sigma
 *		<SHIFT>-S    -  Increase Lorenz parameter sigma
 *		b            -  Decrease Lorenz parameter beta
 *		<SHIFT>-B    -  Increase Lorenz parameter beta
 *		r            -  Decrease Lorenz parameter rho
 *		<SHIFT>-R    -  Increase Lorenz parameter rho
 */

// Fixes warning: implicit declaration of function ‘glWindowPos2i’
#define GL_GLEXT_PROTOTYPES

// Include GLUT libraries on any OS
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

// Other necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

//  Globals
int th=0;            // Azimuth of view angle
int ph=0;            // Elevation of view angle
double z=0;          // Z variable
double w=1;          // W variable
double dim=2;        // Dimension of orthogonal box
double s  = 10;      // Lorenz Parameter - Sigma
double b  = 2.6666;  // Lorenz Parameter - Beta
double r  = 28;      // Lorenz Parameter - Rho


/*
 *  Convenience routine to output raster text
 *  Use VARARGS to make this more flexible.
 */
#define LEN 8192  // Maximum length of text string
void Print(const char* format , ...)
{
   char    buf[LEN];
   char*   ch=buf;
   va_list args;
   //  Turn the parameters into a character string
   va_start(args,format);
   vsnprintf(buf,LEN,format,args);
   va_end(args);
   //  Display the characters one at a time at the current raster position
   while (*ch)
      glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,*ch++);
}


/*
 * This function is called by GLUT to display the scene
 */
void display()
{
   //  Clear screen
   glClear(GL_COLOR_BUFFER_BIT);
   //  Reset previous transforms
   glLoadIdentity();
   //  Set view angle
   glRotated(ph,1,0,0);
   glRotated(th,0,1,0);

   //  Begin line
   glBegin(GL_LINE_STRIP);

   //  Draws scene by looping through Lorenz integrator
   int i;  // Counter variable
   //  Initial position
   double x = 1;
   double y = 1;
   double z = 1;
   //  Timestep
   double dt = 0.001;
   //  Set line color to green
   glColor3f(0.0,1.0,0.0);
   //  Draw first point. Needs to be scaled to be within viewing window
   glVertex3d(x/50,y/50,z/50);
   /*
    *  Integrate 50,000 steps (50 time units with dt = 0.001)
    *  Explicit Euler integration
    */
   for (i=0;i<50000;i++)
   {
      // Integration
      double dx = s*(y-x);
      double dy = x*(r-z)-y;
      double dz = x*y - b*z;
      x += dt*dx;
      y += dt*dy;
      z += dt*dz;
      // Draws next point in curve
      glVertex3d(x/50,y/50,z/50); 
   }
   glEnd(); // End drawing

   //  Draw axes
   glColor3f(1,1,1); // Set color to white
   glBegin(GL_LINES);// Begin
   glVertex3d(0,0,0);
   glVertex3d(1,0,0);
   glVertex3d(0,0,0);
   glVertex3d(0,1,0);
   glVertex3d(0,0,0);
   glVertex3d(0,0,1);
   glEnd();
   //  Label axes
   glRasterPos3d(1,0,0);
   Print("X");
   glRasterPos3d(0,1,0);
   Print("Y");
   glRasterPos3d(0,0,1);
   Print("Z");
   //  Display view parameters
   glWindowPos2i(5,5);
   Print("View Angle=%d,%d",th,ph);
   //  Flush scene
   glFlush();
  //  Make scene visible
   glFlush();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase azimuth by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease azimuth by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   double w2h = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Tell OpenGL we want to manipulate the projection matrix
   glMatrixMode(GL_PROJECTION);
   //  Undo previous transformations
   glLoadIdentity();
   //  Orthogonal projection box adjusted for the
   //  aspect ratio of the window
   glOrtho(-dim*w2h,+dim*w2h, -dim,+dim, -dim,+dim);
   //  Switch to manipulating the model matrix
   glMatrixMode(GL_MODELVIEW);
   //  Undo previous transformations
   glLoadIdentity();
}

/*
 *  GLUT calls this routine when a key is pressed
 */
void key(unsigned char ch,int x,int y)
{
   //  Exit on ESC
   if (ch == 27)
      exit(0);
   //  Reset view angle
   else if (ch == '0')
      th = ph = 0;
   //  Increase or decrease Lorenz parameters S,B,R
   //  Capital characters incrase, lowercase decrease
   else if (ch=='s')
      s = s-1; // decrease sigma
   else if (ch=='S')
      s = s+1; // increase sigma
   else if (ch=='b')
      b=b-0.5; // decrease beeta
   else if (ch=='B')
      b=b+0.5; // increase beta
   else if (ch=='r')
      r=r-1;   // decrease rho
   else if (ch=='R')
      r=r+1;   // increase rho
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 * Lorenz Attractor Main
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered true color window
   glutInitDisplayMode(GLUT_RGB);
   //  Request 500 x 500 pixel window
   glutInitWindowSize(500,500);
   //  Create window
   glutCreateWindow("Lorenz Attractor");
   //  Register function used to display scene
   glutDisplayFunc(display);
  //  Tell GLUT to call "reshape" when the window is resized
   glutReshapeFunc(reshape);
   //  Tell GLUT to call "special" when an arrow key is pressed
   glutSpecialFunc(special);
   //  Tell GLUT to call "key" when a key is pressed
   glutKeyboardFunc(key);
   //  Pass control to GLUT for events
   glutMainLoop();
   //  Return to OS
   return 0;
}

