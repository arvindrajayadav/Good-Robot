#ifndef NOISE_H
#define NOISE_H

#define FREQ_LOW  0x01
#define FREQ_MID  0x02
#define FREQ_HIGH 0x04
#define FREQ_ALL  0x07

struct Octant
{
	int   x, y, z;
	//structors
	Octant() : x(0), y(0), z(0) {}
	Octant(int _x, int _y, int _z) : x(_x), y(_y), z(_z) {}

	void  Clear()
	{
		x = y = z = 0;
	}
	//Given size, which of the eight sub-nodes would this pos be in?
	Octant getOctant(int size)
	{
		Octant  result;

		result = Octant();
		size /= 2;
		if (x >= size)
			result.x = 1;
		if (y >= size)
			result.y = 1;
		if (z >= size)
			result.z = 1;
		return result;
	}
	bool    Walk(int size_x, int size_y, int size_z)
	{
		x++;
		if (x == size_x) {
			x = 0;
			y++;
		}
		if (y == size_y) {
			y = 0;
			z++;
		}
		if (z == size_z) {
			x = y = z = 0;
			return true;
		}
		return false;
	}
	bool  Walk(int size) { return Walk(size, size, size); }

	bool operator== (const Octant& c) { if (x == c.x && y == c.y && z == c.z)  return true;    return false; }
	bool operator!= (const Octant& c) { if (x == c.x && y == c.y && z == c.z)  return false;    return true; }
	//same-type operators
	Octant operator+ (const Octant& n) { return Octant(x + n.x, y + n.y, z + n.z); }
	Octant operator- (const Octant& n) { return Octant(x - n.x, y - n.y, z - n.z); }
	Octant operator* (const Octant& n) { return Octant(x*n.x, y*n.y, z*n.z); }
	Octant operator/ (const Octant& n) { return Octant(x / n.x, y / n.y, z / n.z); }
	void   operator+=(const Octant& n) { x += n.x; y += n.y; z += n.z; }
	void   operator-=(const Octant& n) { x -= n.x; y -= n.y; z -= n.z; }
	//int operators
	Octant operator+ (const int& n) { return Octant(x + n, y + n, z + n); }
	Octant operator- (const int& n) { return Octant(x - n, y - n, z - n); }
	Octant operator* (const int& n) { return Octant(x * n, y * n, z * n); }
	Octant operator/ (const int& n) { return Octant(x / n, y / n, z / n); }
	Octant operator% (const int& n) { return Octant(x % n, y % n, z % n); }
	void   operator+=(const int& n) { x += n; y += n; z += n; }
	void   operator-=(const int& n) { x -= n; y -= n; z -= n; }
	void   operator*=(const int& n) { x *= n; y *= n; z *= n; }
	void   operator/=(const int& n) { x /= n; y /= n; z /= n; }
	void   operator%=(const int& n) { x %= n; y %= n; z %= n; }
};

void  NoiseInit();
float Noisef(Octant pos, int freq = FREQ_ALL);
int   Noisei(Octant pos);
int   Noisei(int seed);
float Noisef(int seed);
void  NoiseSeed(int seed);

#endif // NOISE_H
