// This file includes
// 1. different shaped brushes have been implemented(quad-triangle-line-round)
// 2. step rotation of 10 defrees has been applied
// 3. various colors can be obtained on pressing different number buttons
// 4. Finally spray paint brush has been implemented

#include "stdafx.h"
#include <gl/glut.h>
#include <gl/gl.h>
#include <stdio.h>
#include <math.h>

#define WINDOW_WIDTH 400                                  // Window width and height are of the ratio 4:3
#define WINDOW_HEIGHT 300

float size_of_rectangle = 10;                             // defines the size of the polygon
float xmouse = -10;                                       // x coordinate of mouse position
float ymouse = -10;                                       // y coordinate of mouse position
int colornum = 0;                                         // color of the polygon
int flag_for_clearing = 0;                                // used for clearing the screen
int count_brush = 1;                                      // it stores the brush shape that has been selected
int color[3]={0};                                         // array stores RGB values which defines the color
float rotation_theta = 0;                                 // stores the current rotation angle

// This functions draws a circle by using the idea that a circle is a polygon made up of infinitely many edges
void drawCircle(float cx, float cy, float r)                         
{ 
	glBegin(GL_POLYGON);                 
	float theta =0;
	for(int i = 0; i < 360; i++) 
	{ 
		theta = theta + 2.0f * 3.1415926f / 360.0;           //get the current angle 

		float x = r * cosf(theta);                           //calculate the x component 
		float y = r * sinf(theta);                           //calculate the y component 

		glVertex2f(x + cx, y + cy);                          //output vertex 
	} 
	glEnd(); 
	glFlush ( );
}

//This function is used to draw spray paint brush. concept of blending by varying the alpha values which is the fourth parameter in the color vector
// is used. 
void drawCircleSpray(float cx, float cy, float r) 
{ 
	int j;
	
	for(j=0;j<r;j++)                                                 // draws several polygons with varying radius
	{
		float theta =0;
		glColor4f ( color[0],color[1],color[2],(expf(-(j*2.5)/r)));   // alpha value is increased from centre to outside by using an exponential function
		glBegin(GL_POLYGON); 
		for(int i = 0; i < 360*2; i=i++) 
		{ 
			theta = theta + 2.0f * 3.1415926f*0.5 /360.0;//get the current angle 

			float x = j * cosf(theta);//calculate the x component 
			float y = j * sinf(theta);//calculate the y component 

			glVertex2f(x + cx, y + cy);//output vertex 
		} 
		glEnd();
	}
	glEnd(); 
	glFlush ( );
}

void display ( void )
{
	if (flag_for_clearing == 1)
	{
		glClear ( GL_COLOR_BUFFER_BIT );                // clears the screen
	}
	if(flag_for_clearing != 1)                          // when user presees 'c' for clearing ,this 'if' block is not executed
	{
		color[0]=color[1]=color[2]=0;                   // array used to store RGB bits
		int num=colornum;                               // based on the user input number the color of polygon is being identified
		int i=0;
		while (num>0)
		{
			color[i]=num%2;
			num=num/2;
			i++;
		}
		glColor3f ( color[0], color[1], color[2]);
		// here mathematical rotation and translational formulas have been used
		// new x = position of mousex + x*cos(theta) - y*sin(theta)
		// new y = position of mousey + x*sin(theta) + y*cos(theta)
		switch (count_brush)
		{
		case 1:
			glBegin (GL_QUADS);
				glVertex2f (xmouse-size_of_rectangle*cosf(rotation_theta)+size_of_rectangle*sinf(rotation_theta),ymouse-size_of_rectangle*sinf(rotation_theta)-size_of_rectangle*cosf(rotation_theta));
				glVertex2f (xmouse+size_of_rectangle*cosf(rotation_theta)+size_of_rectangle*sinf(rotation_theta),ymouse+size_of_rectangle*sinf(rotation_theta)-size_of_rectangle*cosf(rotation_theta));
				glVertex2f (xmouse+size_of_rectangle*cosf(rotation_theta)-size_of_rectangle*sinf(rotation_theta),ymouse+size_of_rectangle*sinf(rotation_theta)+size_of_rectangle*cosf(rotation_theta));
				glVertex2f (xmouse-size_of_rectangle*cosf(rotation_theta)-size_of_rectangle*sinf(rotation_theta),ymouse-size_of_rectangle*sinf(rotation_theta)+size_of_rectangle*cosf(rotation_theta));
			glEnd ( );
			break;
		case 2:
			glBegin (GL_TRIANGLES);
				glVertex2f (xmouse-size_of_rectangle*cosf(rotation_theta)+size_of_rectangle*sinf(rotation_theta),ymouse-size_of_rectangle*sinf(rotation_theta)-size_of_rectangle*cosf(rotation_theta));
				glVertex2f (xmouse+size_of_rectangle*cosf(rotation_theta)+size_of_rectangle*sinf(rotation_theta),ymouse+size_of_rectangle*sinf(rotation_theta)-size_of_rectangle*cosf(rotation_theta));
				glVertex2f (xmouse-size_of_rectangle*sinf(rotation_theta),ymouse+size_of_rectangle*cosf(rotation_theta));
			glEnd ( );
			break;
		case 3:
			glBegin (GL_LINES);
				glVertex2f (xmouse+size_of_rectangle*cosf(rotation_theta),ymouse+ size_of_rectangle*sinf(rotation_theta));
				glVertex2f (xmouse-size_of_rectangle*cosf(rotation_theta),ymouse-size_of_rectangle*sinf(rotation_theta));
			glEnd ( );
			break;
		case 4:
			drawCircle(xmouse, ymouse, size_of_rectangle);
			break;
		case 5:
			drawCircleSpray(xmouse, ymouse, size_of_rectangle);
			break;
		}
	}
	glFlush ( );
}

void keyboard ( unsigned char key, int x, int y )
{
	flag_for_clearing=0;
	switch ( key )
	{
		case '+' : 
			if( size_of_rectangle*2>=128 )
				printf("cannot increase size");
			else
				size_of_rectangle=size_of_rectangle*2;
			break;
		case '-' :
			if(size_of_rectangle/2<=1)
				printf("cannot decrease size");
			else
				size_of_rectangle=size_of_rectangle/2;
			break;
		case '0' :
			colornum = 0;
			break;
		case '1' :
			colornum = 1;
			break;
		case '2' :
			colornum = 2;
			break;
		case '3' :
			colornum = 3;
			break;
		case '4' :
			colornum = 4;
			break;
		case '5' :
			colornum = 5;
			break;
		case '6' :
			colornum = 6;
			break;
		case '7' :
			colornum = 7;
			break;
		case 'c' :
			xmouse = -10;
			ymouse = -10;
			flag_for_clearing=1;
			break;
		case 'b' :                              // brush shape changes quad - triangle - line - round
			xmouse = -10; 
			ymouse = -10;
			if(count_brush >= 4)
				count_brush = 1;
			else
				count_brush++;
			break;
		case 'a' :                              // On presing successive a the brush toggles between solid circle and spray paint brush
			xmouse = -10;
			ymouse = -10;
			if(count_brush == 5)
				count_brush = 4;
			else
				count_brush=5;
			break;
		case 'r' :                                // successive rotation of angle by 10 degrees
			rotation_theta=rotation_theta+10*2.0f * 3.1415926f / 360.0;
			break;
	}
	glutPostRedisplay ( );
}

// all the functions described below are same as those in first file

void init ( void )
{
	glEnable (GL_BLEND);                                      // initialisation for blending operations for setting alpha
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel (GL_FLAT);
    glClearColor (0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0.0, WINDOW_WIDTH-1, WINDOW_HEIGHT-1, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void mouseMove ( int x, int y )
{
	xmouse = x;
	ymouse = y;
	glutPostRedisplay ( );
}

int main ( int argc, char *argv[] )
{
	glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize (WINDOW_WIDTH, WINDOW_HEIGHT); 
    glutInitWindowPosition (100, 100);
    glutCreateWindow ("Rajashree Rao Polsani - Assignment 1");
    init ();
    glutDisplayFunc(display);
	glutMotionFunc ( mouseMove );
	glutKeyboardFunc ( keyboard );
	glutMainLoop ( );
	return 0;
}