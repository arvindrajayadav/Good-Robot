/*-----------------------------------------------------------------------------

  Noise.cpp

  Generates value noise.

  http://en.wikipedia.org/wiki/Value_noise

  (c) 2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"
#include "noise.h"
#include "texture.h"

#define SQRT2             1.41421356
#define USE_BILINEAR      1
#define SAMPLE            254.0f

static unsigned char*     buffer;
static int                size_val;
static Octant             shift;

/*-----------------------------------------------------------------------------

  This forms a theoretical quad the with four given values.  Given the
  offset from the upper-left corner, it determines what the value
  should be at that point in the center area.  left" determines if the
  quad is cut from y2 to y1, or from y0 to y3.

  y0-----y1
  |     |
  |     |
  y2-----y3

  -----------------------------------------------------------------------------*/
#if USE_BILINEAR
static float interpolate_quad(float y0, float y1, float y2, float y3, GLvector2 offset, bool)
{
	float top = Lerp(y0, y1, offset.x);
	float bottom = Lerp(y2, y3, offset.x);
	return Lerp(top, bottom, offset.y);
}
#else

  //lerp2
  //return n1 + (n2 – n1) * delta;

static float interpolate_quad(float y0, float y1, float y2, float y3, GLvector2 offset, bool left)
{
	float   a;
	float   b;
	float   c;

	if (left) {
		if (offset.x + offset.y < 1) {
			c = y2 - y0;
			b = y1 - y0;
			a = y0;
		}
		else {
			c = y3 - y1;
			b = y3 - y2;
			a = y3 - (b + c);
		}
	}
	else { //right
		if (offset.x < offset.y) {
			c = y2 - y0;
			b = y3 - y2;
			a = y0;
		}
		else {
			c = y3 - y1;
			b = y1 - y0;
			a = y0;
		}
	}
	return (a + b * offset.x + c * offset.y);
}
#endif

static inline float interpolate(float n1, float n2, float delta)
{
	return n1 * (1.0f - delta) + n2 * delta;
}

static inline void coord(float pos, int* index, float* rem)
{
	float frac;

	*rem = modf(pos, &frac);
	if (*rem < 0)
		*rem = 1.0f + *rem;
	*index = (int)frac % size_val;
}

static inline unsigned char get_sample(unsigned x, unsigned y, unsigned z)
{
	x = (x + z * 13) % size_val;
	y = (y + z * 17) % size_val;
	return buffer[(x + y * size_val)];
}

static float get_octave(GLvector pos)
{
	Octant      gridpos;
	GLvector    offset;
	float       u0, u1, u2, u3;
	float       l0, l1, l2, l3;
	float       upper, lower;
	bool        left;

	////FOR TESTING, TO SPEED UP LEVEL GEN.
	//return 0.5f;/////////////////////////////////////////
	coord(pos.x, &gridpos.x, &offset.x);
	coord(pos.y, &gridpos.y, &offset.y);
	coord(pos.z, &gridpos.z, &offset.z);
	left = ((gridpos.x + gridpos.y + gridpos.z) % 2) > 0;
	u0 = (float)get_sample(gridpos.x, gridpos.y, gridpos.z) / SAMPLE;
	u1 = (float)get_sample(gridpos.x + 1, gridpos.y, gridpos.z) / SAMPLE;
	u2 = (float)get_sample(gridpos.x, gridpos.y + 1, gridpos.z) / SAMPLE;
	u3 = (float)get_sample(gridpos.x + 1, gridpos.y + 1, gridpos.z) / SAMPLE;
	upper = interpolate_quad(u0, u1, u2, u3, GLvector2(offset.x, offset.y), left);
	l0 = (float)get_sample(gridpos.x, gridpos.y, gridpos.z + 1) / SAMPLE;
	l1 = (float)get_sample(gridpos.x + 1, gridpos.y, gridpos.z + 1) / SAMPLE;
	l2 = (float)get_sample(gridpos.x, gridpos.y + 1, gridpos.z + 1) / SAMPLE;
	l3 = (float)get_sample(gridpos.x + 1, gridpos.y + 1, gridpos.z + 1) / SAMPLE;
	lower = interpolate_quad(l0, l1, l2, l3, GLvector2(offset.x, offset.y), left);
	return interpolate(upper, lower, offset.z);
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void NoiseSeed(int seed)
{
	shift.x = seed * 397;
	shift.y = seed * 499;
	shift.z = seed * 541;
	shift.x %= 16384;
	shift.y %= 16384;
	shift.z %= 16384;
}

void NoiseInit()
{
	GLcoord2        image_size;
	int             index;
	Texture*        img;
	const unsigned char*  temp;

	img = new Texture("noise3.png");
	image_size = img->Size();
	//temp = (unsigned char*)TextureData ("noise3.png", &image_size);
	temp = (const unsigned char*)img->Data();
	size_val = MIN(image_size.x, image_size.y);
	buffer = new unsigned char[size_val * size_val];
	index = 0;
	for (int x = 0; x < size_val; x++) {
		for (int y = 0; y < size_val; y++) {
			buffer[index] = temp[(x + y * image_size.x) * 4];
			index++;
		}
	}
	img->Destroy();
	delete img;
	Console("NoiseInit: %dx%d noise map loaded.", size_val, size_val);
	NoiseSeed(1);
}

float Noisef(int seed)
{
	return (float)Noisei(seed) / 255.0f;
}

int Noisei(int seed)
{
	Octant pos = shift + seed;
	return get_sample(pos.x, pos.y, pos.z);
}

int Noisei(Octant pos)
{
	pos += shift;
	return get_sample(pos.x, pos.y, pos.z);
}

float Noisef(Octant pos_in, int freq)
{
	GLvector    origin;
	float       result;
	float       octaves;
	Octant      pos;

	pos = pos_in + shift;
	origin = GLvector((float)pos.x, (float)pos.y, (float)pos.z);
	result = 0;
	octaves = 0;
	if (freq & FREQ_LOW) {
		result += get_octave(origin * GLvector(0.002f, 0.002f, 0.002f)); octaves++;
		result += get_octave(origin * GLvector(0.004f, 0.004f, 0.004f)); octaves++;
		result += get_octave(origin * GLvector(0.008f, 0.008f, 0.008f)); octaves++;
	}
	if (freq & FREQ_MID) {
		result += get_octave(origin * GLvector(0.016f, 0.016f, 0.016f)); octaves++;
		result += get_octave(origin * GLvector(0.032f, 0.032f, 0.032f)); octaves++;
		result += get_octave(origin * GLvector(0.064f, 0.064f, 0.064f)); octaves++;
	}
	if (freq & FREQ_HIGH) {
		result += get_octave(origin * GLvector(0.128f, 0.128f, 0.128f)); octaves++;
		result += get_octave(origin * GLvector(0.256f, 0.256f, 0.256f)); octaves++;
		result += get_octave(origin * GLvector(0.512f, 0.512f, 0.512f)); octaves++;
	}
	//result += get_octave (origin * GLvector (0.256f, 0.256f, 0.256)); octaves++;
	result /= octaves;
	result -= 0.5f;
	result *= 2.6f;
	result += 0.5f;
	result = UNIT(result);
	return result;
}