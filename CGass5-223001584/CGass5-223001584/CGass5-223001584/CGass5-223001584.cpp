#include "stdafx.h"
#include "vectors.h"
#include "shapes.h"
#include <vector>
#include <Windows.h>
#include <GL/glut.h>
#include <math.h>
#include <iostream>
#include<string>

# define ImageH 400
# define ImageW 400
using namespace std;

vector<CommonShape*> list_of_objects;
int maxiter = 0;
int scene = 0;
vector<Point3D> light_sources;
int flag_ambient = 0;
int flag_shadow = 0;

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
	//y = ImageH - 1 - y;
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

Color lightCalculations1(Point3D normal,int id,Point3D pt,Point3D ambient_coeff,Point3D diffuse_coeff,Point3D specular_coeff,int specular_expo,Point3D lightss)
{
	Color light;
	float red = 0;
	float green = 0;
	float blue = 0;

	Point3D intensity(1,1,1);

	// ambient light calculations
	Point3D ambient_intensity(0.5,0.5,0.5);

	if(flag_ambient == 0)
	{
		red = red + ambient_coeff.x*ambient_intensity.x ;
		green = green + ambient_coeff.y*ambient_intensity.y ;
		blue = blue + ambient_coeff.z*ambient_intensity.z ;
	}
	flag_ambient = 1;

	// specular and diffusion calculations
	Point3D lights;
	Point3D normals;

	Point3D light_dir(vecSubtraction(lightss,pt));
	lights = light_dir;

	// diffuse light
	lights = normalize(lights);
	normals = normalize(constMultiplication(normal,1)); 
	double lightnormal = dotProduct(lights,normals);
	if (lightnormal >= 0)
	{
		red = red + lightnormal*intensity.x*diffuse_coeff.x;
		green = green + lightnormal*intensity.y*diffuse_coeff.y;
		blue = blue + lightnormal*intensity.z*diffuse_coeff.z;

		// specular light
		Point3D reflected = vecSubtraction(constMultiplication(normals,2*lightnormal),lights);
		reflected = normalize(reflected);
		Point3D eyes = normalize(vecSubtraction(eye,pt));
		double eye_reflected = pow(dotProduct(reflected,eyes),specular_expo);
		if(eye_reflected >= 0)
		{
			red = red + eye_reflected*intensity.x*specular_coeff.x;
			green = green + eye_reflected*intensity.y*specular_coeff.y;
			blue = blue + eye_reflected*intensity.z*specular_coeff.z;
		}
	}

	light.red = red;
	light.blue = blue;
	light.green = green;
	return light;
}

// trying to find the closest intersection for a ray
int getClosestObject(vector<Point3D> intersections)
{
	float length=0;
	float minLen=INT_MAX;
	int position = -1;
	for (int i = 0; i < intersections.size(); i++)
	{
		if(intersections[i].x == -999 && intersections[i].y == -999 && intersections[i].z == -999)	
			continue;
		length = magnVector(vecSubtraction(intersections[i],eye));
		if(length < minLen)
		{
			minLen=length;
			position=i;
		}
	}
	return position;
}

// checking whether a given pixel falls into shadow or not
bool shadow_cal(rayProp obj_light,vector<CommonShape*> list_of_objects,int id)
{
	double dist_obj_light = distVectors(obj_light.startPoint,obj_light.endPoint);
	for(int i = 0;i<list_of_objects.size();i++)
	{
		if (id == list_of_objects[i]->getId())
			continue;
		Point3D intersect = list_of_objects[i]->getIntersectionPoint(obj_light);
		double dist_obj_otherobj = distVectors(obj_light.startPoint,intersect);
		if (dist_obj_otherobj < dist_obj_light)
			return false;
	}
	return true;
}

//  given starting light source , incident point and normal...it calculates reflected ray
rayProp reflectedRay(Point3D start, Point3D pt, Point3D normal)
{
	Point3D incident_dir(vecSubtraction(start,pt));
	incident_dir = normalize(incident_dir);
	Point3D normals = normalize(normal);
	double incidentnormal = dotProduct(incident_dir,normals);
	Point3D reflected = vecSubtraction(constMultiplication(normals,2*incidentnormal),incident_dir);
	reflected = normalize(reflected);
	return rayProp(pt,vecAddition(pt,reflected));
}

// recursive function to calculate reflections
Color rayTracer(rayProp ray, int recursionLevel, Point3D start,Point3D light)
{
	if(recursionLevel >= maxiter)
		return Color(0,0,0);
	else
	{
		// trying to find intersection of ray with all the objects in the scene
		vector<Point3D> intersections;
		for (int ii = 0; ii < list_of_objects.size(); ii++)
		{
			Point3D interPoint=list_of_objects[ii]->getIntersectionPoint(ray);
			intersections.push_back(interPoint);
		}
		// choosing the best intersection
		int bestIndex = getClosestObject(intersections);
		if(bestIndex==-1)
			return Color(0,0,0);
		int id = list_of_objects[bestIndex]->getId();
		// checking for shadows
		if(flag_shadow == 0)
		{
			if (!shadow_cal(rayProp(intersections.at(bestIndex),light),list_of_objects,id))
				return Color(0,0,0);
		}
		Point3D tot_color(0,0,0);
		// sphere
		if (id == 0)
		{
			Sphere *sphere = ((Sphere*)list_of_objects[bestIndex]);
			Point3D normal = sphere->getNormal(intersections.at(bestIndex));
			// checkingg condition for reflection
			if (dotProduct(vecSubtraction(light,intersections.at(bestIndex)),normal) <= 0)
				return Color(0,0,0);
			// calculating the reflected ray
			rayProp reflected = reflectedRay(start,intersections.at(bestIndex),normal);
			Color returned = rayTracer(reflected,recursionLevel+1,intersections.at(bestIndex),light);
			Point3D xxxx(returned.red,returned.green,returned.blue);
			// calculating the reflected color
			tot_color = constMultiplication(xxxx,sphere->ref_coeff);
			Color lightcal = lightCalculations1(normal,id,intersections.at(bestIndex),sphere->ambient_coeff,sphere->diffuse_coeff,sphere->specular_coeff,sphere->spec_expo,start);
			// calculating the total colr
			Color c = Color(vecAddition(Point3D(lightcal.red,lightcal.green,lightcal.blue),tot_color));
			if (flag_shadow == 0)
			{
				if (!shadow_cal(rayProp(intersections.at(bestIndex),light),list_of_objects,id))
					return Color(0,0,0);
				else
					return c;
			}
			else
				return c;
		}
		// plane
		if (id == 1)
		{
			Plane *plane = ((Plane*)list_of_objects[bestIndex]);
			Point3D normal = plane->getNormal(); 
			if (dotProduct(vecSubtraction(light,intersections.at(bestIndex)),normal) <= 0)
				return Color(0,0,0);
			rayProp reflected = reflectedRay(start,intersections.at(bestIndex),normal);
			Color returned = rayTracer(reflected,recursionLevel+1,intersections.at(bestIndex),light);
			Point3D xxxx(returned.red,returned.green,returned.blue);
			tot_color = constMultiplication(xxxx,plane->ref_coeff);
			Color lightcal = lightCalculations1(normal,id,intersections.at(bestIndex),plane->ambient_coeff,plane->diffuse_coeff,plane->specular_coeff,plane->spec_expo,start);
			Color c = Color(vecAddition(Point3D(lightcal.red,lightcal.green,lightcal.blue),tot_color));
			if (flag_shadow == 0)
			{
				if (!shadow_cal(rayProp(intersections.at(bestIndex),light),list_of_objects,id))
					return Color(0,0,0);
				else
					return c;
			}
			else
				return c;
		}
		// cylinder
		if (id == 2)
		{
			Cylinder *cylinder = ((Cylinder*)list_of_objects[bestIndex]);
			Point3D normal = cylinder->getNormal(intersections.at(bestIndex)); 
			if (dotProduct(vecSubtraction(light,intersections.at(bestIndex)),normal) <= 0)
				return Color(0,0,0);
			rayProp reflected = reflectedRay(start,intersections.at(bestIndex),normal);
			Color returned = rayTracer(reflected,recursionLevel+1,intersections.at(bestIndex),light);
			Point3D xxxx(returned.red,returned.green,returned.blue);
			tot_color = constMultiplication(xxxx,cylinder->ref_coeff);
			Color lightcal = lightCalculations1(normal,id,intersections.at(bestIndex),cylinder->ambient_coeff,cylinder->diffuse_coeff,cylinder->specular_coeff,cylinder->spec_expo,start);
			Color c = Color(vecAddition(Point3D(lightcal.red,lightcal.green,lightcal.blue),tot_color));
			if (flag_shadow == 0)
			{
				if (!shadow_cal(rayProp(intersections.at(bestIndex),light),list_of_objects,id))
					return Color(0,0,0);
				else
					return c;
			}
			else
				return c;
		}
		// ellipsoid
		if (id == 3)
		{
			Ellipsoid *ellipsoid = ((Ellipsoid*)list_of_objects[bestIndex]);
			Point3D normal = ellipsoid->getNormal(intersections.at(bestIndex)); 
			if (dotProduct(vecSubtraction(light,intersections.at(bestIndex)),normal) <= 0)
				return Color(0,0,0);
			rayProp reflected = reflectedRay(start,intersections.at(bestIndex),normal);
			Color returned = rayTracer(reflected,recursionLevel+1,intersections.at(bestIndex),light);
			Point3D xxxx(returned.red,returned.green,returned.blue);
			tot_color = constMultiplication(xxxx,ellipsoid->ref_coeff);
			Color lightcal = lightCalculations1(normal,id,intersections.at(bestIndex),ellipsoid->ambient_coeff,ellipsoid->diffuse_coeff,ellipsoid->specular_coeff,ellipsoid->spec_expo,start);
			Color c = Color(vecAddition(Point3D(lightcal.red,lightcal.green,lightcal.blue),tot_color));
			if (flag_shadow == 0)
			{
				if (!shadow_cal(rayProp(intersections.at(bestIndex),light),list_of_objects,id))
					return Color(0,0,0);
				else
					return c;
			}
			else
				return c;
		}
	}
}

void rayTracing()
{
	int ActualWindowH = ImageH;
	int ActualWindowW = ImageW;

	// assuming virtual screen is a square
	float virtualScreenX = 15;
	float virtualScreenY = 15;

	// creating normals near the eye position
	// up -- y ; right -- x; into the screen --- z
	Point3D normal_y(0,1,0) ;
	Point3D normal_x(1,0,0) ;
	Point3D normal_z(0,0,1) ;

	// eye is at the centre of the virtual screen
	// distance between eye and screen is d
	int d = 4;

	Point3D virtual_centre = vecAddition(constMultiplication(normal_z,d),eye);

	// Left most lowest corner of virtual screen
	Point3D vir_left_lower = vecSubtraction(virtual_centre,vecAddition(constMultiplication(normal_x,virtualScreenX/2),constMultiplication(normal_y,virtualScreenY/2)));

	if(scene == 1)
	{
		Sphere sphere1(3.0,Point3D(0,0,10),0,Point3D(0.1,0.1,0.1),Point3D(0.1,0.1,0.1),Point3D(0.5,0.5,0.5),5,0.7);
		Plane plane1(Point3D(0,1,0), Point3D(0,-15,0),1,Point3D(0,0.1,0),Point3D(0,0.7,0),Point3D(0.5,0.2,0.7),15,0.0);
		Cylinder cylinder1(1.0,Point3D(0,0,8),Point3D(0,1,0),2,Point3D(0,0.2,0),Point3D(0,0.7,0.0),Point3D(0.5,0.5,0.5),13,0.0);
		Ellipsoid ellip1(Point3D(-2,-6,11), 3.0,3.0,4.0,3,Point3D(0,0,0.1),Point3D(0,0,0.7),Point3D(0.5,0.5,0.5),5,0.7);

		// enter all the objects present in the scene
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&plane1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&ellip1));

		// adding light sources
		light_sources.push_back(Point3D(2,4,3));
		light_sources.push_back(Point3D(-7,3,2));

		maxiter = 1;
	}

	if(scene == 2)
	{
		Sphere sphere1(3.0,Point3D(-5,0,10),0,Point3D(0.1,0.0,0.1),Point3D(0.1,0.0,0.1),Point3D(0.5,0.5,0.5),5,0.7);
		Plane plane1(Point3D(0,1,0), Point3D(0,-6,0),1,Point3D(0,0.1,0),Point3D(0,0.7,0),Point3D(0.5,0.2,0.7),15,0.0);
		Cylinder cylinder1(1.0,Point3D(0,0,8),Point3D(0,1,0),2,Point3D(0.5,0.9,0.9),Point3D(0.5,0.9,0.9),Point3D(0.5,0.5,0.5),13,0.0);

		// enter all the objects present in the scene
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&plane1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder1));

		// adding light sources
		light_sources.push_back(Point3D(2,4,3));
		light_sources.push_back(Point3D(-7,3,2));

		maxiter = 1;
	}

	if(scene == 3)
	{
		Sphere sphere1(3.0,Point3D(0,0,10),0,Point3D(0.1,0.1,0.1),Point3D(0.7,0.7,0.7),Point3D(0.5,0.5,0.5),5,0.7);
		Plane plane1(Point3D(0,1,0), Point3D(0,-15,0),1,Point3D(0,0.1,0),Point3D(0,0.7,0),Point3D(0.5,0.2,0.7),15,0.0);
		Cylinder cylinder1(2.0,Point3D(0,0,10),Point3D(0,1,0),2,Point3D(0.5,0.9,0.9),Point3D(0.5,0.9,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Ellipsoid ellip1(Point3D(6,0,9), 3.0,3.0,4.0,3,Point3D(0,0,0.1),Point3D(0,0,0.7),Point3D(0.5,0.5,0.5),5,0.0);

		// enter all the objects present in the scene
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&plane1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&ellip1));

		// adding light sources
		light_sources.push_back(Point3D(0,0,0));

		maxiter = 3;
		flag_shadow = 1;
	}

	if(scene == 4)
	{
		cout << "scene 4 takes 1 minute to render";
		Sphere sphere1(8.0,Point3D(0,0,12),0,Point3D(1,1,1),Point3D(1,1,1),Point3D(1,1,1),5,0.7);
		Sphere sphere5(10.0,Point3D(0,0,25),0,Point3D(0,0,0),Point3D(0,0,0),Point3D(0,0,0),5,0.7);
		Plane plane1(Point3D(0,1,0), Point3D(0,-15,0),1,Point3D(0,0,0.1),Point3D(0,0,0.7),Point3D(0.5,0.2,0.7),15,0.0);
		Plane plane2(Point3D(0,0,-1), Point3D(0,0,20),1,Point3D(0.1,0,0),Point3D(0.7,0,0),Point3D(0.5,0.2,0.7),15,0.0);
		Sphere sphere2(0.20,Point3D(-1.5,1,4),0,Point3D(0,0,0),Point3D(0,0,0),Point3D(0,0,0),5,0.7);
		Sphere sphere3(0.20,Point3D(1.5,1,4),0,Point3D(0,0,0),Point3D(0,0,0),Point3D(0,0,0),5,0.7);
		Sphere sphere4(0.20,Point3D(0,-0.5,4),0,Point3D(0,0,0),Point3D(0,0,0),Point3D(0,0,0),5,0.7);
		Ellipsoid ellip1(Point3D(0,-2,4), 0.50,0.10,0.10,3,Point3D(0,0,0),Point3D(0,0,0),Point3D(0,0,0),5,0.0);
		Cylinder cylinder1(5.0,Point3D(0,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder2(5.0,Point3D(-10,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder3(5.0,Point3D(-20,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder4(5.0,Point3D(-20,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder5(5.0,Point3D(-30,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder6(5.0,Point3D(-40,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder7(5.0,Point3D(10,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder8(5.0,Point3D(20,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder9(5.0,Point3D(30,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);
		Cylinder cylinder10(5.0,Point3D(40,0,25),Point3D(0,1,0),2,Point3D(0.5,0.4,0.9),Point3D(0.5,0.4,0.9),Point3D(0.5,0.5,0.5),13,0.0);

		// enter all the objects present in the scene
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&plane1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere2));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere3));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere4));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&sphere5));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&ellip1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder1));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder2));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder3));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder4));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder5));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder6));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder7));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder8));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder9));
		list_of_objects.push_back(dynamic_cast<CommonShape*>(&cylinder10));

		// adding light sources
		light_sources.push_back(Point3D(0,0,0));
		light_sources.push_back(Point3D(0,0,0));

		maxiter = 2;
	}

	int Number_of_objects = list_of_objects.size();	
	
	// checking whether eye is inside any of the objects
	for (int i = 0; i < list_of_objects.size(); i++)
	{
		if(!list_of_objects[i]->isEyeOutside(eye))
		{	
			cout<<"Eye is inside the object \n";
			return;
		}
	}
	int count = 0;
	// drawing the color of pixels
	for (int i = 0; i<ImageW ; i++)
	{
		for(int j = 0; j<ImageH ; j++)
		{
			double x=(i+0.5)/ImageW;
			double y=(j+0.5)/ImageH;
			Point3D endpoint = vecAddition(vir_left_lower,vecAddition(constMultiplication(normal_x,(x*virtualScreenX)),constMultiplication(normal_y,(y*virtualScreenY))));
			rayProp current(eye,endpoint);
			Color c(0,0,0);
			// the ray is made to intersect each object and all the intersection points are collected
			for (int k = 0; k<light_sources.size(); k++)
			{
				c=c.addColor(rayTracer(current,0,light_sources.at(k),light_sources.at(k)));
			}
			setFramebuffer(i,j,c.red,c.green,c.blue);
		}
	}
	flag_ambient = 0;
	flag_shadow = 0;
	light_sources.clear();
	list_of_objects.clear();
}

void keyboard ( unsigned char key, int x, int y )           // this function deals with all the keyboard inputs given by the user
{
	switch ( key )
	{
		case '1' : 
			scene = 1;                          // scene1
			break;
		case '2' :   
			scene = 2;                          // scene2
			break;
		case '3' : 
			scene = 3;                           // scene3
			break;
		case '4' : 
			scene = 4;                           // scene4
			break;
	}
	glutPostRedisplay ( );
}

void display(void)
{
	clearFramebuffer();
	drawit();
	rayTracing();
	drawit();
}

int main(int argc, char** argv)
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Rajashree Rao Polsani - Homework 5");
	glutDisplayFunc(display);
	glutKeyboardFunc ( keyboard );
	glutMainLoop();
	return 0;
}
