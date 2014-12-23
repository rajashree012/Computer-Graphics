#include "stdafx.h"
#include <Windows.h>
#include <GL/glut.h>
#include <math.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>
#include <map>

# define ImageH 800
# define ImageW 800

class dataPt
{
public:
	float x, y, z;

	dataPt ( void )
	{
		x = y = z = 0.0;
	}

	dataPt ( float nX, float nY, float nZ)
	{
		x = nX;
		y = nY;
		z = nZ;
	}
};

using namespace std;
string name;
vector<dataPt> textPoints;
vector<vector<int>> faces;
vector<dataPt> textPointsConstant;
vector<vector<int>> facesConstant;
vector<dataPt> newTextPoints;
vector<vector<int>> newFaces;
double rotate_y=0; 
double rotate_x=0;
std::map < std::pair < int, int >, int > hashedgecentres;
float max_x = 0;
float max_y = 0;
float max_z = 0;
float maximum = 0;
int linear_subdivision = 0;
int averag = 0;
float initialxmouse = 0;
float initialymouse = 0;
float finalxmouse = 0;
float finalymouse = 0;
int flag_zoom = 0;
int flag_rotate = 0;

// calculates the normal of a suraface
vector<float> normalCalculation (int one,int two,int three)
{
	vector<float> crossproduct;
	float temp1 = 0;
	float temp2 = 0;
	float temp3 = 0;
	temp1 = (textPoints.at(three-1).y - textPoints.at(one-1).y)*(textPoints.at(two-1).z - textPoints.at(one-1).z) - (textPoints.at(two-1).y - textPoints.at(one-1).y)*(textPoints.at(three-1).z - textPoints.at(one-1).z);
	temp2 = (textPoints.at(two-1).x - textPoints.at(one-1).x)*(textPoints.at(three-1).z - textPoints.at(one-1).z) - (textPoints.at(three-1).x - textPoints.at(one-1).x)*(textPoints.at(two-1).z - textPoints.at(one-1).z);
	temp3 = (textPoints.at(three-1).x - textPoints.at(one-1).x)*(textPoints.at(two-1).y - textPoints.at(one-1).y) - (textPoints.at(two-1).x - textPoints.at(one-1).x)*(textPoints.at(three-1).y - textPoints.at(one-1).y);
	float sum = sqrt(temp1*temp1 + temp2*temp2 + temp3*temp3);
	crossproduct.push_back(temp1/sum);
	crossproduct.push_back(temp2/sum);
	crossproduct.push_back(temp3/sum);
	return crossproduct;
}

// calculates the centre of the edge
int getVertex(int p1,int p2)
{
	std::pair < int, int > order;
	if (p1 >= p2)
	{
		order.first = p2;
		order.second = p1;
	}
	else
	{
		order.first = p1;
		order.second = p2;
	}
	if (hashedgecentres.find(order) == hashedgecentres.end())
	{
		float sumx = (textPoints.at(p1).x + textPoints.at(p2).x)/2 ;
		float sumy = (textPoints.at(p1).y + textPoints.at(p2).y)/2 ;
		float sumz = (textPoints.at(p1).z + textPoints.at(p2).z)/2 ;
		newTextPoints.push_back(dataPt(sumx,sumy,sumz));
		hashedgecentres[order] = newTextPoints.size() - 1;
		return hashedgecentres[order];
	}
	else 
	{
		return hashedgecentres[order];
	}
}

// averaging for all the faces
void averaging()
{
	newFaces = vector<vector<int>>();
	newFaces = faces;
	newTextPoints = vector<dataPt>();
	vector<float> val;
	for (int i = 0; i<textPoints.size() ; i++)
	{
		newTextPoints.push_back(dataPt(0,0,0));
		val.push_back(0);
	}
	for (int i = 0; i<faces.size() ; i++)
	{
		float centx = 0;
		float centy = 0;
		float centz = 0;
		for (int j = 0; j<4; j++)
		{
			centx = centx + textPoints.at(faces.at(i).at(j)-1).x;
			centy = centy + textPoints.at(faces.at(i).at(j)-1).y;
			centz = centz + textPoints.at(faces.at(i).at(j)-1).z;
		}
		centx = centx/4;
		centy = centy/4;
		centz = centz/4;
		for (int j = 0; j<4; j++)
		{
			newTextPoints.at(faces.at(i).at(j)-1).x = newTextPoints.at(faces.at(i).at(j)-1).x + centx;
			newTextPoints.at(faces.at(i).at(j)-1).y = newTextPoints.at(faces.at(i).at(j)-1).y + centy;
			newTextPoints.at(faces.at(i).at(j)-1).z = newTextPoints.at(faces.at(i).at(j)-1).z + centz;
			val.at(faces.at(i).at(j)-1) = val.at(faces.at(i).at(j)-1) + 1;
		}
	}
	for (int i = 0; i<textPoints.size() ; i++)
	{
		newTextPoints.at(i).x = newTextPoints.at(i).x/val.at(i);
		newTextPoints.at(i).y = newTextPoints.at(i).y/val.at(i);
		newTextPoints.at(i).z = newTextPoints.at(i).z/val.at(i);
	}
	faces = newFaces;
	textPoints = newTextPoints;
}

// linear subdivision happens here
void linearSubdivision()
{
	newTextPoints = textPoints;
	newFaces = vector<vector<int>>();
	for (int i = 0; i<faces.size() ; i++)
	{
		vector<int> edgeMidPoints;
		float centx = 0;
		float centy = 0;
		float centz = 0;
		for (int j = 0; j<4; j++)
		{
			int k = (j+1)%4;
			edgeMidPoints.push_back(getVertex(faces.at(i).at(j)-1,faces.at(i).at(k)-1));
			centx = centx + textPoints.at(faces.at(i).at(j)-1).x;
			centy = centy + textPoints.at(faces.at(i).at(j)-1).y;
			centz = centz + textPoints.at(faces.at(i).at(j)-1).z;
		}
		newTextPoints.push_back(dataPt(centx/4,centy/4,centz/4));
		for (int j = 0; j<4; j++)
		{
			int k;
			vector<int> addnewface;
			if (j == 0)
				k = 3;
			else 
				k = j-1;
			addnewface.push_back(faces.at(i).at(j));
			addnewface.push_back(edgeMidPoints.at(j)+1);
			addnewface.push_back(newTextPoints.size());
			addnewface.push_back(edgeMidPoints.at(k)+1);
			newFaces.push_back(addnewface);
		}
	}
	
	textPoints = newTextPoints;
	faces = newFaces;
}

// reads the vertices and faces of a file
void readFromFile()
{
	string line;
	cout<<name;
	//the variable of type ifstream:
	ifstream myfile (name);
  
	//check to see if the file is opened:
	if (myfile.is_open())
	{
		//while there are still lines in the
		//file, keep reading:
		while (! myfile.eof() )
		{
			//place the line from myfile into the
			//line variable:
			getline (myfile,line);

			istringstream buf(line);
			istream_iterator<std::string> beg(buf), end;

			vector<std::string> tokens(beg, end); // done!

			// storing vertices of coordinates in a vector
			if (tokens.at(0) == "v")
			{
				float fx,fy,fz; 
				istringstream(tokens.at(1)) >> fx; 
				istringstream(tokens.at(2)) >> fy;
				istringstream(tokens.at(3)) >> fz;
				if (max_x < abs(fx))
					max_x = abs(fx);
				if (max_y < abs(fy))
					max_y = abs(fy);
				if (max_z < abs(fz))
					max_z = abs(fz);
				textPoints.push_back(dataPt(fx,fy,fz));
			}
			if (tokens.at(0) == "f")
			{
				int one,two,three,four;
				istringstream(tokens.at(1)) >> one; 
				istringstream(tokens.at(2)) >> two;
				istringstream(tokens.at(3)) >> three;
				istringstream(tokens.at(4)) >> four;
				vector<int> face;
				face.push_back(one);
				face.push_back(two);
				face.push_back(three);
				face.push_back(four);
				faces.push_back(face);
			}
		}
	}
	else cout << "Unable to open file";
}

void drawing()
{
	for (int i = 0; i<faces.size() ; i++)
	{
		glBegin(GL_POLYGON);
			vector<float> normal = normalCalculation(faces.at(i).at(0),faces.at(i).at(1),faces.at(i).at(2));
			glNormal3f(normal.at(0),normal.at(1),normal.at(2));
			glVertex3f( textPoints.at(faces.at(i).at(0)-1).x, textPoints.at(faces.at(i).at(0)-1).y , textPoints.at(faces.at(i).at(0)-1).z);
			glVertex3f( textPoints.at(faces.at(i).at(1)-1).x, textPoints.at(faces.at(i).at(1)-1).y , textPoints.at(faces.at(i).at(1)-1).z );
			glVertex3f( textPoints.at(faces.at(i).at(2)-1).x, textPoints.at(faces.at(i).at(2)-1).y , textPoints.at(faces.at(i).at(2)-1).z );
			glVertex3f( textPoints.at(faces.at(i).at(3)-1).x, textPoints.at(faces.at(i).at(3)-1).y , textPoints.at(faces.at(i).at(3)-1).z );
		glEnd();
		glFlush();
	}
	//glFlush();
	glutSwapBuffers();
}

void display(void)
{
	cout << "As the number of levels increases the process may take more time";
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	// only linear subdivision and averaging
	if (flag_rotate == 0 && flag_zoom == 0)
	{
		newTextPoints = vector<dataPt>();
		newFaces = vector<vector<int>>();
		hashedgecentres = std::map < std::pair < int, int >, int >();
		if (linear_subdivision == 1)
			linearSubdivision();
		if (averag == 1)
			averaging();
	}
	// only zooming
	if (flag_zoom == 1)
	{
		float diff = (finalymouse - initialymouse)/1000.0 ;
		if (finalymouse > initialymouse)
			maximum = maximum + 0.5;
		else
			maximum = maximum - 0.5;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0-maximum, maximum, 0-maximum, maximum, 0-maximum, maximum);
	}
	// only rotation
	if (flag_rotate == 1)
	{
		float diffy =abs( ((finalymouse - initialymouse) * 360.0)/ImageH);
		float diffx =abs( ((finalxmouse - initialxmouse) * 360.0)/ImageW);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		if (diffy > diffx)
		{
			if (finalymouse > initialymouse)
			{
				rotate_x = rotate_x + diffy;
				glRotatef( rotate_x, 1.0, 0.0, 0.0 );
				glRotatef( rotate_y, 0.0, 1.0, 0.0 );
			}
			else
			{
				rotate_x = rotate_x - diffy;
				glRotatef( rotate_x, 1.0, 0.0, 0.0 );
				glRotatef( rotate_y, 0.0, 1.0, 0.0 );
			}
		}
		else
		{
			if (finalxmouse > initialxmouse)
			{
				rotate_y = rotate_y + diffx;
				glRotatef( rotate_y, 0.0, 1.0, 0.0 );
				glRotatef( rotate_x, 1.0, 0.0, 0.0 );
			}
			else 
			{
				rotate_y = rotate_y - diffx;
				glRotatef( rotate_y, 0.0, 1.0, 0.0 );
				glRotatef( rotate_x, 1.0, 0.0, 0.0 );
			}
		}
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0-maximum, maximum, 0-maximum, maximum, 0-maximum, maximum);
		glClearColor (0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	}
	drawing();
	flag_rotate = 0;
	flag_zoom = 0;
	linear_subdivision = 0;
	averag = 0;
	cout << "the rendering is completed";
}

void keyboard ( unsigned char key, int x, int y )           // this function deals with all the keyboard inputs given by the user
{
	cout << "As the number of levels increases the process may take more time";
	switch(key)
	{
		case '+' :
			linear_subdivision = 1;
			averag = 1;
			break;
		case 'L' :
			linear_subdivision = 1;
			break;
		case 'A' :
			averag = 1;
			break;
	}
	glutPostRedisplay ( );
}

// initialization
void init(void)
{
	maximum = 0;
	readFromFile();
	facesConstant = faces;
	textPointsConstant = textPoints;
	if (max_x > max_y)
		maximum = max_x;
	else 
		maximum = max_y;
	if (maximum < max_z)
		maximum = max_z;
	maximum = maximum + 0.5;
	
	glClearColor (0.0, 0.0, 0.0, 0.0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	glOrtho(0-maximum, maximum, 0-maximum, maximum, 0-maximum, maximum);

	glMatrixMode(GL_MODELVIEW);
	gluLookAt(0.0, 0.0, 0-(maximum*10), 0.0, 0.0, -1.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glShadeModel(GL_FLAT);  
	glEnable(GL_NORMALIZE); 
	glEnable(GL_LIGHTING); 	GLfloat diffuse[] = {1,1,1,1}; 
	GLfloat ambient[] = {0.5,0.5,0.5,1}; 
	GLfloat specular[] = {1,1,1,1}; 
 
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse); 
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient); 
	glLightfv(GL_LIGHT0, GL_SPECULAR, specular); 	glEnable(GL_LIGHT0);	GLfloat lightpos[] = {0,0,-2000,1};
	glLightfv(GL_LIGHT0,GL_POSITION, lightpos); 	GLfloat ambient1[] = {.1,0,0,1}; 
	GLfloat specular1[] = {0.1,0.1,0.1,1}; 	GLfloat diffuse1[] = {0.7,0,0,1};	glMaterialfv(GL_FRONT, GL_AMBIENT, ambient1); 
	glMaterialfv(GL_FRONT, GL_SPECULAR, specular1);	glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse1);
}

// mouse movements 
void mouseMove (int button, int state, int x, int y)                          
{
	
	if(button==GLUT_RIGHT_BUTTON && state==GLUT_UP )
	{
		finalxmouse = x;
		finalymouse = y;
		flag_zoom = 1;
		glutPostRedisplay();
	}
	
	if(button == GLUT_RIGHT_BUTTON && state==GLUT_DOWN)
	{
		initialxmouse = x;
		initialymouse = y;
	}
	
	if(button==GLUT_LEFT_BUTTON && state==GLUT_UP) //release
	{
		finalxmouse = x;
		finalymouse = y;
		flag_rotate = 1;
		glutPostRedisplay();
	}
	
	if(button == GLUT_LEFT_BUTTON && state==GLUT_DOWN)
	{
		initialxmouse = x;
		initialymouse = y;
	}
}

// main function
int main(int argc, char** argv)
{
	name=string(argv[1]);
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Rajashree Rao Polsani - Homework 6");
	init();	
	glEnable(GL_DEPTH_TEST);
	glutDisplayFunc(display);
	glutKeyboardFunc ( keyboard );
	glutMouseFunc ( mouseMove );
	glutMainLoop();
	return 0;
}
