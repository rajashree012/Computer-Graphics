//NOTE : IT MAY TAKE UPTO 2-3 SECONDS TO SHOW THE CLIPPED POLYGONS

#include "stdafx.h"
#include <stdlib.h>
#include <stdio.h>
#include <gl/glut.h>
#include <gl/gl.h>
#include <math.h>


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

#define ImageW 400
#define ImageH 400

// stores the current pixel values
float framebuffer[ImageH][ImageW][3];
// stores the pixel values when all the original polygons are entered
float framebufferstore[ImageH][ImageW][3];
// stores the x coordinate of current mouse position
float xmouse = 0;            
// stores the y coordinate of current mouse position
float ymouse = 0; 
// indicates end of the polygon
int flag_end_of_polygon = 0;
// indicates start of the polygon
int flag_start_of_polygon = 1;
int num_of_polygon = 0;
int flag_start_clipping = 0;
int flag_start_draw_polygons = 1;
// flag_clearing is used to clear the screen when set to 1
int flag_clearing = 1;       
// this flag is set after clipping rectangle has been captured
int flag_capture = 0;
// end coordinates of the clipping rectangle
float capture_x = 0;
float capture_x_final = 0;
float capture_y_final = 0;
float capture_y = 0;
// ymax and ymin for scanning a polygon
int ymin = ImageH;
int ymax = 0;
int flag = 0;

// colors for the 10 polygons
float colors [10][3] = {{1.0,0.0,0.0},{0.0,1.0,0.0},{0.0,0.0,1.0},
						{1.0,1.0,0.0},{1.0,0.0,1.0},{0.0,1.0,1.0},
						{1.0,0.5,0.0},{0.5,1.0,0.5},{0.0,0.5,1.0},
						{1.0,1.0,1.0}};

struct color {
	float r, g, b;		// Color (R,G,B values)
};

// structure for vertex of each polygon
struct vertex
{
	int num_vertex;
	float x_coord;
	float y_coord;
};

// structure of vertex while clipping
struct vertexNew
{
	int num_vertex;
	float x_coord;
	float y_coord;
	struct vertexNew *next;
};

// structure for polygon
struct polygon
{
	int num_polygon;
	int num_vertices;
	struct vertex vertices[100];
};

// array to store all the polygons drawn
struct polygon polygons[10];
// array which stores polygons after clipping has been done
struct polygon polygon_clip[10];
// head pointer for the active edge table
struct activeEdge *root_active_edge_table = NULL;

// structure for edge
struct edge
{
	float xcoord1;
	float ycoord1;
	float xcoord2;
	float ycoord2;
	float maxY;
	float currentX;
	float xIncr;
	struct edge *next;
};

// structure for storing the edges for each scan line during creation of active edge table
struct activeEdge
{
	int scan_line;
	struct edge *head;
	struct activeEdge *next;
};

// Draws the scene
void drawit(void)
{
   glDrawPixels(ImageW,ImageH,GL_RGB,GL_FLOAT,framebuffer);
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

// sorting has to be done on active edge list based on currentX values during scanning of polygons
struct edge *bubblesort(struct edge *list)
{
	struct edge *lst = NULL, *tmp = list, *prev = NULL, *potentialprev = list;
	int idx, idx2, n = 0;
 
	//determine total number of nodes
	for (;tmp != NULL; tmp=tmp->next)
	{
		n++;
	}
	for (idx=0; idx<n-1; idx++)
	{
		for (idx2=0,lst=list;
				lst && lst->next && (idx2<=n-1-idx);
				idx2++)
		{
			if (!idx2)
			{
				//we are at beginning, so treat start 
				//node as prev node
				prev = lst;
			}
     
			//compare the two neighbors
			if (lst->next->currentX < lst->currentX)
			{ 
				//swap the nodes
				tmp = (lst->next!=NULL?lst->next->next:NULL);
               
				if (!idx2 && (prev == list))
				{
					//we do not have any special sentinal nodes
					//so change beginning of the list to point 
					//to the smallest swapped node
					list = lst->next;
				}
				potentialprev = lst->next;
				prev->next = lst->next;
				lst->next->next = lst;
				lst->next = tmp;
				prev = potentialprev;
			}
			else
			{
			lst = lst->next;
				if(idx2)
				{
					//just keep track of previous node, 
					//for swapping nodes this is required
					prev = prev->next;
				}
			}    
		}
	}
	return list;
}

// This function creates an active edge table. Corresponding to each scan line a list of edges starting at that scan line are stored.
void activeEdgeTable(struct activeEdge *root,int yMax,int yMin,int polygon_num,struct polygon polygons[])
{   
	int yminimum = 0;
	for(int i = yMin; i <= yMax ;i++)
	{
		root->scan_line = i;
		if (i == yMax)
			root->next = NULL;
		else
			root->next = (struct activeEdge*)malloc(sizeof(struct activeEdge));
		root->head = NULL;
		for (int j = 0; j < polygons[polygon_num].num_vertices; j++)
		{
			int k=0;
			if (j==polygons[polygon_num].num_vertices-1)
				k=0;
			else
				k=j+1;
			if (polygons[polygon_num].vertices[j].y_coord > polygons[polygon_num].vertices[k].y_coord)
				yminimum = polygons[polygon_num].vertices[k].y_coord;
			else if(polygons[polygon_num].vertices[j].y_coord < polygons[polygon_num].vertices[k].y_coord)
				yminimum = polygons[polygon_num].vertices[j].y_coord;
			else
				yminimum = -1;               // horizontal edges are neglected
			if(yminimum == i)
			{
				struct edge *temp = root-> head;
				if (temp == NULL)
				{
					temp = (struct edge*)malloc(sizeof(struct edge));
					root->head = temp;
				}
				else 
				{
					while(temp->next != NULL)
						temp = temp ->next;
					temp->next = (struct edge*)malloc(sizeof(struct edge));
					temp = temp->next;
				}
				temp->xcoord1 = polygons[polygon_num].vertices[j].x_coord;
				temp->xcoord2 = polygons[polygon_num].vertices[k].x_coord;
				temp->ycoord1 = polygons[polygon_num].vertices[j].y_coord;
				temp->ycoord2 = polygons[polygon_num].vertices[k].y_coord;
				if (temp->ycoord1 < temp->ycoord2)
				{
					temp->currentX = temp->xcoord1;
					temp->maxY = temp->ycoord2;
				}
				else
				{
					temp->currentX = temp->xcoord2;
					temp->maxY = temp->ycoord1;
				}
				temp->xIncr=((temp->xcoord2-temp->xcoord1)*1.0)/(temp->ycoord2-temp->ycoord1);
				temp->next = NULL;
			}
		}
		root = root -> next;
	}
}

// algorithm for scanning of polygons is implemented
void scanningAlgorithm(struct activeEdge *root_active_edge_table,int yMax,int yMin,int polygon_num,struct polygon polygons[])
{
	drawit();
	struct edge *activeEdgeList = NULL;
	struct edge *activeEdgeListroot = NULL;
	struct edge *parent = NULL;
	for(int i = yMin; i <= yMax ;i++)
	{
		struct edge *temp2 = root_active_edge_table->head;
		if(temp2 != NULL)
		{
			// edges starting at the current scanline are added to the active edge list
			while(temp2!=NULL)
			{
				activeEdgeList = activeEdgeListroot;
				parent = NULL;
				while(activeEdgeList != NULL)
				{
					if (activeEdgeList->currentX <= temp2->currentX)
					{
						parent = activeEdgeList;
						activeEdgeList = activeEdgeList->next;
					}
					else
						break;
				}
				if (activeEdgeListroot == NULL)
				{
					activeEdgeList = (struct edge*)malloc(sizeof(struct edge));
					activeEdgeListroot = activeEdgeList;
					activeEdgeList->next = NULL;
				}
				else
				{
					if(parent == NULL)
					{
						activeEdgeList = (struct edge*)malloc(sizeof(struct edge));
						activeEdgeList->next = activeEdgeListroot;
						activeEdgeListroot = activeEdgeList;
					}
					else
					{
						parent->next = (struct edge*)malloc(sizeof(struct edge));
						if(activeEdgeList != NULL)
							parent -> next -> next = activeEdgeList;
						else
							parent -> next -> next = NULL;
						activeEdgeList = parent->next;
					}
				}
				activeEdgeList->xcoord1 = temp2->xcoord1;
				activeEdgeList->xcoord2 = temp2->xcoord2;
				activeEdgeList->ycoord1 = temp2->ycoord1;
				activeEdgeList->ycoord2 = temp2->ycoord2;
				activeEdgeList->currentX = temp2->currentX;
				activeEdgeList->maxY = temp2->maxY;
				activeEdgeList->xIncr = temp2->xIncr;
				temp2=temp2->next;
			}
		}
		root_active_edge_table = root_active_edge_table->next;
		// removing edges that end at this scan line
		activeEdgeList = activeEdgeListroot;
		parent = NULL;
		while (activeEdgeList != NULL)
		{
			if (activeEdgeList->maxY == i)
				if(activeEdgeListroot == activeEdgeList)
				{
					activeEdgeListroot = activeEdgeList ->next;
					parent = activeEdgeList;
				}
				else
					parent->next = activeEdgeList->next;
			else
				parent = activeEdgeList;
			activeEdgeList = activeEdgeList->next;
		}
		// filling the pixels
		activeEdgeList = activeEdgeListroot;
		float start =0;
		float end = 0;
		while (activeEdgeList != NULL)
		{
			start = activeEdgeList->currentX;
			if(activeEdgeList->next != NULL)
				end = activeEdgeList->next->currentX;
			else 
				end = start;
			for(int ii=ceil(start);ii<end;ii++) 
			{
				setFramebuffer(ii,i,colors[polygon_num][0],colors[polygon_num][1],colors[polygon_num][2]);
			}
			drawit();
			if(activeEdgeList->next != NULL)
				activeEdgeList = activeEdgeList->next->next;
			else
				activeEdgeList = activeEdgeList->next;
		}
		struct edge *activeEdgeList = activeEdgeListroot;
		// updating the values of currentX
		while(activeEdgeList !=NULL)
		{
			activeEdgeList->currentX = activeEdgeList->currentX + activeEdgeList->xIncr;
			activeEdgeList = activeEdgeList->next;
		}
		activeEdgeListroot = bubblesort(activeEdgeListroot);
	}
}

// This function creates active edge table and calls the scanning function
void scanningPolygon(int yMax,int yMin,int polygon_num,struct polygon polygons[])
{
	flag_end_of_polygon = 0;
	flag_start_of_polygon = 1;
	root_active_edge_table = NULL;
	root_active_edge_table = (struct activeEdge*)malloc(sizeof(struct activeEdge));
	activeEdgeTable(root_active_edge_table,yMax,yMin,polygon_num,polygons);
	scanningAlgorithm(root_active_edge_table,yMax,yMin,polygon_num,polygons);
}

// This function stores information regarding each polygon like coordinates of its vertices
void drawPolygon (void)
{
	if (flag_start_of_polygon == 1 && flag_end_of_polygon != 1)
	{
		polygons[num_of_polygon].num_polygon = num_of_polygon;
		polygons[num_of_polygon].num_vertices = 0;
		flag_start_of_polygon = 0;
	}
	polygons[num_of_polygon].vertices[polygons[num_of_polygon].num_vertices].num_vertex = polygons[num_of_polygon].num_vertices;
	polygons[num_of_polygon].vertices[polygons[num_of_polygon].num_vertices].x_coord = xmouse;
	polygons[num_of_polygon].vertices[polygons[num_of_polygon].num_vertices].y_coord = ymouse;
	polygons[num_of_polygon].num_vertices++;
	if (ymouse >= ymax)
		ymax = ymouse;
	if (ymouse <= ymin)
		ymin = ymouse;
	if (flag_end_of_polygon == 1)
		num_of_polygon++;
	glColor3f (1.0,0.0,0.0);
	glBegin (GL_POINTS);
		glVertex2f(xmouse,ymouse);
	glEnd();
	glFlush();
	if (flag_end_of_polygon == 1)
	{
		scanningPolygon(ymax,ymin,num_of_polygon-1,polygons);
		ymin = ImageH;
		ymax = 0;
		free(root_active_edge_table);
	}
}

// clipping algorithm
void clippingProcess()
{
	// taking care of the clipping window coordinates
	float x1 = 0.0,x2 = 0.0,y1 = 0.0,y2 = 0.0;
	int flag_set = 0;
	if(capture_x > capture_x_final)
	{
		x1 = capture_x_final;
		x2 = capture_x;
	}
	else 
	{
		x1 = capture_x;
		x2 = capture_x_final;
	}
	if(capture_y > capture_y_final)
	{
		y1 = capture_y_final;
		y2 = capture_y;
	}
	else 
	{
		y1 = capture_y;
		y2 = capture_y_final;
	}
	for (int i=0; i<num_of_polygon ; i++)
	{
		//storing the vertices of polygon in linked list
		struct vertexNew *original = NULL;
		original = (struct vertexNew*)malloc(sizeof(struct vertexNew));
		struct vertexNew *rootofvertices = original;
		for (int j = 0; j < polygons[i].num_vertices ; j++)
		{
			original->num_vertex = j;
			original->x_coord = polygons[i].vertices[j].x_coord;
			original->y_coord = polygons[i].vertices[j].y_coord;
			if(j < polygons[i].num_vertices-1)
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
			else
				original->next = NULL;
			original=original->next;
		}
		// clipping is done wrt to each boundary. All the 4 cases are considered in the following order.
		// 1. if both the points are outside then no output
		// 2. if start point is outside and end point is inside then output end point and point of intersection with that boundary.
		// 3. if both of them are inside then output the end point
		// 4. if end point is out and start point is inside then output the point of intersection.

		//clipping wrt to left boundary
		struct vertexNew first = {rootofvertices->num_vertex,rootofvertices->x_coord,rootofvertices->y_coord,NULL};
		struct vertexNew *temp = rootofvertices;
		original = (struct vertexNew*)malloc(sizeof(struct vertexNew));
		struct vertexNew *leftrootofvertices = original;
		int count = 0;
		flag = 0;
		while(temp->next != NULL)
		{
			flag_set = 0;
			if(temp->x_coord <= x1 && temp->next->x_coord <= x1)
			{
				flag_set = 1;
			}
			else if(temp->x_coord <= x1 && temp->next->x_coord >= x1)
			{
				float yy = (((temp->y_coord - temp->next->y_coord)*1.0)/(temp->x_coord - temp->next->x_coord))* (x1 - temp->x_coord) + (temp->y_coord);
				original->num_vertex = count;
				original->x_coord = x1;
				original->y_coord = yy;
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->x_coord >= x1 && temp->next->x_coord >= x1)
			{
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->x_coord >= x1 && temp->next->x_coord <= x1)
			{
				float yy = (((temp->y_coord - temp->next->y_coord)*1.0)/(temp->x_coord - temp->next->x_coord))* (x1 - temp->x_coord) + (temp->y_coord);
				original->num_vertex = count;
				original->x_coord = x1;
				original->y_coord = yy;
			}
			if (temp->next->next == NULL && flag == 0)
			{
				temp->next->next = &first;
				flag = 1;
			}
			temp = temp->next;
			if(temp->next == NULL)
				original->next = NULL;
			else if(flag_set == 0)
			{
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
			}
		}
		temp = leftrootofvertices;
		if(flag_set == 1)
		{
			for(int k = 0;k<count-1;k++)
			{
				temp = temp->next;
			}
			if(count > 0)
				temp->next = NULL;
		}
		if (count == 0)
			leftrootofvertices->next = NULL;

		// clipping with repect to right boundary
		struct vertexNew first3 = {leftrootofvertices->num_vertex,leftrootofvertices->x_coord,leftrootofvertices->y_coord,NULL};
		temp = leftrootofvertices;
		original = (struct vertexNew*)malloc(sizeof(struct vertexNew));
		struct vertexNew *rightrootofvertices = original;
		count = 0;
		flag = 0;
		while(temp->next != NULL)
		{
			flag_set = 0;
			if(temp->x_coord >= x2 && temp->next->x_coord >= x2)
			{
				flag_set = 1;
			}
			else if(temp->x_coord >= x2 && temp->next->x_coord <= x2)
			{
				float yy = (((temp->y_coord - temp->next->y_coord)*1.0)/(temp->x_coord - temp->next->x_coord))* (x2 - temp->x_coord) + (temp->y_coord);
				original->num_vertex = count;
				original->x_coord = x2;
				original->y_coord = yy;
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->x_coord <= x2 && temp->next->x_coord <= x2)
			{
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->x_coord <= x2 && temp->next->x_coord >= x2)
			{
				float yy = (((temp->y_coord - temp->next->y_coord)*1.0)/(temp->x_coord - temp->next->x_coord))* (x2 - temp->x_coord) + (temp->y_coord);
				original->num_vertex = count;
				original->x_coord = x2;
				original->y_coord = yy;
			}
			if (temp->next->next == NULL && flag == 0)
			{
				temp->next->next = &first3;
				flag = 1;
			}
			temp = temp->next;
			if(temp->next == NULL)
				original->next = NULL;
			else if(flag_set == 0)
			{
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
			}
		}
		temp = rightrootofvertices;
		if(flag_set == 1)
		{
			for(int k = 0;k<count-1;k++)
			{
				temp = temp->next;
			}
			if(count > 0)
				temp->next = NULL;
		}
		if (count == 0)
			rightrootofvertices->next = NULL;

		//clipping with repect to bottom boundary
		struct vertexNew first1 = {rightrootofvertices->num_vertex,rightrootofvertices->x_coord,rightrootofvertices->y_coord,NULL};
		temp = rightrootofvertices;
		original = (struct vertexNew*)malloc(sizeof(struct vertexNew));
		struct vertexNew *bottomrootofvertices = original;
		count = 0;
		flag = 0;
		while(temp->next != NULL)
		{
			flag_set = 0;
			if(temp->y_coord <= y1 && temp->next->y_coord <= y1)
			{
				flag_set = 1;
			}
			else if(temp->y_coord <= y1 && temp->next->y_coord >= y1)
			{
				float xx = (((temp->x_coord - temp->next->x_coord)*1.0)/(temp->y_coord - temp->next->y_coord))* (y1 - temp->y_coord) + (temp->x_coord);
				original->num_vertex = count;
				original->x_coord = xx;
				original->y_coord = y1;
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->y_coord >= y1 && temp->next->y_coord >= y1)
			{
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->y_coord >= y1 && temp->next->y_coord <= y1)
			{
				float xx = (((temp->x_coord - temp->next->x_coord)*1.0)/(temp->y_coord - temp->next->y_coord))* (y1 - temp->y_coord) + (temp->x_coord);
				original->num_vertex = count;
				original->x_coord = xx;
				original->y_coord = y1;
			}
			if (temp->next->next == NULL && flag == 0)
			{
				temp->next->next = &first1;
				flag = 1;
			}
			temp = temp->next;
			if(temp->next == NULL)
				original->next = NULL;
			else if(flag_set == 0)
			{
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
			}
		}
		temp = bottomrootofvertices;
		if(flag_set == 1)
		{
			for(int k = 0;k<count-1;k++)
			{
				temp = temp->next;
			}
			if(count > 0)
				temp->next = NULL;
		}
		//clipping with repect to top boundary
		if (count == 0)
			bottomrootofvertices->next = NULL;
		struct vertexNew first2 = {bottomrootofvertices->num_vertex,bottomrootofvertices->x_coord,bottomrootofvertices->y_coord,NULL};
		temp = bottomrootofvertices;
		original = (struct vertexNew*)malloc(sizeof(struct vertexNew));
		struct vertexNew *toprootofvertices = original;
		count = 0;
		flag = 0;
		while(temp->next != NULL)
		{
			flag_set = 0;
			if(temp->y_coord >= y2 && temp->next->y_coord >= y2)
			{
				flag_set = 1;
			}
			else if(temp->y_coord >= y2 && temp->next->y_coord <= y2)
			{
				float xx = (((temp->x_coord - temp->next->x_coord)*1.0)/(temp->y_coord - temp->next->y_coord))* (y2 - temp->y_coord) + (temp->x_coord);
				original->num_vertex = count;
				original->x_coord = xx;
				original->y_coord = y2;
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->y_coord <= y2 && temp->next->y_coord <= y2)
			{
				original->num_vertex = count;
				original->x_coord = temp->next->x_coord;
				original->y_coord = temp->next->y_coord;
			}
			else if(temp->y_coord <= y2 && temp->next->y_coord >= y2)
			{
				float xx = (((temp->x_coord - temp->next->x_coord)*1.0)/(temp->y_coord - temp->next->y_coord))* (y2 - temp->y_coord) + (temp->x_coord);
				original->num_vertex = count;
				original->x_coord = xx;
				original->y_coord = y2;
			}
			if (temp->next->next == NULL && flag == 0)
			{
				temp->next->next = &first2;
				flag = 1;
			}
			temp = temp->next;
			if(temp->next == NULL)
				original->next = NULL;
			else if(flag_set == 0) 
			{
				original->next = (struct vertexNew*)malloc(sizeof(struct vertexNew));
				original = original->next;
				count++;
			}
		}
		temp = toprootofvertices;
		if(flag_set == 1)
		{
			for(int k = 0;k<count-1;k++)
			{
				temp = temp->next;
			}
			if(count > 0)
				temp->next = NULL;
		}

		// new vertices generated after clipping are stored and then new set of polygons are passed to scanning function
		polygon_clip[i].num_polygon = i;
		polygon_clip[i].num_vertices = count+1;
		int j=0;
		temp = toprootofvertices;
		if(i == 0)
		{
			clearFramebuffer();
			drawit();
			glFlush();
		}
		if(count > 0)
		{
			while(temp != NULL && j<=count)
			{
				polygon_clip[i].vertices[j].num_vertex = j;
				polygon_clip[i].vertices[j].x_coord = ceil(temp->x_coord);
				polygon_clip[i].vertices[j].y_coord = ceil(temp->y_coord);
				temp = temp->next;
				j++;
			}
			scanningPolygon(y2,y1,i,polygon_clip);
		}
	}
}

// display function is used for drawing polygons, clipping rectangle and polygons shapes after clipping
void display(void)
{
	// clears the screen
	if (flag_clearing == 1)
	{
		glClear ( GL_COLOR_BUFFER_BIT );
	}
	// Draws the polygons by using scanning algorithms. Maximum upto 10 polygons can be entered.
	if (flag_start_clipping == 0 && flag_start_draw_polygons == 1 && flag_clearing == 0)
	{
		drawPolygon();
	}
	// Draws the cliping rectangle
	if (flag_start_clipping == 0 && flag_start_draw_polygons == 0 && flag_clearing == 0)
	{
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(capture_x,capture_y);
			glVertex2f(capture_x,ymouse);
			glVertex2f(xmouse,ymouse);
			glVertex2f(xmouse,capture_y);
		glEnd();
	}
	// Does the clipping of polygons after user has released the mouse and also draws the final clipping rectangle
	if(flag_start_clipping == 1 && flag_clearing == 0)
	{
		clippingProcess();
		glColor3f(1.0,1.0,1.0);
		glBegin(GL_LINE_LOOP);
			glVertex2f(capture_x,capture_y);
			glVertex2f(capture_x,ymouse);
			glVertex2f(xmouse,ymouse);
			glVertex2f(xmouse,capture_y);
		glEnd();
		flag_start_clipping = 0;
		flag_capture = 1;
	}
	glFlush ( );
}

void mouseMove (int button, int state, int x, int y)                          
{
	flag_clearing = 0;
	xmouse = x;
	ymouse = y;
	// when the user clicks right button that is the end of polygon drawing
	if(button==GLUT_RIGHT_BUTTON && state==GLUT_DOWN )
	{
		flag_start_draw_polygons = 1;
		flag_end_of_polygon = 1;
		glutPostRedisplay ( );
	}
	// user clicks the left mouse button to select the vertices of polygon
	if(button == GLUT_LEFT_BUTTON && state==GLUT_DOWN && flag_start_clipping == 0 && flag_start_draw_polygons == 1 && flag_clearing == 0)
	{
		flag_start_draw_polygons = 1;
		glutPostRedisplay ( );
	}
	// final coordinates of clipping rectangle are captured
	if(button==GLUT_LEFT_BUTTON && state==GLUT_UP && flag_start_draw_polygons == 0 && flag_start_clipping == 0 && flag_clearing == 0)
	{
		capture_x_final = xmouse;
		capture_y_final = ymouse;
		flag_start_clipping = 1;
		glutPostRedisplay ( );
	}
	// initial coordinates of clipping rectangle are captured
	if(button == GLUT_LEFT_BUTTON && state==GLUT_DOWN && flag_start_clipping == 1 && flag_start_draw_polygons == 0 && flag_clearing == 0)
	{
		clearFramebuffer();
		for(int i=0; i<ImageH; i++)
				for(int j=0; j<ImageW; j++)
					for(int k=0; k<3 ; k++)
						framebuffer[i][j][k] = framebufferstore[i][j][k];
		drawit();
		glFlush();
	}
}

// capturing the clipping rectangle as it is being changed continuously until released
void mouseMove1 (int x, int y)
{
	xmouse = x;
	ymouse = y;
	if( flag_start_clipping == 0 && flag_start_draw_polygons == 0 && flag_clearing == 0)
	{
		clearFramebuffer();
		for(int i=0; i<ImageH; i++)
				for(int j=0; j<ImageW; j++)
					for(int k=0; k<3 ; k++)
						framebuffer[i][j][k] = framebufferstore[i][j][k];
		drawit();
		if(flag_capture == 1)
		{
			capture_x = xmouse;
			capture_y = ymouse;
			flag_capture = 0;
		}
		glutPostRedisplay ( );
	}
	if( flag_start_clipping == 1 && flag_start_draw_polygons == 0 && flag_clearing == 0)
	{
		clearFramebuffer();
		for(int i=0; i<ImageH; i++)
				for(int j=0; j<ImageW; j++)
					for(int k=0; k<3 ; k++)
						framebuffer[i][j][k] = framebufferstore[i][j][k];
		drawit();
		if(flag_capture == 1)
		{
			capture_x = xmouse;
			capture_y = ymouse;
			flag_capture = 0;
		}
		glutPostRedisplay ( );
	}
}

// enter c to change to clipping mode. Polygons cannot be entered after this
void keyboard ( unsigned char key, int x, int y )           
{
	switch ( key )
	{
		case 'c' :                                      
			flag_start_draw_polygons = 0;
			flag_capture = 1;
			for(int i=0; i<ImageH; i++)
				for(int j=0; j<ImageW; j++)
					for(int k=0; k<3 ; k++)
						framebufferstore[i][j][k] = framebuffer[i][j][k];
			break;
	}
}

// sets background color and initalises details regarding matrix mode and ortho2d 
void init(void)
{
	glClearColor (0.0, 0.0, 0.0, 0.0);                               
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, ImageW-1, ImageH-1, 0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
}


// This function initialises the window size and positions. Glut mouse and motion and keyboard functions are also defined.
int main(int argc, char** argv)
{
	glutInit(&argc,argv);                                            
	glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
	glutInitWindowSize(ImageW,ImageH);
	glutInitWindowPosition(100,100);
	glutCreateWindow("Rajashree Rao Polsani - Homework 2");
	init();	
	glutDisplayFunc(display);
	glutMouseFunc ( mouseMove );
	glutMotionFunc( mouseMove1 );
	glutKeyboardFunc ( keyboard );
	glutMainLoop();
	return 0;
}

