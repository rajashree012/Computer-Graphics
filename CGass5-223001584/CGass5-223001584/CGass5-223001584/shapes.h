#ifndef SHAPES_H
#define SHAPES_H

# include <math.h>
# include "vectors.h"
# include <string>
# include <algorithm>

// Putting the eye at the origin (0,0,0)
Point3D eye(0.0,0.0,0.0);

// color

class Color
{
	public:
		float red,green,blue;
		Color()
		{
			red=0;
			blue=0;
			green=0;
		}
		Color(Point3D color)
		{
			red = color.x; green = color.y; blue = color.z;
		}
		Color(float R,float G, float B){
			red = R;
			green=G;
			blue=B;
		}
		Color addColor(Color c)
		{
			return(Point3D(red+c.red,green+c.green,blue+c.blue));
		}
};

// ray
class rayProp
{
	public:
		Point3D startPoint;
		Point3D endPoint;
		rayProp ()
		{
		}
		rayProp(Point3D s, Point3D e)
		{
			startPoint = s;
			endPoint = e;
		}
		// start at eye
	    Point3D directionRay(Point3D s, Point3D e)
		{
			return vecSubtraction(e,s);
		}
};

// super class
class CommonShape 
{
	public:
		virtual int getId() = 0;
		virtual Point3D getIntersectionPoint(rayProp ray) = 0;
		virtual bool isEyeOutside(Point3D Pe) = 0;
};

// sphere class
class Sphere : public CommonShape
{
	public: 
		
		double radius;
		Point3D centre; 
		int shapeId;
		Point3D ambient_coeff;
		Point3D diffuse_coeff;
		Point3D specular_coeff;
		int spec_expo;
		float ref_coeff;

		Sphere()
		{
		}

		Sphere(float r, Point3D c, int id, Point3D a, Point3D d, Point3D s, int spec_exp, float ref)
		{
			radius = r;
			centre = c;
			shapeId = id;	
			ambient_coeff = a;
			diffuse_coeff = d;
			specular_coeff = s;
			ref_coeff = ref;
			spec_expo = spec_exp;
		}

		Point3D getNormal(Point3D point)
		{
			Point3D n = vecSubtraction(point,centre);
			n = normalize(n);
			return n;
		}

		virtual bool isEyeOutside(Point3D pt)
		{
			double diff = dotProduct(vecSubtraction(pt,centre),vecSubtraction(pt,centre)) - radius*radius;
			if(diff>0)		//Eye is outside the sphere 
				return true;
			else 	
				return false;
		}

		virtual int getId()
		{
			return shapeId;
		}

		virtual Point3D getIntersectionPoint(rayProp ray)
		{
			Point3D intersection;
			double a = dotProduct(vecSubtraction(ray.endPoint,ray.startPoint),vecSubtraction(ray.endPoint,ray.startPoint));
			double b = 2*dotProduct(vecSubtraction(ray.endPoint,ray.startPoint),vecSubtraction(ray.startPoint,centre)) ;
			double c = dotProduct(vecSubtraction(ray.startPoint,centre),vecSubtraction(ray.startPoint,centre)) - radius*radius;
		
			double delta = b*b - 4*a*c;
			if (delta<0)
			{
				return Point3D(-999,-999,-999);
			}

			//when object is behind the eye
			if(b>0)	
			{
				return Point3D(-999,-999,-999);
			}

			double t1,t2;
			t1 = ((-1)*b - sqrt(delta))/(2*a) ;
			t2 = ((-1)*b + sqrt(delta))/(2*a) ;

			double tmin = 0;
			if (t1<t2)
				tmin = t1;
			else
				tmin = t2;
			
			return (Point3D(vecAddition(ray.startPoint,constMultiplication(vecSubtraction(ray.endPoint,ray.startPoint),tmin))));
		}
};

// plane class
class Plane : public CommonShape
{
	public: 
		int shapeId;
		Point3D normal;
		Point3D point_in_plane ;
		Point3D ambient_coeff;
		Point3D diffuse_coeff;
		Point3D specular_coeff;
		int spec_expo;
		float ref_coeff;

		Plane()
		{
		}

		Plane(Point3D n, Point3D p, int id, Point3D a, Point3D d, Point3D s, int spec_exp, float ref)
		{
			normal = n;
			point_in_plane = p;
			shapeId = id;
			ambient_coeff = a;
			diffuse_coeff = d;
			specular_coeff = s;
			ref_coeff = ref;
			spec_expo = spec_exp;
		}

		Point3D getNormal()
		{
			return normalize(normal);
		}

		virtual bool isEyeOutside(Point3D pt)
		{
			// checking whether the eye is in the plane or not
			double check = dotProduct(vecSubtraction(pt,point_in_plane),normal);
			if (check == 0)
				return false;
			else
				return true ;
		}

		virtual int getId()
		{
			return shapeId;
		}

		virtual Point3D getIntersectionPoint(rayProp ray)
		{
			Point3D intersection;
			double t = dotProduct(vecSubtraction(point_in_plane,ray.startPoint),normal)/dotProduct(vecSubtraction(ray.endPoint,ray.startPoint),normal);
			if (t < 0)
				return Point3D(-999,-999,-999);
			else 
				return vecAddition(ray.startPoint,constMultiplication(vecSubtraction(ray.endPoint,ray.startPoint),t));
		}
};

// cylinder class
class Cylinder : public CommonShape
{
	public: 
		double radius;
		Point3D start;
		Point3D direction;
		int shapeId;
		Point3D ambient_coeff;
		Point3D diffuse_coeff;
		Point3D specular_coeff;
		int spec_expo;
		float ref_coeff;

		Cylinder()
		{
		}

		Cylinder(double r,Point3D st,Point3D dir, int id, Point3D a, Point3D d, Point3D s, int spec_exp, float ref)
		{
			radius = r;
			start = st;
			direction = dir;
			shapeId = id;
			ambient_coeff = a;
			diffuse_coeff = d;
			specular_coeff = s;
			ref_coeff = ref;
			spec_expo = spec_exp;
		}

		Point3D getNormal(Point3D pt)
		{
			Point3D diff = vecSubtraction(pt,start);
			Point3D parellel = constMultiplication(normalize(direction),magnVector(diff)*dotProduct(normalize(diff),normalize(direction)));
			Point3D n = vecSubtraction(diff,parellel);
			return normalize(n);
		}

		virtual bool isEyeOutside(Point3D pt)
		{
			return true ;
		}

		virtual int getId()
		{
			return shapeId;
		}

		virtual Point3D getIntersectionPoint(rayProp ray)
		{
			Point3D intersection;
			Point3D dir = direction;
			Point3D ray_dir = ray.directionRay(ray.startPoint,ray.endPoint);
			Point3D delta = vecSubtraction(ray.startPoint,start);
			Point3D temp = vecSubtraction(ray_dir,constMultiplication(dir,dotProduct(dir,ray_dir)));
			Point3D temp1 = vecSubtraction(delta,constMultiplication(dir,dotProduct(dir,delta)));
			double a = dotProduct(temp,temp);
			double b = 2*dotProduct(temp,temp1);
			double c = dotProduct(temp1,temp1)  - radius*radius;

			float delta_quad = (b*b)-(4.0*a*c);

			if ( delta_quad<0.0 || a==0.0 || b==0.0 || c==0.0 )
			   return Point3D(-999,-999,-999);
		
			double t1,t2;
			t1 = ((-1)*b - sqrt(delta_quad))/(2*a) ;
			t2 = ((-1)*b + sqrt(delta_quad))/(2*a) ;

			float small_value = 0.00001;
		
			if( t1<=small_value && t2<=small_value ) 
				return Point3D(-999,-999,-999); 
			double tmin = 0;
			if( t1<=small_value )
			   tmin = t2;
			else
			   if( t2<=small_value )
				  tmin = t1;
			   else
				   if(t1<t2)
					   tmin = t1;
				   else
					   tmin = t2;
				//  tmin=(t1<t2) ? t1 : t2;
		
			if( tmin<small_value ) 
				return Point3D(-999,-999,-999);
			
			return (Point3D(vecAddition(ray.startPoint,constMultiplication(vecSubtraction(ray.endPoint,ray.startPoint),tmin))));
	
		}
};

// ellipsoid class
class Ellipsoid : public CommonShape
{
	public:
		double radiusx;
		double radiusy;
		double radiusz;
		Point3D center; 
		int shapeId;
		Point3D ambient_coeff;
		Point3D diffuse_coeff;
		Point3D specular_coeff;
		int spec_expo;
		float ref_coeff;

		Ellipsoid()
		{
		}

		Ellipsoid(Point3D c, double radx,double rady,double radz,int id , Point3D a, Point3D d, Point3D s,int spec_exp, float ref)
		{
			radiusx = radx;
			radiusy = rady;
			radiusz = radz;
			shapeId=id;
			center=c;
			ambient_coeff = a;
			diffuse_coeff = d;
			specular_coeff = s;
			ref_coeff = ref;
			spec_expo = spec_exp;
		}

		Point3D getNormal(Point3D point)
		{
			Point3D n=vecSubtraction(point,center);
			n = normalize(n);
			return n;
		}

		virtual bool isEyeOutside(Point3D pt)
		{
			return true;
		}

		virtual int getId()
		{
			return shapeId;
		}

		virtual Point3D getIntersectionPoint(rayProp ray)
		{
			Point3D raystart_center = vecSubtraction(ray.startPoint,center);
			Point3D direction = ray.directionRay(ray.startPoint,ray.endPoint);
			float a = ((direction.x*direction.x)/(radiusx*radiusx)) + ((direction.y*direction.y)/(radiusy*radiusy)) + ((direction.z*direction.z)/(radiusz*radiusz));
			float b = ((2.0*raystart_center.x*direction.x)/(radiusx*radiusx)) + ((2.0*raystart_center.y*direction.y)/(radiusy*radiusy)) + ((2.0*raystart_center.z*direction.z)/(radiusz*radiusz));
			float c = ((raystart_center.x*raystart_center.x)/(radiusx*radiusx)) + ((raystart_center.y*raystart_center.y)/(radiusy*radiusy)) + ((raystart_center.z*raystart_center.z)/(radiusz*radiusz)) - 1.0;

			float delta_quad = (b*b)-(4.0*a*c);

			if ( delta_quad<0.0 || a==0.0 || b==0.0 || c==0.0 )
			   return Point3D(-999,-999,-999);
		
			double t1,t2;
			t1 = ((-1)*b - sqrt(delta_quad))/(2*a) ;
			t2 = ((-1)*b + sqrt(delta_quad))/(2*a) ;

			float small_value = 0.00001;
		
			if( t1<=small_value && t2<=small_value ) 
				return Point3D(-999,-999,-999); 
			double tmin = 0;
			if( t1<=small_value )
			   tmin = t2;
			else
			   if( t2<=small_value )
				  tmin = t1;
			   else
				   if(t1<t2)
					   tmin = t1;
				   else
					   tmin = t2;
				//  tmin=(t1<t2) ? t1 : t2;
		
			if( tmin<small_value ) 
				return Point3D(-999,-999,-999);
		
			return (Point3D(vecAddition(ray.startPoint,constMultiplication(vecSubtraction(ray.endPoint,ray.startPoint),tmin))));
	}
};
#endif