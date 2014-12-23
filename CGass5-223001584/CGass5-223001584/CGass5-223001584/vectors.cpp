#include "stdafx.h"
#include "vectors.h"
#include <math.h>

// magnitude of vector
float magnVector (Point3D p)
{
	return sqrt(p.x*p.x + p.y*p.y + p.z*p.z) ;
}

// normalizing of vector
Point3D normalize (Point3D p)
{
	return constDivision(p,magnVector(p));
}
	
// vector addition
Point3D vecAddition (Point3D p1, Point3D p2)
{
	return Point3D(p1.x+p2.x,p1.y+p2.y,p1.z+p2.z);
}

// vector subtraction
Point3D vecSubtraction (Point3D p1, Point3D p2)
{
	return Point3D(p1.x-p2.x,p1.y-p2.y,p1.z-p2.z);
}

// dot product of two vectors
float dotProduct(Point3D p1, Point3D p2)
{
	return p1.x*p2.x + p1.y*p2.y + p1.z*p2.z ;
}

// cross product
Point3D crossProduct(Point3D p1, Point3D p2)
{
		return Point3D(p1.y*p2.z-p1.z*p2.y, p1.z*p2.x-p1.x*p2.z, p1.x*p2.y-p1.y*p2.x);
}

// multiply a vector with constant
Point3D constMultiplication (Point3D p, float n)
{
	return Point3D(p.x*n,p.y*n,p.z*n);
}

// divide a vector with a constant
Point3D constDivision (Point3D p, float n)
{
	return Point3D(p.x/n,p.y/n,p.z/n);
}

// distance between two vectors
double distVectors(Point3D p1, Point3D p2)
{
	return sqrt((p1.x-p2.x)*(p1.x-p2.x) + (p1.y-p2.y)*(p1.y-p2.y) + (p1.z-p2.z)*(p1.z-p2.z));
}