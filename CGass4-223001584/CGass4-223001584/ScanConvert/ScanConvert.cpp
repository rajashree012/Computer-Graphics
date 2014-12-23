#include "stdafx.h"
#include <Windows.h>
#include <GL/glut.h>
#include <math.h>
#include "ScanConvert.h"
#include "PolygonDrawer.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <iterator>

using namespace std;

/******************************************************************
	Notes:
	Image size is 400 by 400 by default.  You may adjust this if
		you want to.
	You can assume the window will NOT be resized.
	Call clearFramebuffer to clear the entire framebuffer.
	Call setFramebuffer to set a pixel.  This should be the only
		routine you use to set the color (other than clearing the
		entire framebuffer).  drawit() will cause the current
		framebuffer to be displayed.
	As is, your scan conversion should probably be called from
		within the display function.  There is a very short sample
		of code there now.
	You may add code to any of the subroutines here,  You probably
		want to leave the drawit, clearFramebuffer, and
		setFramebuffer commands alone, though.
  *****************************************************************/

float framebuffer[ImageH][ImageW][3];
string name;
// contains points from the file
vector<dataPt> textPoints;
// contains normals at each vertex
vector<vector<float>> vertexnormal;
vector<float> countOfFaces; 
// stores information about faces
vector<dataPt> faces;
int type_of_shading = 1;

// Draws the scene
void drawit(void)
{
   glDrawPixels(ImageW,ImageH,GL_RGB,GL_FLOAT,framebuffer);
   glFlush();
}

// Clears framebuffer to black
void clearFramebuffer()
{
	int i,j;

	for(i=0;i<ImageH;i++) {
		for (j=0;j<ImageW;j++) {
			framebuffer[i][j][0] = 0.0;
			framebuffer[i][j][1] = 0.0;
			framebuffer[i][j][2] = 0.0;
		}
	}
}

// Sets pixel x,y to the color RGB
// I've made a small change to this function to make the pixels match
// those returned by the glutMouseFunc exactly - Scott Schaefer 
void setFramebuffer(int x, int y, float R, float G, float B)
{
	// changes the origin from the lower-left corner to the upper-left corner
	y = ImageH - 1 - y;
	if (R<=1.0)
		if (R>=0.0)
			framebuffer[y][x][0]=R;
		else
			framebuffer[y][x][0]=0.0;
	else
		framebuffer[y][x][0]=1.0;
	if (G<=1.0)
		if (G>=0.0)
			framebuffer[y][x][1]=G;
		else
			framebuffer[y][x][1]=0.0;
	else
		framebuffer[y][x][1]=1.0;
	if (B<=1.0)
		if (B>=0.0)
			framebuffer[y][x][2]=B;
		else
			framebuffer[y][x][2]=0.0;
	else
		framebuffer[y][x][2]=1.0;
}

// returns distance between two points
float distancePoints (dataPt P1, dataPt P2)
{
	return sqrt((P1.x-P2.x)*(P1.x-P2.x) + (P1.y-P2.y)*(P1.y-P2.y) + (P1.z-P2.z)*(P1.z-P2.z) );
}

// gets area of triangular face
float areaOfTriangle(dataPt P1, dataPt P2, dataPt P3)
{
	float s = (distancePoints (P1,P2) + distancePoints (P2,P3) + distancePoints (P3,P1))/2;
	return sqrt(s*(s-distancePoints (P1,P2))*(s-distancePoints (P2,P3))*(s-distancePoints (P3,P1)));
}

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

// does all the light calculations
vector<float> lightCalculations(vector<float> normal)
{
	vector<float> light;
	float red = 0;
	float green = 0;
	float blue = 0;

	// light direction = [-1,-1,1] so in the program we take [1,1,-1]
	// intensity of light = C = [1,1,1]
	
	// ambient light
	// ambient coeff ka = [0.1,0,0] and ambient intensity A = [0.5,0.5,0.5]
	// I = ka*A
	red = red + 0.05;

	// diffuse light
	// diffuse coeff kd = [0.7,0,0]
	// I = C*kd (L.N)
	// considering L and N as unit vectors
	//vector<float> normal = normalCalculation (one,two,three);
	float lightnormal = ((1 * normal.at(0)) + (1 * normal.at(1)) + (-1 * normal.at(2)))/1.732;
	if (lightnormal > 0)
		red = red + 0.7 * lightnormal;

	// specular light
	// viewer unit vector E = [0,0,-1]
	// ks = [0.5,0.5,0.5] n = 5
	// I = C*ks (R.E)^n  ;   R = 2*(L.N)N - L
	if (lightnormal > 0)
	{
		float reflected_magn = sqrt(pow((2*lightnormal*normal.at(0)-1/1.732),2)+pow((2*lightnormal*normal.at(1)-1/1.732),2)+pow((2*lightnormal*normal.at(2)+1/1.732),2));
		float temp = ((-2*lightnormal*normal.at(2) - 1/1.732)/reflected_magn);
		if(temp > 0)
		{
			red = red + 0.5*pow(((-2*lightnormal*normal.at(2) - 1/1.732)/reflected_magn),5);
			green = green + 0.5*pow(((-2*lightnormal*normal.at(2) - 1/1.732)/reflected_magn),5);
			blue = blue + 0.5*pow(((-2*lightnormal*normal.at(2) - 1/1.732)/reflected_magn),5);
		}
	}

	light.push_back(red);
	light.push_back(green);
	light.push_back(blue);
	return light;
}

void readFromFile()
{
	string line;
	cout<<"it takes sometime to show the figure like around 20 sec";
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
			textPoints.push_back(dataPt(fx,fy,fz));
			vector<float> a;
			a.push_back(0);
			a.push_back(0);
			a.push_back(0);
			vertexnormal.push_back(a);
			countOfFaces.push_back(0);
		}

		// calculating normals at each vertex by passing through each face
		if (tokens.at(0) == "f")
		{
			int one,two,three;
			istringstream(tokens.at(1)) >> one; 
			istringstream(tokens.at(2)) >> two;
			istringstream(tokens.at(3)) >> three;
			
			faces.push_back(dataPt(one,two,three));

			float area = areaOfTriangle(textPoints.at(one-1),textPoints.at(two-1),textPoints.at(three-1));

			// calculating weighted normal
			vector<float> normal = normalCalculation(one,two,three);
			vertexnormal.at(one-1).at(0) = vertexnormal.at(one-1).at(0) + normal.at(0)*area;
			vertexnormal.at(one-1).at(1) = vertexnormal.at(one-1).at(1) + normal.at(1)*area;
			vertexnormal.at(one-1).at(2) = vertexnormal.at(one-1).at(2) + normal.at(2)*area;
			countOfFaces.at(one-1) = countOfFaces.at(one-1) + area;

			vertexnormal.at(two-1).at(0) = vertexnormal.at(two-1).at(0) + normal.at(0)*area;
			vertexnormal.at(two-1).at(1) = vertexnormal.at(two-1).at(1) + normal.at(1)*area;
			vertexnormal.at(two-1).at(2) = vertexnormal.at(two-1).at(2) + normal.at(2)*area;
			countOfFaces.at(two-1) = countOfFaces.at(two-1) + area;

			vertexnormal.at(three-1).at(0) = vertexnormal.at(three-1).at(0) + normal.at(0)*area;
			vertexnormal.at(three-1).at(1) = vertexnormal.at(three-1).at(1) + normal.at(1)*area;
			vertexnormal.at(three-1).at(2) = vertexnormal.at(three-1).at(2) + normal.at(2)*area;
			countOfFaces.at(three-1) = countOfFaces.at(three-1) + area;

		}
	}

	for(size_t i = 0; i < vertexnormal.size() ; i++)
	{
		vertexnormal.at(i).at(0) = vertexnormal.at(i).at(0)/countOfFaces.at(i);
		vertexnormal.at(i).at(1) = vertexnormal.at(i).at(1)/countOfFaces.at(i);
		vertexnormal.at(i).at(2) = vertexnormal.at(i).at(2)/countOfFaces.at(i);
		float magn = sqrt(pow(vertexnormal.at(i).at(0),2) + pow(vertexnormal.at(i).at(1),2) + pow(vertexnormal.at(i).at(2),2));
		vertexnormal.at(i).at(0) = vertexnormal.at(i).at(0)/magn;
		vertexnormal.at(i).at(1) = vertexnormal.at(i).at(1)/magn;
		vertexnormal.at(i).at(2) = vertexnormal.at(i).at(2)/magn;
	}
	//close the stream:
	myfile.close();
	}
	else cout << "Unable to open file";
}

void drawing()
{
	for(size_t i = 0 ; i<faces.size();i++)
	{
		int one = (int)faces.at(i).x;
		int two = (int)faces.at(i).y;
		int three = (int)faces.at(i).z;
		vector<Pt> pts;
		pts.push_back(Pt((int)((1+textPoints.at(one-1).x)*0.5*(ImageW-1)),(int)((1-textPoints.at(one-1).y)*0.5*(ImageH-1))));
		pts.push_back(Pt((int)((1+textPoints.at(two-1).x)*0.5*(ImageW-1)),(int)((1-textPoints.at(two-1).y)*0.5*(ImageH-1))));
		pts.push_back(Pt((int)((1+textPoints.at(three-1).x)*0.5*(ImageW-1)),(int)((1-textPoints.at(three-1).y)*0.5*(ImageH-1))));

		vector<float> light = lightCalculations(normalCalculation (one,two,three));
		vector<vector<float>> light_components;
		vector<vector<float>> normal_components;

		// flat shading
		if (type_of_shading == 1)
		{
		
		}

		// Gouraud shading
		if(type_of_shading == 2)
		{
			light_components.push_back(lightCalculations(vertexnormal.at(one-1)));
			light_components.push_back(lightCalculations(vertexnormal.at(two-1)));
			light_components.push_back(lightCalculations(vertexnormal.at(three-1)));
		}

		// phong shading
		if(type_of_shading == 3)
		{
			normal_components.push_back(vertexnormal.at(one-1));
			normal_components.push_back(vertexnormal.at(two-1));
			normal_components.push_back(vertexnormal.at(three-1));
		}
		drawPolygon ( pts,light,normalCalculation (one,two,three), textPoints.at(one-1).x,textPoints.at(one-1).y,textPoints.at(one-1).z,light_components
			,normal_components,type_of_shading);
	}
	drawit();
}

void display(void)
{
	clearFramebuffer();
	drawit();
	drawing();
}

void keyboard ( unsigned char key, int x, int y )           // this function deals with all the keyboard inputs given by the user
{
	switch ( key )
	{
		case '1' : 
			type_of_shading = 1;                          // flat shading
			break;
		case '2' :   
			type_of_shading = 2;                          // Gouraud shading
			break;
		case '3' : 
			type_of_shading = 3;                           // Phong shading
			break;
	}
	glutPostRedisplay ( );
}

void init(void)

{
	readFromFile();
}

int main(int argc, char** argv)
{
	name=string(argv[1]);
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Rajashree Rao Polsani - Homework 4");
	initialization();
	init();	
	glutDisplayFunc(display);
	glutKeyboardFunc ( keyboard );
	glutMainLoop();
	return 0;
}
