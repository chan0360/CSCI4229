#include <math.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <sys/timeb.h>
#include "CSCIx229.h"

// INCLUDES AND STUFF

#define WINDOW_TITLE "Water Simulation"
#define FPS_TIMER 1
#define FPS_INTERVAL 500
#define RAIN_TIMER 2
#define GRID_SIZE 100


// Other Variables
unsigned int base;
int DrawWedge;
int ShowHelp;
int RainInterval;
float Viscosity;

float Position [GRID_SIZE][GRID_SIZE];
float Velocity [GRID_SIZE][GRID_SIZE];

typedef struct{
     float x;
     float y;
     float z;
} coordinate;

coordinate Vertex [GRID_SIZE][GRID_SIZE];
coordinate Normals [GRID_SIZE][GRID_SIZE];

float xAngle;
float yAngle;

//misc interaction variables
int mode=1;
int th=0;
int ph=0;
int light=0;
int rep=1;
double asp=1;
double dim=3.0;
int roll=1;
float alpha =.5;

//light values
int emission =0;
int ambient =30;
int diffuse=100;
int specular=0;
int shininess=0;
float shinyvec[1];
float sinyvec[2];
int zh=90;
float ylight=0;
unsigned int texture[7];

static void ball(double x, double y, double z, double r){
     glPushMatrix();
     glTranslated(x,y,z);
     glScaled(r,r,r);
     glColor3f(1,1,1);
     glutSolidSphere(1,16,16);
     glPopMatrix();

}
/*
 *  GLUT calls this routine when the window is resized
 */
void idle()
{
   //  Elapsed time in seconds
   double t = glutGet(GLUT_ELAPSED_TIME)/1000.0;
   zh = fmod(90*t,360.0);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when an arrow key is pressed
 */
void special(int key,int x,int y)
{
   //  Right arrow key - increase angle by 5 degrees
   if (key == GLUT_KEY_RIGHT)
      th += 5;
   //  Left arrow key - decrease angle by 5 degrees
   else if (key == GLUT_KEY_LEFT)
      th -= 5;
   //  Up arrow key - increase elevation by 5 degrees
   else if (key == GLUT_KEY_UP)
      ph += 5;
   //  Down arrow key - decrease elevation by 5 degrees
   else if (key == GLUT_KEY_DOWN)
      ph -= 5;
   //  PageUp key - increase dim
   else if (key == GLUT_KEY_PAGE_DOWN)
      dim += 0.1;
   //  PageDown key - decrease dim
   else if (key == GLUT_KEY_PAGE_UP && dim>1)
      dim -= 0.1;
   //  Keep angles to +/-360 degrees
   th %= 360;
   ph %= 360;
   //  Update projection
   Project(45,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}

/*
 *  GLUT calls this routine when the window is resized
 */
void reshape(int width,int height)
{
   //  Ratio of the width to the height of the window
   asp = (height>0) ? (double)width/height : 1;
   //  Set the viewport to the entire window
   glViewport(0,0, width,height);
   //  Set projection
   Project(45,asp,dim);
}




void CalcSurfaceNormal(
        double      x1,
        double      y1,
        double      z1,
        double      x2,
        double      y2,
        double      z2,
        double      x3,
        double      y3,
        double      z3,
        double*     pNormalX,
        double*     pNormalY,
        double*     pNormalZ
    ){
    double  d1x;
    double  d1y;
    double  d1z;
    double  d2x;
    double  d2y;
    double  d2z;
    double  crossx;
    double  crossy;
    double  crossz;
    double  dist;

    d1x = x2 - x1;
    d1y = y2 - y1;
    d1z = z2 - z1;

    d2x = x3 - x2;
    d2y = y3 - y2;
    d2z = z3 - z3;

    crossx = d1y*d2z - d1z*d2y;
    crossy = d1y*d2x - d1x*d2z;
    crossz = d1x*d2y - d1y*d2x;

    dist = sqrt( crossx*crossx + crossy*crossy + crossz*crossz );

    *pNormalX = crossx / dist;
    *pNormalY = crossy / dist;
    *pNormalZ = crossz / dist;
}

void CalcWater(){
     int i,j;
     float VectLength;
     float Speed;

     Speed = .75;//Higher speeds give longer ripples

     //Calculate the new velocity
     for(i=2;i<GRID_SIZE-2;i++){
          for(j=2;j<GRID_SIZE-2;j++){
               Velocity[i][j] = Velocity[i][j] + (Position[i][j] - (4.0*(Position[i-1][j] + Position[i+1][j] + Position[i][j-1] + Position[i][j+1]) +  /* left, right, above, below*/  Position[i-1][j-1] + Position[i+1][j-1] + Position[i-1][j+1] + Position[i+1][j+1])/25.0) / 7.0;  // diagonally across
          }
     }

     // Calculate the new ripple positions
     for(i=2;i<GRID_SIZE-2;i++){
          for(j=2;j<GRID_SIZE-2;j++){
               Position[i][j] = Position[i][j] - Velocity[i][j]*Speed;
               Velocity[i][j] = Velocity[i][j] * Viscosity;
          }
     }

     // Calculate new vertex coordinates
     for (i=0;i<GRID_SIZE;i++){
          for(j=0;j<GRID_SIZE;j++){
               Vertex[i][j].x = (i-GRID_SIZE/2.0)/GRID_SIZE*2.0;
               Vertex[i][j].y = (Position[i][j]/Speed/1024.0)/GRID_SIZE*2.0;
               Vertex[i][j].z = (j - GRID_SIZE/2.0)/GRID_SIZE*2.0;
          }
     }

     // Calculate the new vertex normals
     // do this by using the points to each side to get the right angle
     for (i=0; i<GRID_SIZE; i++){
          for (j=0; j<GRID_SIZE; j++){
               if(i>0 && j>0 && i<GRID_SIZE-2 && j<GRID_SIZE-2){
                    Normals[i][j].x = Position[i+1][j]-Position[i-1][j];
                    Normals[i][j].y = -2048;
                    Normals[i][j].z = Position[i][j+1]-Position[i][j-1];

                    VectLength = sqrt(Normals[i][j].x*Normals[i][j].x+Normals[i][j].y*Normals[i][j].y+Normals[i][j].z*Normals[i][j].z);
                    if(VectLength!=0){
                         Normals[i][j].x = Normals[i][j].x/VectLength;
                         Normals[i][j].y = Normals[i][j].y/VectLength;
                         Normals[i][j].z = Normals[i][j].z/VectLength;
                    }
               }else{
                    Normals[i][j].x=0;
                    Normals[i][j].y=1;
                    Normals[i][j].z=0;
               }
          }    
     }




}


void DrawWater(){
     int i,j;

     CalcWater();

     // Draw the water texture
     glBindTexture(GL_TEXTURE_2D, texture[0]);
     for(j=0; j<GRID_SIZE-1; j++){
          glBegin(GL_QUAD_STRIP);
//          glColor3f(0,0,1);
          for(i=0;i<GRID_SIZE;i++){
               glNormal3f(Normals[i][j+1].x,Normals[i][j+1].y,Normals[i][j+1].z);
               glTexCoord2f(i/GRID_SIZE,(j+1)/GRID_SIZE);
               glVertex3f(Vertex[i][j+1].x,Vertex[i][j+1].y,Vertex[i][j+1].z);

               glNormal3f(Normals[i][j].x,Normals[i][j].y,Normals[i][j].z);
               glTexCoord2f(i/GRID_SIZE,j/GRID_SIZE);
               glVertex3f(Vertex[i][j].x,Vertex[i][j].y,Vertex[i][j].z);
          }
          glEnd();
     }
}

void DrawBox(){
     glBindTexture(GL_TEXTURE_2D, texture[1]);
     glBegin(GL_QUADS);
     // back inner wall
     glColor3f(0.6, 0.6, 0.6);
     glTexCoord2f(0, 0.85);  glVertex3f(-1, -0.1, -1);
     glTexCoord2f(1, 0.85);  glVertex3f( 1, -0.1, -1);
     glTexCoord2f(1, 1.00);  glVertex3f( 1,  0.2, -1);
     glTexCoord2f(0, 1.00);  glVertex3f(-1,  0.2, -1);

     // back outer wall
     glTexCoord2f(0, 0.8);  glVertex3f(-1.1, -0.2, -1.1);
     glTexCoord2f(1, 0.8);  glVertex3f( 1.1, -0.2, -1.1);
     glTexCoord2f(1, 1.0);  glVertex3f( 1.1,  0.2, -1.1);
     glTexCoord2f(0, 1.0);  glVertex3f(-1.1,  0.2, -1.1);

     // front left outer wall
     glColor3f(0.5, 0.5, 0.5);
     glTexCoord2f(0.00, 0.8);  glVertex3f(-1.1, -0.2, -1.0);
     glTexCoord2f(0.03, 0.8);  glVertex3f(-1.1, -0.2, -1.1);
     glTexCoord2f(0.03, 1.0);  glVertex3f(-1.1,  0.2, -1.1);
     glTexCoord2f(0.00, 1.0);  glVertex3f(-1.1,  0.2, -1.0);

     // front right outer wall
     glTexCoord2f(0.00, 0.8);  glVertex3f(1.1, -0.2, -1.0);
     glTexCoord2f(0.03, 0.8);  glVertex3f(1.1, -0.2, -1.1);
     glTexCoord2f(0.03, 1.0);  glVertex3f(1.1,  0.2, -1.1);
     glTexCoord2f(0.00, 1.0);  glVertex3f(1.1,  0.2, -1.0);

     // back top wall
     glColor3f(1.0, 1.0, 1.0);
     glTexCoord2f(0, 1.0);  glVertex3f(-1.1, 0.2, -1.0);
     glTexCoord2f(1, 1.0);  glVertex3f( 1.1, 0.2, -1.0);
     glTexCoord2f(1, 0.9);  glVertex3f( 1.1, 0.2, -1.1);
     glTexCoord2f(0, 0.9);  glVertex3f(-1.1, 0.2, -1.1);

     // front inner wall
     glColor3f(0.6, 0.6, 0.6);
     glTexCoord2f(0, 0.85);  glVertex3f(-1, -0.1, 1);
     glTexCoord2f(1, 0.85);  glVertex3f( 1, -0.1, 1);
     glTexCoord2f(1, 1.00);  glVertex3f( 1,  0.2, 1);
     glTexCoord2f(0, 1.00);  glVertex3f(-1,  0.2, 1);

     // front outer wall
     glTexCoord2f(0, 0.8);  glVertex3f(-1.1, -0.2, 1.1);
     glTexCoord2f(1, 0.8);  glVertex3f( 1.1, -0.2, 1.1);
     glTexCoord2f(1, 1.0);  glVertex3f( 1.1,  0.2, 1.1);
     glTexCoord2f(0, 1.0);  glVertex3f(-1.1,  0.2, 1.1);

     // front left outer wall
     glColor3f(0.5, 0.5, 0.5);
     glTexCoord2f(0.00, 0.8);  glVertex3f(-1.1, -0.2, 1.0);
     glTexCoord2f(0.03, 0.8);  glVertex3f(-1.1, -0.2, 1.1);
     glTexCoord2f(0.03, 1.0);  glVertex3f(-1.1,  0.2, 1.1);
     glTexCoord2f(0.00, 1.0);  glVertex3f(-1.1,  0.2, 1.0);
     
     // front right outer wall
     glTexCoord2f(0.00, 0.8);  glVertex3f(1.1, -0.2, 1.0);
     glTexCoord2f(0.03, 0.8);  glVertex3f(1.1, -0.2, 1.1);
     glTexCoord2f(0.03, 1.0);  glVertex3f(1.1,  0.2, 1.1);
     glTexCoord2f(0.00, 1.0);  glVertex3f(1.1,  0.2, 1.0);
     
     // front top wall
     glColor3f(1.0, 1.0, 1.0);
     glTexCoord2f(0, 1.0);  glVertex3f(-1.1, 0.2, 1.0);
     glTexCoord2f(1, 1.0);  glVertex3f( 1.1, 0.2, 1.0);
     glTexCoord2f(1, 0.9);  glVertex3f( 1.1, 0.2, 1.1);
     glTexCoord2f(0, 0.9);  glVertex3f(-1.1, 0.2, 1.1);

     // right inner wall
     glColor3f(0.6, 0.6, 0.6);
     glTexCoord2f(0, 0.85);  glVertex3f( 1, -0.1, -1);
     glTexCoord2f(1, 0.85);  glVertex3f( 1, -0.1,  1);
     glTexCoord2f(1, 1.00);  glVertex3f( 1,  0.2,  1);
     glTexCoord2f(0, 1.00);  glVertex3f( 1,  0.2, -1);

     // right outer wall
     glTexCoord2f(0, 0.8);  glVertex3f( 1.1, -0.2,-1.0);
     glTexCoord2f(1, 0.8);  glVertex3f( 1.1, -0.2, 1.0);
     glTexCoord2f(1, 1.0);  glVertex3f( 1.1,  0.2, 1.0);
     glTexCoord2f(0, 1.0);  glVertex3f( 1.1,  0.2,-1.0);
     
     // right top wall
     glColor3f(1.0, 1.0, 1.0);
     glTexCoord2f(0, 1.0);  glVertex3f( 1.1, 0.2,-1.0);
     glTexCoord2f(1, 1.0);  glVertex3f( 1.1, 0.2, 1.0);
     glTexCoord2f(1, 0.9);  glVertex3f( 1.0, 0.2, 1.0);
     glTexCoord2f(0, 0.9);  glVertex3f( 1.0, 0.2,-1.0);

     // left inner wall
     glColor3f(0.6, 0.6, 0.6);
     glTexCoord2f(0, 0.85);  glVertex3f(-1, -0.1, -1);
     glTexCoord2f(1, 0.85);  glVertex3f(-1, -0.1,  1);
     glTexCoord2f(1, 1.00);  glVertex3f(-1,  0.2,  1);
     glTexCoord2f(0, 1.00);  glVertex3f(-1,  0.2, -1);
     
     // left outer wall
     glTexCoord2f(0, 0.8);  glVertex3f(-1.1, -0.2,-1.0);
     glTexCoord2f(1, 0.8);  glVertex3f(-1.1, -0.2, 1.0);
     glTexCoord2f(1, 1.0);  glVertex3f(-1.1,  0.2, 1.0);
     glTexCoord2f(0, 1.0);  glVertex3f(-1.1,  0.2,-1.0);

     // left top wall
     glColor3f(1.0, 1.0, 1.0);
     glTexCoord2f(0, 1.0);  glVertex3f(-1.1, 0.2,-1.0);
     glTexCoord2f(1, 1.0);  glVertex3f(-1.1, 0.2, 1.0);
     glTexCoord2f(1, 0.9);  glVertex3f(-1.0, 0.2, 1.0);
     glTexCoord2f(0, 0.9);  glVertex3f(-1.0, 0.2,-1.0);

     // bottom wall
     glTexCoord2f(0, 1);  glVertex3f(-1.1,-0.2,-1.1);
     glTexCoord2f(1, 1);  glVertex3f( 1.1,-0.2,-1.1);
     glTexCoord2f(1, 0);  glVertex3f( 1.1,-0.2, 1.1);
     glTexCoord2f(0, 0);  glVertex3f(-1.1,-0.2, 1.1);

     if(DrawWedge){
          // wedge top 1
          glTexCoord2f(0, 0.0);  glVertex3f(0.2,-0.10, 0.0);
          glTexCoord2f(1, 0.0);  glVertex3f(0.7,-0.10, 0.0);
          glTexCoord2f(1, 0.5);  glVertex3f(0.7, 0.15, -0.5);
          glTexCoord2f(0, 0.5);  glVertex3f(0.2, 0.15, -0.5);

          // wedge top 2
          glTexCoord2f(0, 0.5);  glVertex3f(0.2, 0.15, -0.5);
          glTexCoord2f(1, 0.5);  glVertex3f(0.7, 0.15, -0.5);
          glTexCoord2f(1, 1.0);  glVertex3f(0.7,-0.10, -1.0);
          glTexCoord2f(0, 1.0);  glVertex3f(0.2,-0.10, -1.0);

          // wedge left wall
          glColor3f(0.6, 0.6, 0.6);
          glTexCoord2f(0.0, 0.0);  glVertex3f(0.2,-0.1, -1.0);
          glTexCoord2f(1.0, 0.0);  glVertex3f(0.2,-0.1,  0.0);
          glTexCoord2f(0.5, 1.0);  glVertex3f(0.2, 0.15,-0.5);
          glTexCoord2f(0.0, 0.0);  glVertex3f(0.2,-0.1, -1.0);

          // wedge right wall
          glTexCoord2f(0.0, 0.0);  glVertex3f(0.7,-0.1, -1.0);
          glTexCoord2f(1.0, 0.0);  glVertex3f(0.7,-0.1,  0.0);
          glTexCoord2f(0.5, 1.0);  glVertex3f(0.7, 0.15,-0.5);
          glTexCoord2f(0.0, 0.0);  glVertex3f(0.7,-0.1, -1.0);
     }
  glEnd();
}

void TimerFunc(int Unused){
     CalcWater();
     glutPostRedisplay();
     glutTimerFunc(1000/60,TimerFunc,0);
}

void display(){

     glClearDepth(1.0);
     // Erase the window adn the depth buffer
     glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
     // Enable Z-buffering in OpenGL
     glEnable(GL_DEPTH_TEST);

     // Undo previous transformations
     glLoadIdentity();

      double Ex = -2*dim*Sin(th)*Cos(ph);
      double Ey = +2*dim        *Sin(ph);
      double Ez = +2*dim*Cos(th)*Cos(ph);
      gluLookAt(Ex,Ey,Ez , 0,0,0 , 0,Cos(ph),0);

   //  Light switch
   if (light)
   {
      //  Translate intensity to color vectors
      float Ambient[]   = {0.01*ambient ,0.01*ambient ,0.01*ambient ,1.0};
      float Diffuse[]   = {0.01*diffuse ,0.01*diffuse ,0.01*diffuse ,1.0};
      float Specular[]  = {0.01*specular,0.01*specular,0.01*specular,1.0};
      //  Light direction
      float Position[]  = {5*Cos(zh),ylight,5*Sin(zh),1};
      //  Draw light position as ball (still no lighting here)
      glDisable(GL_BLEND);
      glColor3f(1,1,1);
      ball(Position[0],Position[1],Position[2] , 0.1);
      glEnable(GL_BLEND);
      //  OpenGL should normalize normal vectors
      glEnable(GL_NORMALIZE);
      //  Enable lighting
      glEnable(GL_LIGHTING);
      //  glColor sets ambient and diffuse color materials
      glColorMaterial(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE);
      glEnable(GL_COLOR_MATERIAL);
      //  Enable light 0
      glEnable(GL_LIGHT0);
      //  Set ambient, diffuse, specular components and position of light 0
      glLightfv(GL_LIGHT0,GL_AMBIENT ,Ambient);
      glLightfv(GL_LIGHT0,GL_DIFFUSE ,Diffuse);
      glLightfv(GL_LIGHT0,GL_SPECULAR,Specular);
      glLightfv(GL_LIGHT0,GL_POSITION,Position);
   }
   else
      glDisable(GL_LIGHTING);

     
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    DrawBox();


    glColor4f(.85,1,.85,alpha);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

     DrawWater();



     glFlush();
     glutSwapBuffers();

}

// Creates a raindrop
void CreateRainDrop(){
     int index1 = (rand() % (GRID_SIZE-4))+2;
     int index2 = (rand() % (GRID_SIZE-4))+2;
     Velocity[index1][index2] = 1000;
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
   else if (ch =='m')
      mode = (mode+8-1)%8;
   //  Toggle lighting
   else if (ch == 'l' || ch == 'L')
      light = 1-light;
   //  Light elevation
   else if (ch=='[')
      ylight -= 0.1;
   else if (ch==']')
      ylight += 0.1;
   //  Ambient level
   else if (ch=='a' && ambient>0)
      ambient -= 5;
   else if (ch=='A' && ambient<100)
      ambient += 5;
   //  Diffuse level
   else if (ch=='d' && diffuse>0)
      diffuse -= 5;
   else if (ch=='D' && diffuse<100)
      diffuse += 5;
   //  Specular level
   else if (ch=='s' && specular>0)
      specular -= 5;
   else if (ch=='S' && specular<100)
      specular += 5;
   //  Emission level
   else if (ch=='e' && emission>0)
      emission -= 5;
   else if (ch=='E' && emission<100)
      emission += 5;
   //  Shininess level
   else if (ch=='n' && shininess>-1)
      shininess -= 1;
   else if (ch=='N' && shininess<7)
      shininess += 1;
   //  Blending method
   else if (ch=='q' && alpha>0)
      alpha-=.1;
   else if (ch=='Q' && alpha<1)
      alpha +=.1;
   //  Repitition
   else if (ch=='+')
      rep++;
   else if (ch=='-' && rep>1)
      rep--;
   else if (ch==' ')
      CreateRainDrop();
   //  Translate shininess power to value (-1 => 0)
   shinyvec[0] = shininess<0 ? 0 : pow(2.0,shininess);
   //  Reproject
   Project(45,asp,dim);
   //  Tell GLUT it is necessary to redisplay the scene
   glutPostRedisplay();
}


/*
 *  Start up GLUT and tell it what to do
 */
int main(int argc,char* argv[])
{
   //  Initialize GLUT
   glutInit(&argc,argv);
   //  Request double buffered, true color window with Z buffering at 600x600
   glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
   glutInitWindowSize(600,600);
   glutCreateWindow("Textures and Lighting");
   //  Set callbacks
   glutDisplayFunc(display);
   glutTimerFunc(1000/60.0, TimerFunc,0);
   glutReshapeFunc(reshape);
   glutSpecialFunc(special);
   glutKeyboardFunc(key);
   glutIdleFunc(idle);
//   glutTimerFunc(
   //  Load textures

     glDepthFunc(GL_LESS);
     glBlendFunc(GL_SRC_COLOR,GL_ONE);
     glEnable(GL_TEXTURE_2D);
     Viscosity=0.99;

   texture[0] = LoadTexBMP("reflection.bmp");
   texture[1] = LoadTexBMP("wood1.bmp");
   //  Pass control to GLUT so it can interact with the user
   glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
   glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP); 

  ErrCheck("init");
   glutMainLoop();
   return 0;
}

