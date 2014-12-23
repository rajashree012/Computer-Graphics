// import statements
#include "stdafx.h"
#include "assignment3.h"
#include "init.h"
#include <gl/glut.h>
#include <gl/gl.h>
#include <stdio.h>
#include <math.h>
#include <queue>

// stores the transformations
vector<Matrix> iat = vector<Matrix>();
// initial points
vector<Pt> points = vector<Pt>();
// stores condensation points
vector<Pt> condensation_points = vector<Pt>();
// number of iterations vary with number of transformations
int num_iterations = 0;

// translate transformation
Matrix translate ( Vec v )
{
	Matrix rvalue;
	rvalue.data[0][0] = 1;
	rvalue.data[1][1] = 1;
	rvalue.data[2][2] = 1;
	rvalue.data[0][2] = v.x;
	rvalue.data[1][2] = v.y;
	return rvalue;
}

//rotate transformation
Matrix rotate ( Pt p, float theta )
{
	Matrix rvalue;
	rvalue.data[0][0] = cos(theta);
	rvalue.data[0][1] = -sin(theta);
	rvalue.data[0][2] = (p.x) + (p.y)*sin(theta) - (p.x)*cos(theta);
	rvalue.data[1][0] = sin(theta);
	rvalue.data[1][1] = cos(theta);
	rvalue.data[1][2] = (p.y) - (p.y)*cos(theta) - (p.x)*sin(theta);
	rvalue.data[2][2] = 1;
	return rvalue;
}

// uniform scaling
Matrix scale ( Pt p, float alpha )
{
	Matrix rvalue;
	rvalue.data[0][0] = alpha;
	rvalue.data[0][2] = (1-alpha)*p.x;
	rvalue.data[1][1] = alpha;
	rvalue.data[1][2] = (1-alpha)*p.y;
	rvalue.data[2][2] = 1;
	return rvalue;
}

// non-uniform scaling
Matrix nscale ( Pt p, Vec v, float alpha )
{
	Matrix rvalue;
	rvalue.data[0][0] = 1 + (alpha-1)*(v.x)*(v.x);
	rvalue.data[0][1] = (alpha-1)*(v.x)*(v.y);
	rvalue.data[0][2] = (v.x)*((p.x)*(v.x)+(p.y)*(v.y))*(1-alpha);
	rvalue.data[1][0] = (alpha-1)*(v.x)*(v.y);
	rvalue.data[1][1] = 1 + (alpha-1)*(v.y)*(v.y);
	rvalue.data[1][2] = (v.y)*((p.x)*(v.x)+(p.y)*(v.y))*(1-alpha);
	rvalue.data[2][2] = 1;
	return rvalue;
}

// image transformation
Matrix image ( Pt p1, Pt p2, Pt p3, Pt q1, Pt q2, Pt q3 )
{
	Matrix rvalue;
	Matrix p;
	p.data[0][0] = p1.x;
	p.data[0][1] = p2.x;
	p.data[0][2] = p3.x;
	p.data[1][0] = p1.y;
	p.data[1][1] = p2.y;
	p.data[1][2] = p3.y;
	p.data[2][0] = 1;
	p.data[2][1] = 1;
	p.data[2][2] = 1;
	Matrix q;
	q.data[0][0] = q1.x;
	q.data[0][1] = q2.x;
	q.data[0][2] = q3.x;
	q.data[1][0] = q1.y;
	q.data[1][1] = q2.y;
	q.data[1][2] = q3.y;
	q.data[2][0] = 1;
	q.data[2][1] = 1;
	q.data[2][2] = 1;
	Matrix p_inverse;
	Matrix cofactor;
	cofactor.data[0][0] = (p2.y)*1 - (p3.y)*1;
	cofactor.data[0][1] = (p3.y)*1 - (p1.y)*1;
	cofactor.data[0][2] = (p1.y)*1 - (p2.y)*1;
	cofactor.data[1][0] = (p3.x)*1 - (p2.x)*1;
	cofactor.data[1][1] = (p1.x)*1 - (p3.x)*1;
	cofactor.data[1][2] = (p2.x)*1 - (p1.x)*1;
	cofactor.data[2][0] = (p2.x)*(p3.y) - (p2.y)*(p3.x);
	cofactor.data[2][1] = (p3.x)*(p1.y) - (p3.y)*(p1.x);
	cofactor.data[2][2] = (p1.x)*(p2.y) - (p1.y)*(p2.x);
	Matrix cof_transpose;
	for (int i = 0; i<=2 ; i++)
	{
		for (int j = 0; j<=2 ; j++)
		{
			cof_transpose.data[i][j] = cofactor.data[j][i];
		}
	}
	float det = (p1.x)*((p2.y)*1 - (p3.y)*1) + (p2.x)*((p3.y)*1 - (p1.y)*1) + (p3.x)*((p1.y)*1 - (p2.y)*1);
	for (int i = 0; i<=2 ; i++)
	{
		for (int j = 0; j<=2 ; j++)
		{
			p_inverse.data[i][j] = (cof_transpose.data[i][j]*1.0)/det;
		}
	}
	for (int i = 0; i<=2 ; i++)
	{
		for (int j = 0; j<=2 ; j++)
		{
			for (int k = 0; k<=2 ; k++)
			{
				rvalue.data[i][j] = rvalue.data[i][j] + q.data[i][k]*p_inverse.data[k][j] ;
			}
		}
	}
	return rvalue;
}

// composing of two matrices
Matrix compose ( Matrix m2, Matrix m1 )
{
	Matrix rvalue;
	for (int i = 0; i<=2 ; i++)
	{
		for (int j = 0; j<=2 ; j++)
		{
			for (int k = 0; k<=2 ; k++)
			{
				rvalue.data[i][j] = rvalue.data[i][j] + m2.data[i][k]*m1.data[k][j] ;
			}
		}
	}
	return rvalue;
}

// multiplication of transformed matrix and original point to get new point
Pt matrix_vec_mult (Matrix m, Pt p)
{
	float pp[3];
	pp[0] = p.x;
	pp[1] = p.y;
	pp[2] = 1;
	float p2[3]={0,0,0};
	for (int i =0; i<=2 ; i++)
	{
		for (int j=0; j<=2 ; j++)
		{
			p2[i] = p2[i] + m.data[i][j]*pp[j];
		}
	}
	Pt tran_point = Pt(p2[0],p2[1]);
	return tran_point;
}

// sets the condensation set
void setCondensationSet ( vector<Pt> pts )
{
	condensation_points = vector<Pt>();
	for (int k=0 ; k<pts.size() ; k++)
	{
		condensation_points.push_back(pts.at(k));
	}
}

// sets the transformation as well as chooses number of iterations
void setIATTransformations ( vector<Matrix> transformations )
{
	iat = vector<Matrix>();
	for (int k=0 ; k<transformations.size() ; k++)
	{
		iat.push_back(transformations.at(k));
	}
	if (transformations.size() <= 4)
		num_iterations = 9;
	else if (transformations.size() <= 7)
		num_iterations = 6;
	else
		num_iterations = 4;
}

void drawing()
{
	// current stores current set of figures
	queue<vector<Pt>> current;
	int num_initial_points = 0;
	// if there are no condensation points then the four corners of the screen are considered as initial points else the condensation points are
	// themselves considered
	if (points.size() == 0)
	{
		points.push_back(Pt(-1,-1));
		points.push_back(Pt(-1,1));
		points.push_back(Pt(1,-1));
		points.push_back(Pt(1,1));
	}
	if (condensation_points.size() == 0)
	{
		num_initial_points = points.size();
		current.push(points);
	}
	else 
	{
		current.push(condensation_points);
		num_initial_points = condensation_points.size();
	}
	// within each iteration each diagram of previous iteration is considered and each transformation is applied on each one
	// in case there is a condensation set it is added at the end
	for (int i=0; i<num_iterations;i++)
	{
		int x = (int)current.size();
		for(int j=0 ; j<x; j++)
		{
			vector<Pt> pointsinitial = current.front();
			current.pop();
			for (int k=0 ; k<iat.size() ; k++)
			{
				vector<Pt> pointsfinal;
				for (int l=0; l<num_initial_points ; l++)
				{
					pointsfinal.push_back(matrix_vec_mult(iat.at(k),pointsinitial.at(l)));
				}
				current.push(pointsfinal);
			}
		}
		if (condensation_points.size() != 0)
			current.push(condensation_points);
	}
	glColor3f(1.0,1.0,1.0);
	// figures are drawn after popping out of the queue
	int size = current.size();
	for (int i=0;i<size;i++)
	{
		if (condensation_points.size() > 1)
			glBegin(GL_LINE_LOOP);
		else
			glBegin(GL_POINTS);
		for (int j=0; j<num_initial_points ;j++)
		{
			glVertex2f(current.front().at(j).x,current.front().at(j).y);
		}
		current.pop();
		glEnd();
	}
	glFlush();
	condensation_points = vector<Pt>();
}

// Draws the current IAT
void display ( void )
{
	glClear ( GL_COLOR_BUFFER_BIT );
	glFlush ( );
	if (iat.size() > 0)
		drawing();
}		

// iterations for all the figures are stored
void keyboard ( unsigned char key, int x, int y )           
{
	vector<Matrix> iat = vector<Matrix>();
	vector<Pt> condensation_points = vector<Pt>();
	switch ( key )
	{
		case '1' :  
			iat.push_back ( scale ( Pt ( -1,-1 ), 0.5 ) );
			iat.push_back ( compose (scale ( Pt ( 0.5,-1 ), 0.5 ) ,translate ( Pt ( 0.5,0 ) ) ) );
			iat.push_back ( compose (rotate (Pt(0.5,0.5),1.57),compose (translate(Pt(0,1)) , compose (scale ( Pt ( 0.5,-1 ), 0.5 ) ,translate ( Pt ( 0.5,0 ) ) ) ) ) );
			setIATTransformations(iat);
			break;

		case '2' :  
			iat.push_back ( compose (scale ( Pt ( 0,-1 ), 0.5 ) ,translate ( Pt ( 1,0 ) ) ) );
			iat.push_back ( compose (scale ( Pt ( -1,0 ), 0.5 ) ,translate ( Pt ( 0,1 ) ) ) );
			setIATTransformations(iat);
			condensation_points.push_back(Pt(-1,-1));
			condensation_points.push_back(Pt(0,-1));
			condensation_points.push_back(Pt(0,0));
			condensation_points.push_back(Pt(-1,0));
			setCondensationSet(condensation_points);
			break;

		case '3' :
			iat.push_back ( scale ( Pt ( -.9, 0.2915 ), 0.381 ) );
			iat.push_back ( scale ( Pt ( -0.555,-0.7645 ), 0.381 ) );
			iat.push_back ( scale ( Pt ( 0.555,-0.7645 ), 0.381 ) );
			iat.push_back ( scale ( Pt ( .9, 0.2915 ), 0.381 ) );
			iat.push_back ( scale ( Pt ( 0, 0.9435 ), 0.381 ) );
			iat.push_back ( scale ( Pt ( 0, 0 ),-0.381 ) );
			setIATTransformations(iat);
			break;

		case '4' : 
			iat.push_back ( scale ( Pt ( -.9, 0 ), 0.33 ) );
			iat.push_back ( scale ( Pt ( -.45, -.7794 ), 0.33 ) );
			iat.push_back ( scale ( Pt ( .45, -.7794 ), 0.33 ) );
			iat.push_back ( scale ( Pt ( .9, 0 ), 0.33 ) );
			iat.push_back ( scale ( Pt ( .45, .7794 ), 0.33 ) );
			iat.push_back ( scale ( Pt ( -.45, .7794 ), 0.33 ) );
			setIATTransformations(iat);
			break;

		case '5' :
			points = vector<Pt>();
			points.push_back(Pt(0,-0.28));
			points.push_back(Pt(-0.08,-0.08));
			points.push_back(Pt(-0.28,-0.07));
			points.push_back(Pt(-0.14,0.08));
			points.push_back(Pt(-0.18,0.27));
			points.push_back(Pt(0.0,0.11));
			points.push_back(Pt(0.18,0.27));
			points.push_back(Pt(0.14,0.08));
			points.push_back(Pt(0.28,-0.07));
			points.push_back(Pt(0.08,-0.08));
			iat.push_back ( scale ( Pt ( 0, -.28 ), -0.25 ) );
			iat.push_back ( scale ( Pt ( -0.28,-0.07 ), -0.25 ) );
			iat.push_back ( scale ( Pt ( -0.18,0.27 ), -0.25 ) );
			iat.push_back ( scale ( Pt ( 0.18,0.27 ), -0.25 ) );
			iat.push_back ( scale ( Pt ( 0.28,-0.07 ), -0.25 ) );
			iat.push_back ( scale ( Pt ( 0, -.28 ), 0.25 ) );
			iat.push_back ( scale ( Pt ( -0.28,-0.07 ), 0.25 ) );
			iat.push_back ( scale ( Pt ( -0.18,0.27 ), 0.25 ) );
			iat.push_back ( scale ( Pt ( 0.18,0.27 ), 0.25 ) );
			iat.push_back ( scale ( Pt ( 0.28,-0.07 ), 0.25 ) );
			iat.push_back ( scale ( Pt ( 0,0 ), 0.24 ) );
			setIATTransformations(iat);
			break;

		case '6': 
			condensation_points.push_back(Pt(0,0.25));
			condensation_points.push_back(Pt(0,0));
			condensation_points.push_back(Pt(-0.216,-0.125));
			condensation_points.push_back(Pt(0,0));
			condensation_points.push_back(Pt(0.216,-0.125));
			condensation_points.push_back(Pt(0,0));
			setCondensationSet(condensation_points);
			iat.push_back ( compose(scale( Pt(0,.25),0.5),( compose (translate ( Pt ( 0,0.25 )) ,rotate ( Pt ( 0,0 ),3.14 )) ))) ;
			iat.push_back ( compose(scale( Pt(-0.216,-0.125),0.5),( compose (translate ( Pt ( -0.216,-0.125 )) ,rotate ( Pt ( 0,0 ),3.14 )) ))) ;
			iat.push_back ( compose(scale( Pt(0.216,-0.125),0.5),( compose (translate ( Pt ( 0.216,-0.125 )) ,rotate ( Pt ( 0,0 ),3.14 )) ))) ;
			setIATTransformations(iat);
			break;
	}
	glutPostRedisplay();
}

/* do not modify the reshape function */
void reshape ( int width, int height )
{
	glViewport ( 0, 0, width, height );
	glMatrixMode ( GL_PROJECTION );
	glLoadIdentity ( );    
	gluOrtho2D (-1, 1, -1, 1);
	glMatrixMode ( GL_MODELVIEW );
    glLoadIdentity ( );
}

int main ( int argc, char** argv )
{
	glutInit ( &argc, argv );
	glutInitDisplayMode ( GLUT_SINGLE | GLUT_RGB );
	glutInitWindowSize ( 500, 500 );
	glutInitWindowPosition ( 100, 100 );
	glutCreateWindow ( "Rajashree Rao Polsani - Homework 3" );
	init ( );	
	glutDisplayFunc ( display );
	glutReshapeFunc ( reshape );
	glutKeyboardFunc ( keyboard );
	glutMainLoop ( );
	return 0;
}
