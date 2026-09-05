

#include "java/StrictMath.h"
#include "java/Number.h"

#include <cstring>
#include <limits>




static const double ln2_hi = 6.93147180369123816490e-01;
static const double ln2_lo = 1.90821492927058770002e-10;
static const double two54 = 1.80143985094819840000e+16;
static const double Lg1 = 6.666666666666735130e-01;
static const double Lg2 = 3.999999999940941908e-01;
static const double Lg3 = 2.857142874366239149e-01;
static const double Lg4 = 2.222219843214978396e-01;
static const double Lg5 = 1.818357216161805012e-01;
static const double Lg6 = 1.531383769920937332e-01;
static const double Lg7 = 1.479819860511658591e-01;

static ulong_t doubleBits(double value)
{
	ulong_t bits;
	std::memcpy(&bits, &value, sizeof(bits));
	return bits;
}

static double withHighWord(double value, uint_t high)
{
	ulong_t bits = (static_cast<ulong_t>(high) << 32) | (doubleBits(value) & 0xffffffffULL);
	std::memcpy(&value, &bits, sizeof(value));
	return value;
}

double StrictMath::log(double x)
{
	static_assert(std::numeric_limits<double>::is_iec559 && std::numeric_limits<double>::digits == 53,
		"Java StrictMath requires IEEE binary64");
	int_t hx = Java::intFromBits(static_cast<uint_t>(doubleBits(x) >> 32));
	uint_t lx = static_cast<uint_t>(doubleBits(x));
	int_t k = 0;
	if (hx < 0x00100000)
	{
		if (((static_cast<uint_t>(hx) & 0x7fffffffU) | lx) == 0)
			return -std::numeric_limits<double>::infinity();
		if (hx < 0)
			return std::numeric_limits<double>::quiet_NaN();
		k -= 54;
		x *= two54;
		hx = static_cast<int_t>(doubleBits(x) >> 32);
	}
	if (hx >= 0x7ff00000)
		return x + x;
	k += (hx >> 20) - 1023;
	hx &= 0x000fffff;
	int_t i = (hx + 0x95f64) & 0x100000;
	x = withHighWord(x, static_cast<uint_t>(hx | (i ^ 0x3ff00000)));
	k += i >> 20;
	double f = x - 1.0;
	if ((0x000fffff & (2 + hx)) < 3)
	{
		if (f == 0.0)
		{
			if (k == 0)
				return 0.0;
			double dk = static_cast<double>(k);
			return dk * ln2_hi + dk * ln2_lo;
		}
		double r = f * f * (0.5 - 0.33333333333333333 * f);
		if (k == 0)
			return f - r;
		double dk = static_cast<double>(k);
		return dk * ln2_hi - ((r - dk * ln2_lo) - f);
	}
	double s = f / (2.0 + f);
	double dk = static_cast<double>(k);
	double z = s * s;
	i = hx - 0x6147a;
	double w = z * z;
	int_t j = 0x6b851 - hx;
	double t1 = w * (Lg2 + w * (Lg4 + w * Lg6));
	double t2 = z * (Lg1 + w * (Lg3 + w * (Lg5 + w * Lg7)));
	i |= j;
	double r = t2 + t1;
	if (i > 0)
	{
		double hfsq = 0.5 * f * f;
		if (k == 0)
			return f - (hfsq - s * (hfsq + r));
		return dk * ln2_hi - ((hfsq - (s * (hfsq + r) + dk * ln2_lo)) - f);
	}
	if (k == 0)
		return f - s * (f - r);
	return dk * ln2_hi - ((s * (f - r) - dk * ln2_lo) - f);
}
