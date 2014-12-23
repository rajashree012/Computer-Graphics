#ifndef VECTORS_H
#define VECTORS_H

# include <math.h>

class Point3D
{
	public :
		float x;
		float y;
		float z;

		Point3D()
		{
			x = y = z = 0;
		}
		Point3D(float xx, float yy, float zz)
		{
			x = xx;
			y = yy;
			z = zz;
		}
};

// all the vector operations are defined in this header file
float magnVector (Point3D p);

Point3D normalize (Point3D p);
	
Point3D vecAddition (Point3D p1, Point3D p2);

Point3D vecSubtraction (Point3D p1, Point3D p2);

float dotProduct(Point3D p1, Point3D p2);

Point3D crossProduct(Point3D p1, Point3D p2);

Point3D constMultiplication (Point3D p, float n);

Point3D constDivision (Point3D p, float n);

double distVectors(Point3D p1, Point3D p2);

#endif