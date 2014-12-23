#include "stdafx.h"
#include "PolygonDrawer.h"
#include "ScanConvert.h"
#include <algorithm>
#include <math.h>
#include "ScanConvert.h"
#include <iostream>

float zbuffer[ImageW][ImageH];
// initializes Z value of each pixel
void initialization()
{
	for (int i = 0; i<ImageW ; i++)
	{
		for (int j = 0; j<ImageH ; j++)
		{
			zbuffer[i][j] = 1000;
		}
	}
}

class Edge
{
public:
	float slopeRecip;
	float maxY;
	float currentX;
	vector<float> currentF;
	vector<float> fincr;

	bool operator < ( const Edge &e )
	{
		if ( currentX == e.currentX )
		{
			return slopeRecip < e.slopeRecip;
		}
		else
		{
			return currentX < e.currentX;
		}
	}
};

vector<vector<Edge> > activeEdgeTable;
vector<Edge> activeEdgeList;

// Z-buffering
// This function checks whether a pixel has to be drawn or not depending upon the z value
int drawPixel(vector<float> normal, float x, float y, float xx,float yy, float zz, int row,int column)
{
	float newz = zz - (((x-xx)*normal.at(0) + (y-yy)*normal.at(1))/normal.at(2));
	if (newz <= zbuffer[row][column])
	{
		zbuffer[row][column] = newz;
		return 1;
	}
	else
		return 0;
}

vector<float> lightCalculations1(vector<float> normal)
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

void buildActiveEdgeTable ( vector<Pt> &points, vector<vector<float>> light_components, vector<vector<float>> normal_components, int type_of_shading )
{
	size_t i;

	activeEdgeTable.clear ( );

	// add rows equal to height of image to active edge table
	for ( i = 0; i < ImageH; i++ )
	{
		vector<Edge> row;

		activeEdgeTable.push_back ( row );
	}

	for ( i = 0; i < points.size ( ); i++ )
	{
		Edge e;
		int next = ( i + 1 ) % points.size ( );

		// ignore horizontal lines
		if ( points [ i ].y == points [ next ].y )
		{
			continue;
		}
		e.maxY = (float)max ( points [ i ].y, points [ next ].y );
		e.slopeRecip = ( points [ i ].x - points [ next ].x ) / (float)( points [ i ].y - points [ next ].y );
		if (type_of_shading == 2)
		{
			e.fincr.push_back((light_components.at(i).at(0) - light_components.at(next).at(0) ) /  (float)( points [ i ].y - points [ next ].y ));
			e.fincr.push_back((light_components.at(i).at(1) - light_components.at(next).at(1) ) /  (float)( points [ i ].y - points [ next ].y ));
			e.fincr.push_back((light_components.at(i).at(2) - light_components.at(next).at(2) ) /  (float)( points [ i ].y - points [ next ].y ));
		}
		if (type_of_shading == 3)
		{
			e.fincr.push_back((normal_components.at(i).at(0) - normal_components.at(next).at(0) ) /  (float)( points [ i ].y - points [ next ].y ));
			e.fincr.push_back((normal_components.at(i).at(1) - normal_components.at(next).at(1) ) /  (float)( points [ i ].y - points [ next ].y ));
			e.fincr.push_back((normal_components.at(i).at(2) - normal_components.at(next).at(2) ) /  (float)( points [ i ].y - points [ next ].y ));
		}
		if ( points [ i ].y == e.maxY )
		{
			e.currentX = (float)points [ next ].x;
			if (type_of_shading == 2)
			{
				e.currentF.push_back(light_components.at(next).at(0));
				e.currentF.push_back(light_components.at(next).at(1));
				e.currentF.push_back(light_components.at(next).at(2));
			}
			if (type_of_shading == 3)
			{
				e.currentF.push_back(normal_components.at(next).at(0));
				e.currentF.push_back(normal_components.at(next).at(1));
				e.currentF.push_back(normal_components.at(next).at(2));
			}
			activeEdgeTable [ points [ next ].y ].push_back ( e );
		}
		else
		{
			e.currentX = (float)points [ i ].x;
			if (type_of_shading == 2)
			{
				e.currentF.push_back(light_components.at(i).at(0));
				e.currentF.push_back(light_components.at(i).at(1));
				e.currentF.push_back(light_components.at(i).at(2));
			}
			if (type_of_shading == 3)
			{
				e.currentF.push_back(normal_components.at(i).at(0));
				e.currentF.push_back(normal_components.at(i).at(1));
				e.currentF.push_back(normal_components.at(i).at(2));
			}
			activeEdgeTable [ points [ i ].y ].push_back ( e );
		}
	}
}

// assumes all vertices are within window!!!
void drawPolygon ( vector<Pt> points , vector<float> light, vector<float> normal,  float xx,float yy, float zz ,vector<vector<float>> light_components, vector<vector<float>> normal_components, int type_of_shading)
{
	int x, y;
	size_t i;

	activeEdgeList.clear ( );
	buildActiveEdgeTable ( points,light_components,normal_components,type_of_shading );

	for ( y = 0; y < ImageH; y++ )
	{
		// add edges into active Edge List
		for ( i = 0; i < activeEdgeTable [ y ].size ( ); i++ )
		{
			activeEdgeList.push_back ( activeEdgeTable [ y ] [ i ] );
		}

		// delete edges from active Edge List
		for ( i = 0; i < activeEdgeList.size ( ); i++ )
		{
			if ( activeEdgeList [ i ].maxY <= y )
			{
				activeEdgeList.erase ( activeEdgeList.begin ( ) + i );
				i--;
			}
		}

		// sort according to x value... a little expensive since not always necessary
		sort ( activeEdgeList.begin ( ), activeEdgeList.end ( ) );

		// draw scan line
		if (y>0)
		{
		for ( i = 0; i < activeEdgeList.size ( ); i += 2 )
		{
			vector<float> dF;
			vector<float> F;

			if(type_of_shading == 2 || type_of_shading == 3)
			{
				dF.push_back((activeEdgeList [ i ].currentF.at(0) - activeEdgeList [ i + 1].currentF.at(0))/(activeEdgeList [ i ].currentX - activeEdgeList [ i + 1 ].currentX));
				dF.push_back((activeEdgeList [ i ].currentF.at(1) - activeEdgeList [ i + 1].currentF.at(1))/(activeEdgeList [ i ].currentX - activeEdgeList [ i + 1 ].currentX));
				dF.push_back((activeEdgeList [ i ].currentF.at(2) - activeEdgeList [ i + 1].currentF.at(2))/(activeEdgeList [ i ].currentX - activeEdgeList [ i + 1 ].currentX));

				F.push_back(activeEdgeList [ i ].currentF.at(0) + dF.at(0)*activeEdgeList [ i ].currentF.at(0));
				F.push_back(activeEdgeList [ i ].currentF.at(1) + dF.at(1)*activeEdgeList [ i ].currentF.at(0));
				F.push_back(activeEdgeList [ i ].currentF.at(2) + dF.at(2)*activeEdgeList [ i ].currentF.at(0));
			}

			for ( x = (int)ceil ( activeEdgeList [ i ].currentX ); x < activeEdgeList [ i + 1 ].currentX; x++ )
			{
				if(type_of_shading == 1)
				{
					if (drawPixel(normal, (2*x*1.0/(ImageW - 1))-1, 1-(2*y*1.0/(ImageH - 1)), xx,yy,zz,x,y) == 1)
						setFramebuffer ( x, y, light.at(0), light.at(1), light.at(2) );
				}

				if(type_of_shading == 2)
				{
					if (drawPixel(normal, (2*x*1.0/(ImageW - 1))-1, 1-(2*y*1.0/(ImageH - 1)), xx,yy,zz,x,y) == 1)
						setFramebuffer ( x, y, F.at(0), F.at(1), F.at(2) );
					F.at(0)=(F.at(0) + dF.at(0));
					F.at(1)=(F.at(1) + dF.at(1));
					F.at(2)=(F.at(2) + dF.at(2));
				}

				if(type_of_shading == 3)
				{
					if (drawPixel(normal, (2*x*1.0/(ImageW - 1))-1, 1-(2*y*1.0/(ImageH - 1)), xx,yy,zz,x,y) == 1)
					{
						vector<float> nn;
						float magn = sqrt(F.at(0)*F.at(0) + F.at(1)*F.at(1) + F.at(2)*F.at(2));
						nn.push_back(F.at(0)/magn);
						nn.push_back(F.at(1)/magn);
						nn.push_back(F.at(2)/magn);
						vector<float> colors = lightCalculations1(nn);
						setFramebuffer ( x, y, colors.at(0), colors.at(1), colors.at(2) );
					}
					F.at(0)=(F.at(0) + dF.at(0));
					F.at(1)=(F.at(1) + dF.at(1));
					F.at(2)=(F.at(2) + dF.at(2));
				}
			}
		}
		}

		// update edges in active edge list
		for ( i = 0; i < activeEdgeList.size ( ); i++ )
		{
			activeEdgeList [ i ].currentX += activeEdgeList [ i ].slopeRecip;
			if(type_of_shading == 2 || type_of_shading == 3)
			{
				activeEdgeList [ i ].currentF.at(0) += activeEdgeList [ i ].fincr.at(0);
				activeEdgeList [ i ].currentF.at(1) += activeEdgeList [ i ].fincr.at(1);
				activeEdgeList [ i ].currentF.at(2) += activeEdgeList [ i ].fincr.at(2);
			}
		}
	}
}
