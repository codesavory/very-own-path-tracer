// very_own_path_tracer.cpp : This file contains the 'main' function. Program execution begins and ends there.
// based on smallpt, a Path Tracer by Kevin Beason, 2008

#include "pch.h"
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

double M_PI = 14159265358979;
double M_1_PI = 1.0 / M_PI;
double erand48(unsigned short xsubi[3])
{
	return (double)rand() / (double)RAND_MAX;
}

// Vec Structure acts as points, colors and vectors
struct Vec
{
	double x, y, z;		//position, also color(r,g,b)

	Vec(double x_ = 0, double y_ = 0, double z_ = 0) { x = x_; y = y_; z = z_; }
	Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
	Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
	Vec operator*(double b) const { return Vec(x*b, y*b, z*b); }
	Vec mult(const Vec &b) const { return Vec(x*b.x, y*b.y, z*b.z); }
	Vec& norm() { return *this = *this * (1 / sqrt(x*x + y*y + z*z)); }
	double dot(const Vec &b) const { return x * b.x + y * b.y + z * b.z; }
	Vec operator%(Vec &b) { return Vec(y*b.z - z * b.y, z*b.x - x * b.z, x*b.y - y * b.x); }//cross pdt
};

// Ray Structure
struct Ray
{
	Vec o, d;	//origin and direction

	Ray(Vec o_, Vec d_) : o(o_), d(d_) {}
};

// Enum of material types used in RADIANCE function
enum Refl_t { DIFF, SPEC, REFR };

//smallpt only supports spheres, Sphere Structure
struct Sphere
{
	double rad;		//radius
	Vec p, e, c;	//position, emission and color
	Refl_t refl;	//reflection type (DIFFuse, SPECular, REFRactive)

	Sphere(double rad_, Vec p_, Vec e_, Vec c_, Refl_t refl_) :
		rad(rad_), p(p_), e(e_), c(c_), refl(refl_) {}

	//returns distance or 0 if no hit
	double intersect(const Ray &r) const
	{
		//Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
		Vec op = p - r.o;								// p is Sphere center (C)
		double t, eps = 1e-4;							// eps is small fudge factor
		double b = op.dot(r.d);							// 1/2 b from quadratic eq. setup
		double det = b * b - op.dot(op) + rad * rad;	// (b^2-4ac)/4: a=1 because ray normalized
		if (det < 0)	//ray misses sphere
			return 0;
		else
			det = sqrt(det);
		return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);	//return smaller t, 0 if no hit
	}
};

// HARD CODED scene description
// The Scene Description consists of a bunch of spheres
// Scene: radius, position, emission, color, material
Sphere spheres[] = {
	Sphere(1e5, Vec(1e5 + 1,40.8,81.6), Vec(),Vec(.75,.25,.25),DIFF), //Left
	Sphere(1e5, Vec(-1e5 + 99,40.8,81.6),Vec(),Vec(.25,.25,.75),DIFF), //Rght
	Sphere(1e5, Vec(50,40.8, 1e5),     Vec(),Vec(.75,.75,.75),DIFF), //Back
	Sphere(1e5, Vec(50,40.8,-1e5 + 170), Vec(),Vec(),           DIFF), //Frnt
	Sphere(1e5, Vec(50, 1e5, 81.6),    Vec(),Vec(.75,.75,.75),DIFF), //Botm
	Sphere(1e5, Vec(50,-1e5 + 81.6,81.6),Vec(),Vec(.75,.75,.75),DIFF), //Top
	Sphere(16.5,Vec(27,16.5,47),       Vec(),Vec(1,1,1)*.999, SPEC), //Mirr
	Sphere(16.5,Vec(73,16.5,78),       Vec(),Vec(1,1,1)*.999, REFR), //Glas
	Sphere(1.5, Vec(50,81.6 - 16.5,81.6),Vec(4,4,4) * 100,  Vec(), DIFF)  //Lite
};
int numSpheres = sizeof(spheres) / sizeof(Sphere);

// CLAMP FUNCTION
inline double clamp(double x) { return x < 0 ? 0 : x>1 ? 1 : x; }

// CONVERTS FLOATS TO INTEGERS TO BE SAVED in PPM File
inline int toInt(double x) { return int(pow(clamp(x), 1 / 2.2) * 255 + 0.5); }	//applies a gamma correction of 2.2

// INTERSECTS ray with SCENE
inline bool intersect(const Ray &r, double &t, int &id)
{
	double n = sizeof(spheres) / sizeof(Sphere);
	double d;
	double inf = t = 1e20;

	for (int i = int(n); i--;)
	{
		if ((d = spheres[i].intersect(r)) && d < t) { t = d; id = i; }
	}
	return t < inf; // return true if t is not infinity
}

// COMPUTES the radiance estimate along the Ray r
Vec radiance(const Ray &r, int depth, unsigned short *Xi, int E = 1)
{
	double t;									// distance to intersection
	int id = 0;									// id of intersected object
	if (!intersect(r, t, id))	return Vec();	// if miss, return black
	const Sphere &obj = spheres[id];			// the hit object

	if (depth > 10)	return Vec();
	Vec x = r.o + r.d*t;						// ray intersection point
	Vec n = (x - obj.p).norm();					// sphere normal
	Vec n1 = n.dot(r.d) < 0 ? n : n * -1;		// properly oriented surface normal
	Vec f = obj.c;								// object color (BRDF modulator)

	// Use maximum reflectivity amount for Russian Roulette
	double p = f.x > f.y && f.x > f.z ? f.x : f.y > f.z ? f.y : f.z; // max refl
	if (++depth > 5 || !p)
		if (erand48(Xi) < p)
			f = f * (1 / p);
		else
			return obj.e*E;

	// IDEAL DIFFUSE REFLECTION
	if (obj.refl == DIFF)
	{
		double r1 = 2 * M_PI*erand48(Xi);				// angle around
		double r2 = erand48(Xi), r2s = sqrt(r2);		// distance from center
		Vec w = n1;										// w=normal
		Vec u = ((fabs(w.x) > .1 ? Vec(0, 1) : Vec(1)) % w).norm(); // u is perpendicular to w
		Vec v = w % u;									// v is perpendicular to u and w
		Vec d = (u*cos(r1)*r2s + v * sin(r1)*r2s + w * sqrt(1 - r2)).norm(); // d is random reflection ray

		// loop over any lights(explicit lighting)
		Vec e;
		for (int i = 0; i < numSpheres; i++)
		{
			const Sphere &s = spheres[i];
			if (s.e.x <= 0 && s.e.y <= 0 && s.e.z <= 0)
				continue; //skip non-lights
			
			// create random direction towards sphere using method from realistic ray tracing
			Vec sw = s.p - x, su = ((fabs(sw.x) > .1 ? Vec(0, 1) : Vec(1)) % sw).norm(), sv = sw % su;
			double cos_a_max = sqrt(1-s.rad*s.rad/(x-s.p).dot(x-s.p));

			double eps1 = erand48(Xi), eps2 = erand48(Xi);
			double cos_a = 1 - eps1 + eps1 * cos_a_max;
			double sin_a = sqrt(1 - cos_a * cos_a);
			double phi = 2 * M_PI*eps2;
			Vec l = su * cos(phi)*sin_a + sv * sin(phi)*sin_a + sw * cos_a;
			l.norm();

			// shoot shadow rays
			if (intersect(Ray(x, l), t, id) && id == i) // shadow ray
			{
				double omega = 2 * M_PI*(1 - cos_a_max);
				e = e + f.mult(s.e*l.dot(n1)*omega)*M_1_PI; // 1/pi fpr brdf
			}
		}
		return obj.e*E + e + f.mult(radiance(Ray(x, d), depth, Xi, 0));
	}
	// IDEAL SPECULAR REFLECTION
	else if (obj.refl == SPEC)
	{
		return obj.e + f.mult(radiance(Ray(x, r.d - n * 2 * n.dot(r.d)), depth, Xi));
	}
	// OTHERWISE WE HAVE A DIELECTRIC(GLASS) SURFACE
	Ray reflRay(x, r.d - n * 2 * n.dot(r.d));	// Ideal dielectric reflection
	bool into = n.dot(n1) > 0;					// Ray from outside going in check?
	double nc = 1, nt = 1.5, nnt = into ? nc / nt : nt / nc, ddn = r.d.dot(n1), cos2t;
	
	// if total internal reflection, REFLECT
	if ((cos2t = 1 - nnt * nnt*(1 - ddn * ddn)) < 0) // Total internal reflection
		return obj.e + f.mult(radiance(reflRay, depth, Xi));

	// otherwise, choose REFLECTION or REFRACTION
	Vec tdir = (r.d*nnt - n * ((into ? 1 : -1)*(ddn*nnt + sqrt(cos2t)))).norm();
	double a = nt - nc, b = nt + nc, R0 = a * a / (b*b), c = 1 - (into ? -ddn : tdir.dot(n));
	double Re = R0 + (1 - R0)*c*c*c*c*c, Tr = 1 - Re, P = .25 + .5*Re, RP = Re / P, TP = Tr / (1 - P);
	return obj.e + f.mult(depth > 2 ? (erand48(Xi) < P ?   // Russian roulette
		radiance(reflRay, depth, Xi)*RP : radiance(Ray(x, tdir), depth, Xi)*TP) :
		radiance(reflRay, depth, Xi)*Re + radiance(Ray(x, tdir), depth, Xi)*Tr);

}

int main(int argc, char *argv[])
{
	int w = 512, h = 384; //image size
	int samps = argc == 2 ? atoi(argv[1]) / 4 : 1;				// # of samples(default=1)
	Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).norm());	// camera position and direction
	Vec cx = Vec(w*.5135 / h);									// x-direction increment (uses implicit 0 for y&z)
	Vec cy = (cx%cam.d).norm()*.5135;							// y-direction increment (note cross pdt - '%')
	Vec r;														// used for colors of samples
	Vec *c = new Vec[w*h];										// The image matrix

#pragma omp parallel for schedule(dynamic, 1) private(r)		// OpenMP
	//Loop over all image pixels
	for (int y = 0; y < h; y++)	//loop over image rows
	{
		fprintf(stderr, "\rRendering (%d spp) %5.2f%%", samps * 4, 100.*y / (h - 1));	// print progress
		unsigned short Xi[3] = { 0,0,y*y*y };

		for (unsigned short x = 0; x < w; x++)	// Loop Columns
		{
			// For each pixel do 2x2 subsamples, and avg. samples per subpixel
			for (int sy = 0, i = (h - y - 1)*w + x; sy < 2; sy++)     // 2x2 subpixel rows
				for (int sx = 0; sx < 2; sx++, r = Vec())			  // 2x2 subpixel cols
				{
					for (int s = 0; s < samps; s++)	// Camera rays are pushed ^^^^^ forward to start in interior
					{
						//tent filter
						double r1 = 2 * erand48(Xi), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
						double r2 = 2 * erand48(Xi), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
						Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
							cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
						r = r + radiance(Ray(cam.o + d * 140, d.norm()), 0, Xi)*(1. / samps);
					}
					c[i] = c[i] + Vec(clamp(r.x), clamp(r.y), clamp(r.z))*.25;
				}
		}
	}
	// Write out the file to a PPM
	FILE *f = fopen("image.ppm", "w");	//write image to PPM File
	fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
	for (int i = 0; i < w*h; i++)
		fprintf(f, "%d %d %d ", toInt(c[i].x), toInt(c[i].y), toInt(c[i].z));
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
