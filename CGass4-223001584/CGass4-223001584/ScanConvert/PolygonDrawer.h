#ifndef POLYGON_DRAWER_H
#define POLYGON_DRAWER_H

#include <vector>

using namespace std;

class Pt
{
public:
	int x, y;

	Pt ( void )
	{
		x = y = 0;
	}

	Pt ( int nX, int nY )
	{
		x = nX;
		y = nY;
	}
};

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

void drawPolygon ( vector<Pt> points , vector<float> light, vector<float> normal, float x,float y, float z, vector<vector<float>> light_components,
	 vector<vector<float>> normal_components,int type_of_shading);
void initialization();

#endif