#include "world/level/levelgen/synth/SimplexNoise.h"
#include "java/Number.h"

constexpr int_t SimplexNoise::grad3[12][3];

SimplexNoise::SimplexNoise() : SimplexNoise(Random())
{

}

SimplexNoise::SimplexNoise(Random &random)
{
	xo = random.nextDouble() * 256.0;
	yo = random.nextDouble() * 256.0;
	zo = random.nextDouble() * 256.0;

	for (int_t i = 0; i < 256; i++)
		p[i] = i;
	for (int_t i = 0; i < 256; i++)
	{
		int_t j = random.nextInt(256 - i) + i;
		int_t tmp = p[i];
		p[i] = p[j];
		p[j] = tmp;
		p[i + 256] = p[i];
	}
}

SimplexNoise::SimplexNoise(Random &&random) : SimplexNoise(random)
{

}

int_t SimplexNoise::fastfloor(double x)
{
	int_t value = Java::numberToInt(x);
	return x > 0.0 ? value : Java::intFromBits(static_cast<uint_t>(value) - 1U);
}

double SimplexNoise::dot(const int_t *g, double x, double y)
{
	return g[0] * x + g[1] * y;
}

double SimplexNoise::dot(const int_t *g, double x, double y, double z)
{
	return g[0] * x + g[1] * y + g[2] * z;
}

double SimplexNoise::getValue(double xin, double yin)
{
	double n0, n1, n2; 
	
	double s = (xin + yin) * F2; 
	int i = fastfloor(xin + s);
	int j = fastfloor(yin + s);
	double t = Java::intFromBits(static_cast<uint_t>(i) + static_cast<uint_t>(j)) * G2;
	double X0 = i - t; 
	double Y0 = j - t;
	double x0 = xin - X0; 
	double y0 = yin - Y0;
	
	
	int i1, j1; 
	if (x0 > y0) { i1 = 1; j1 = 0; } 
	else { i1 = 0; j1 = 1; }			
	
	
	
	double x1 = x0 - i1 + G2; 
	double y1 = y0 - j1 + G2;
	double x2 = x0 - 1.0 + 2.0 * G2; 
	double y2 = y0 - 1.0 + 2.0 * G2;
	
	int ii = i & 255;
	int jj = j & 255;
	int gi0 = p[ii + p[jj]] % 12;
	int gi1 = p[ii + i1 + p[jj + j1]] % 12;
	int gi2 = p[ii + 1 + p[jj + 1]] % 12;
	
	double t0 = 0.5 - x0 * x0 - y0 * y0;
	if (t0 < 0) n0 = 0.0;
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0);	
	}
	double t1 = 0.5 - x1 * x1 - y1 * y1;
	if (t1 < 0) n1 = 0.0;
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	double t2 = 0.5 - x2 * x2 - y2 * y2;
	if (t2 < 0) n2 = 0.0;
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}
	
	
	return 70.0 * (n0 + n1 + n2);
}


double SimplexNoise::getValue(double xin, double yin, double zin) {
	double n0, n1, n2, n3; 
	
	double s = (xin + yin + zin) * F3; 
	int i = fastfloor(xin + s);
	int j = fastfloor(yin + s);
	int k = fastfloor(zin + s);
	double t = Java::intFromBits(static_cast<uint_t>(i) + static_cast<uint_t>(j) + static_cast<uint_t>(k)) * G3;
	double X0 = i - t; 
	double Y0 = j - t;
	double Z0 = k - t;
	double x0 = xin - X0; 
	double y0 = yin - Y0;
	double z0 = zin - Z0;
	
	
	int i1, j1, k1; 
	int i2, j2, k2; 
	if (x0 >= y0)
	{
		if (y0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } 
		else if (x0 >= z0) { i1 = 1; j1 = 0; k1 = 0; i2 = 1; j2 = 0; k2 = 1; } 
		else { i1 = 0; j1 = 0; k1 = 1; i2 = 1; j2 = 0; k2 = 1; } 
	}
	else
	{ 
		if (y0 < z0) { i1 = 0; j1 = 0; k1 = 1; i2 = 0; j2 = 1; k2 = 1; } 
		else if (x0 < z0) { i1 = 0; j1 = 1; k1 = 0; i2 = 0; j2 = 1; k2 = 1; } 
		else { i1 = 0; j1 = 1; k1 = 0; i2 = 1; j2 = 1; k2 = 0; } 
	}
	
	
	
	
	double x1 = x0 - i1 + G3; 
	double y1 = y0 - j1 + G3;
	double z1 = z0 - k1 + G3;
	double x2 = x0 - i2 + 2.0 * G3; 
	double y2 = y0 - j2 + 2.0 * G3;
	double z2 = z0 - k2 + 2.0 * G3;
	double x3 = x0 - 1.0 + 3.0 * G3; 
	double y3 = y0 - 1.0 + 3.0 * G3;
	double z3 = z0 - 1.0 + 3.0 * G3;
	
	int ii = i & 255;
	int jj = j & 255;
	int kk = k & 255;
	int gi0 = p[ii + p[jj + p[kk]]] % 12;
	int gi1 = p[ii + i1 + p[jj + j1 + p[kk + k1]]] % 12;
	int gi2 = p[ii + i2 + p[jj + j2 + p[kk + k2]]] % 12;
	int gi3 = p[ii + 1 + p[jj + 1 + p[kk + 1]]] % 12;
	
	double t0 = 0.6 - x0 * x0 - y0 * y0 - z0 * z0;
	if (t0 < 0) n0 = 0.0;
	else
	{
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0, z0);
	}
	double t1 = 0.6 - x1 * x1 - y1 * y1 - z1 * z1;
	if (t1 < 0) n1 = 0.0;
	else
	{
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1, z1);
	}
	double t2 = 0.6 - x2 * x2 - y2 * y2 - z2 * z2;
	if (t2 < 0) n2 = 0.0;
	else
	{
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2, z2);
	}
	double t3 = 0.6 - x3 * x3 - y3 * y3 - z3 * z3;
	if (t3 < 0) n3 = 0.0;
	else
	{
		t3 *= t3;
		n3 = t3 * t3 * dot(grad3[gi3], x3, y3, z3);
	}
	
	
	return 32.0 * (n0 + n1 + n2 + n3);
}

void SimplexNoise::add(double *out, double x, double y, int_t xd, int_t yd, double xs, double ys, double mul)
{
	double *outp = out;
	for (int_t xi = 0; xi < xd; xi++)
	{
		double xin = (x + xi) * xs + xo;
		for (int_t yi = 0; yi < yd; yi++)
		{
			double yin = (y + yi) * ys + yo;
			*outp++ += getValue(xin, yin) * mul;
		}
	}
}

void SimplexNoise::add(double *out, double x, double y, double z, int_t xd, int_t yd, int_t zd, double xs, double ys, double zs, double mul)
{
	double *outp = out;
	for (int_t xi = 0; xi < xd; xi++)
	{
		double xin = (x + xi) * xs + xo;
		for (int_t zi = 0; zi < zd; zi++)
		{
			double zin = (z + zi) * zs + zo;
			for (int_t yi = 0; yi < yd; yi++)
			{
				double yin = (y + yi) * ys + yo;
				*outp++ += getValue(xin, yin, zin) * mul;
			}
		}
	}
}
