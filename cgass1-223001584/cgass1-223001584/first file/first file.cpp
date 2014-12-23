// In this file 
//1. a window of width to height ratio of 4:3 has been created 
//2. a rectangle from (-size, -size) to (size,size) centered at the current mouse position  has been created
//3.various color changes have been applied
//4.clearing the screen functionality has been implemented


#include "stdafx.h"
#include <gl/glut.h>
#include <gl/gl.h>
#include <stdio.h>

// Window width and height are of the ratio 4:3
#define WINDOW_WIDTH 400
#define WINDOW_HEIGHT 300

float size_of_polygon = 10;                    // defines the size of the polygon
float xmouse = -10;                            // x coordinate of mouse position
float ymouse = -10;                            // y coordinate of mouse position
int colornum = 0;                              // color of the polygon
int flag_for_clearing = 0;                     // used for clearing the screen

void display ( void )
{
	glClear ( GL_COLOR_BUFFER_BIT );            // clears the screen
	if(flag_for_clearing != 1)                  // when user presees 'c' for clearing ,this 'if' block is not executed
	{
		int color[3]={0};                       // array used to store RGB bits
		int num=colornum;                       // based on the user input number the color of polygon is being identified
		int i=0;
		while (num>0)
		{
			color[i]=num%2;
			num=num/2;
			i++;
		}
		glColor3f ( color[0], color[1], color[2] );                // color of polygon is being set
		glBegin (GL_QUADS);                                        // polygon is drawn (here it is rectangle with vertices centered about mouse position)
			glVertex2f (xmouse-size_of_polygon,ymouse-size_of_polygon);          
			glVertex2f (xmouse-size_of_polygon,ymouse+size_of_polygon);
			glVertex2f (xmouse+size_of_polygon,ymouse+size_of_polygon);
			glVertex2f (xmouse+size_of_polygon,ymouse-size_of_polygon);
		glEnd ( );
	}
	glFlush ( );
}

void keyboard ( unsigned char key, int x, int y )           // this function deals with all the keyboard inputs given by the user
{
	flag_for_clearing=0;
	switch ( key )
	{
		case '+' : 
			if( size_of_polygon*2>128 )                  // '+' increments size of the polygon by factor of 2 and sees that it is not more than 128
				printf("cannot increase size");
			else
				size_of_polygon=size_of_polygon*2;
			break;
		case '-' :                                      // '-' increments size of the polygon by factor of 2 and sees that it is not less than 1
			if(size_of_polygon/2<1)
				printf("cannot decrease size");
			else
				size_of_polygon=size_of_polygon/2;
			break;
		case '0' :                                      // '0' colors the polygon black
			colornum = 0;
			break;
		case '1' :                                      // '1' colors the polygon red
			colornum = 1;
			break;
		case '2' :                                      // '2' colors the polygon green
			colornum = 2;
			break;
		case '3' :                                      // '3' colors the polygon yellow
			colornum = 3;
			break;
		case '4' :                                      // '4' colors the polygon blue
			colornum = 4;
			break;
		case '5' :                                      // '5' colors the polygon pink
			colornum = 5;                               
			break;
		case '6' :                                      // '6' colors the polygon light blue
			colornum = 6;
			break;
		case '7' :                                      // '7' colors the polygon white
			colornum = 7;
			break;
		case 'c' :                                      // clears the screen and initialises x and y coordinates of mouse
			xmouse = -10;
			ymouse = -10;
			flag_for_clearing=1;                        // flag is set to indicate clearing
			break;
	}
	glutPostRedisplay ( );
}

void init ( void )
{
    glClearColor (0.0, 0.0, 0.0, 0.0);                               // sets background color and initalises details regarding matrix mode and ortho2d
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, WINDOW_WIDTH-1, WINDOW_HEIGHT-1, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}

void mouseMove ( int x, int y )                          // xmouse and ymouse record the position of the mouse cursor
{
	xmouse = x;
	ymouse = y;
	glutPostRedisplay ( );
}

int main ( int argc, char *argv[] )
{
	glutInit(&argc, argv);
    glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);                    // display mode is set to RGB
    glutInitWindowSize (WINDOW_WIDTH, WINDOW_HEIGHT);                // setting window size to 400X300
    glutInitWindowPosition (100, 100);                               // initializing window coordinates with respect to screen coordinates
    glutCreateWindow ("Rajashree Rao Polsani - Assignment 1");       // creating window with the given title
    init ();                                                         // all the initializations
    glutDisplayFunc(display);                                        // display function
	glutMotionFunc ( mouseMove );
	glutKeyboardFunc ( keyboard );
	glutMainLoop ( );
	return 0;
}