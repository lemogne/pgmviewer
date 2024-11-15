#pragma once
#include <math.h>

const double pi = 3.14159265358979323846;

struct vec3 {
	double x, y, z;
	vec3 operator +(vec3 vector);
	void operator +=(vec3 vector);
	vec3 operator -(vec3 vector);
	void operator -=(vec3 vector);
	double operator *(vec3 vector);
	vec3 operator *(double scalar);
	void operator *=(double scalar);
	double abs();
	double& operator[](size_t k);
};

vec3 vec3::operator+(vec3 vector) {
	return vec3{this->x + vector.x, this->y + vector.y, this->z + vector.z};
}

void vec3::operator+=(vec3 vector) {
	x += vector.x;
	y += vector.y;
	z += vector.z;
}

vec3 vec3::operator-(vec3 vector) {
	return vec3{this->x - vector.x, this->y - vector.y, this->z - vector.z};
}

void vec3::operator-=(vec3 vector) {
	x -= vector.x;
	y -= vector.y;
	z -= vector.z;
}

double vec3::operator*(vec3 vector) {
	return this->x * vector.x + this->y * vector.y + this->z * vector.z;
}

vec3 vec3::operator*(double scalar) {
	return vec3{scalar * x, scalar * y, scalar * z};
}

void vec3::operator*=(double scalar) {
	x *= scalar;
	y *= scalar;
	z *= scalar;
}

double vec3::abs() {
	return sqrt(x * x + y * y + z * z);
}

double& vec3::operator[](size_t k) {
	switch (k) {
	case 0:
		return x;
	case 1:
		return y;
	case 2:
		return z;
	default:
		throw "Undefined index for vec3";
	}
}


inline double sq(double i) {
	return i * i;
}

inline double a(double i) {
	if (i < 0)
		return -i;
	return i;
}

double ab(double x){
	return x*((x>0)*2-1);
}

inline double random(double x,double y, int Seed){
	int n = (int) x + (int)(y - Seed) * 57 + Seed;
	n = (n << 13) ^ n;
	int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
	return 1.0 - ((double) nn / 1073741824.0);
}

inline double interpolate1(double a,double b,double x){
	double ft = x * pi;
	double f = (1.0 - cos(ft)) * 0.5;
	return a * (1.0 - f) + b * f;
}

double noise(double x,double y, int Seed){
	double floorx=(double)((int)x);
	double floory=(double)((int)y);
	double s,t,u,v;
	s=random(floorx,floory,Seed);
	t=random(floorx+1,floory,Seed);
	u=random(floorx,floory+1,Seed);
	v=random(floorx+1,floory+1,Seed);
	double int1=interpolate1(s,t,x-floorx);
	double int2=interpolate1(u,v,x-floorx);
	return interpolate1(int1,int2,y-floory);
}