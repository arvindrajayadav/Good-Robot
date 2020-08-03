#ifndef GLTYPES_H
#define GLTYPES_H

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

using namespace std;

#define WRAP(x,y)                 ((unsigned)x % y)
#define SIGN(x)                   (((x) > 0) ? 1 : ((x) < 0) ? -1 : 0)
#define SIGNF(x)                  (((x) > 0.0f) ? 1.0f : ((x) < 0.0f) ? -1.0f : 0.0f)
#define ABS(x)                    (((x) < 0 ? (-x) : (x)))
#define SMALLEST(x,y)             (ABS(x) < ABS(y) ? 0 : x)
#define INTERPOLATE(a,b,delta)    (a * (1.0f - delta) + b * delta)
#define MIN(a, b)                 (((a) < (b)) ? (a) : (b))
#define MAX(a, b)                 (((a) > (b)) ? (a) : (b))
#define CLAMP(n,lower,upper)      (MAX (MIN(n,(upper)), (lower)))
#define UNIT(n)                   (CLAMP(n,0,1))

#define MAX_VALUE                 999999999999999.9f

#define OPERATORS(type)     \
  void    operator+= (const type& c);\
  void    operator+= (const float& c);\
  void    operator-= (const type& c);\
  void    operator-= (const float& c);\
  void    operator*= (const type& c);\
  void    operator*= (const float& c);\
  void    operator/= (const type& c);\
  void    operator/= (const float& c);\
  type    operator+  (const type& c); \
  type    operator+  (const float& c);\
  type    operator-  (const type& c);\
  type    operator-  (const float& c);\
  type    operator*  (const type& c);\
  type    operator*  (const float& c);\
  type    operator/  (const type& c);\
  type    operator/  (const float& c);\
  bool    operator!= (const type& c);\
  bool    operator== (const type& c);

/*-----------------------------------------------------------------------------
GLrgba
-----------------------------------------------------------------------------*/

struct GLrgba
{
	float       red;
	float       green;
	float       blue;
	float       alpha;
	GLrgba() : red(0), green(0), blue(0), alpha(1) {}
	GLrgba(float r, float g, float b, float a) : red(r), green(g), blue(b), alpha(a) {}
	GLrgba(float r, float g, float b) : red(r), green(g), blue(b), alpha(1) {}
	GLrgba(struct GLrgbi);
	void        Clamp();
	void        Negative();
	float       Luminance() const { return (red + green + blue) / 3.0f; }

	GLrgba operator* (const GLrgba& c) { return GLrgba(red*c.red, green*c.green, blue*c.blue); }
	GLrgba operator+ (const GLrgba& c) { return GLrgba(red + c.red, green + c.green, blue + c.blue); }
	GLrgba operator/ (const GLrgba& c) { return GLrgba(red / c.red, green / c.green, blue / c.blue); }

	GLrgba operator/ (const float& f)  { return GLrgba(red / f, green / f, blue / f); }
	GLrgba operator* (const float& f)  { return GLrgba(red*f, green*f, blue*f); }

	void   operator+=(const GLrgba& c) { red += c.red; green += c.green; blue += c.blue; }
	void   operator/=(const float& f) { red /= f; green /= f; blue /= f; }
	void   operator*=(const GLrgba& c) { red *= c.red; green *= c.green; blue *= c.blue; }
	void   operator*=(const float& f) { red *= f; green *= f; blue *= f; }
	bool   operator==(const GLrgba& c)  { if (red == c.red && green == c.green && blue == c.blue)  return true; return false; }
};

GLrgba        GLrgbaFromHex(string hex);
std::string   GLrgbaToHex(GLrgba c);
GLrgba        GLrgbaLerp(GLrgba c1, GLrgba c2, float delta);

/*-----------------------------------------------------------------------------
Color based on unsigned chars
-----------------------------------------------------------------------------*/

struct GLrgbi
{
	uchar red, green, blue;

	GLrgbi() {}
	GLrgbi(uchar r, uchar g, uchar b) : red(r), green(g), blue(b) {}
	GLrgbi(GLrgba c) { red = (uchar)(c.red * 255.0f); green = (uchar)(c.green * 255.0f); blue = (uchar)(c.blue * 255.0f); }

	void   operator/=(const int& f) { red /= f; green /= f; blue /= f; }
};

/*-----------------------------------------------------------------------------
Takes the given index and returns a "random" color unique for that index.
512 Unique values: #0 and #512 will be the same, as will #1 and #513, etc
Useful for visual debugging in some situations.
-----------------------------------------------------------------------------*/

GLrgba GLrgbaUnique(unsigned i);

/*-----------------------------------------------------------------------------
GLcoord
-----------------------------------------------------------------------------*/

struct GLcoord2
{
	int         x;
	int         y;
	GLcoord2() : x(0), y(0) {}
	GLcoord2(int x_in, int y_in) : x(x_in), y(y_in) {}

	bool        Walk(int size) { return Walk(size, size); }
	void        Clear() { x = y = 0; }
	bool        Walk(int x_size, int y_size);

	bool        operator== (const GLcoord2& c)  { if (x == c.x && y == c.y)  return true; return false; }
	bool        operator!= (const GLcoord2& c)  { if (x == c.x && y == c.y)  return false; return true; }

	GLcoord2    operator+  (const int& c)       { GLcoord2 result;  result.x = x + c;  result.y = y + c;  return result; }
	GLcoord2    operator+  (const GLcoord2& c)  { GLcoord2 result;  result.x = x + c.x;  result.y = y + c.y;  return result; }
	void        operator+= (const float& c)     { x += (int)c; y += (int)c; };
	void        operator+= (const int& c)       { x += c; y += c; };
	void        operator+= (const GLcoord2& c)  { x += c.x; y += c.y; };

	GLcoord2    operator-  (const int& c)       { GLcoord2 result;  result.x = x - c;  result.y = y - c;  return result; }
	GLcoord2    operator-  (const GLcoord2& c)  { GLcoord2 result; result.x = x - c.x; result.y = y - c.y;  return result; }

	void        operator-= (const float& c)     { x -= (int)c; y -= (int)c; };
	void        operator-= (const int& c)       { x -= c; y -= c; };
	void        operator-= (const GLcoord2& c)  { x -= c.x; y -= c.y; };

	GLcoord2    operator*  (const int& c)       { GLcoord2 result;  result.x = x * c;  result.y = y * c;  return result; }
	GLcoord2    operator*  (const GLcoord2& c);
	void        operator*= (const int& c) { x *= c; y *= c; };
	void        operator*= (const GLcoord2& c) { x *= c.x; y *= c.y; };

	GLcoord2    operator/  (const int& c)       { GLcoord2 result;  result.x = x / c;  result.y = y / c;  return result; }
	GLcoord2    operator/  (const GLcoord2& c);
	void        operator/= (const int& c) { x /= c; y /= c; };
	void        operator/= (const GLcoord2& c) { x /= c.x; y /= c.y; };
};

struct GLcoord3
{
	int     x, y, z;

	GLcoord3() : x(0), y(0), z(0) {}
	GLcoord3(int x_in, int y_in, int z_in) : x(x_in), y(y_in), z(z_in) {}

	bool        operator== (const GLcoord3& c)  { if (x == c.x && y == c.y && z == c.z)  return true; return false; }
	bool        operator!= (const GLcoord3& c)  { if (x == c.x && y == c.y && z == c.z)  return false; return true; }

	GLcoord3    operator+  (const GLcoord3& c)  { GLcoord3 result;  result.x = x + c.x;  result.y = y + c.y;  result.z = z + c.z; return result; }
	void        operator+= (const GLcoord3& c)  { x += c.x; y += c.y; z += c.z; };
	GLcoord3    operator+  (const int& c)       { GLcoord3 result;  result.x = x + c;  result.y = y + c; result.z = z + c; return result; }
	void        operator+= (const int& c)       { x += c; y += c; z += z; };

	GLcoord3    operator-  (const GLcoord3& c)  { GLcoord3 result;  result.x = x - c.x;  result.y = y - c.y;  result.z = z - c.z; return result; }
	void        operator-= (const GLcoord3& c)  { x -= c.x; y -= c.y; z -= c.z; };
	GLcoord3    operator-  (const int& c)       { GLcoord3 result;  result.x = x - c;  result.y = y - c; result.z = z - c; return result; }
	void        operator-= (const int& c)       { x -= c; y -= c; z -= c; };

	GLcoord3    operator*  (const GLcoord3& c)  { GLcoord3 result;  result.x = x * c.x;  result.y = y * c.y;  result.z = z * c.z; return result; }
	void        operator*= (const GLcoord3& c)  { x *= c.x; y *= c.y; z *= c.z; };
	GLcoord3    operator*  (const int& c)       { GLcoord3 result;  result.x = x * c;  result.y = y * c; result.z = z * c; return result; }
	void        operator*= (const int& c)       { x *= c; y *= c; z *= c; };

	GLcoord3    operator/  (const GLcoord3& c)  { GLcoord3 result;  result.x = x / c.x;  result.y = y / c.y;  result.z = z / c.z; return result; }
	void        operator/= (const GLcoord3& c)  { x /= c.x; y /= c.y; z /= c.z; };
	GLcoord3    operator/  (const int& c)       { GLcoord3 result;  result.x = x / c;  result.y = y / c; result.z = z / c; return result; }
	void        operator/= (const int& c)       { x /= c; y /= c; z /= c; };

	GLcoord3    operator%  (const GLcoord3& c)  { GLcoord3 result;  result.x = x % c.x;  result.y = y % c.y;  result.z = z % c.z; return result; }
	void        operator%= (const GLcoord3& c)  { x %= c.x; y %= c.y; z %= c.z; };
	GLcoord3    operator%  (const int& c)       { GLcoord3 result;  result.x = x % c;  result.y = y % c; result.z = z % c; return result; }
	void        operator%= (const int& c)       { x %= c; y %= c; z %= c; };

	void    Clear() { x = y = z = 0; }
	bool    Walk(int size_x, int size_y, int size_z);
	bool    Walk(int size) { return Walk(size, size, size); }
};

/*-----------------------------------------------------------------------------
GLvector2
-----------------------------------------------------------------------------*/

struct GLvector2
{
	float       x;
	float       y;

	GLvector2() : x(0), y(0) {}
	GLvector2(float x_, float y_) : x(x_), y(y_) {}
	GLvector2(GLcoord2 c) : x((float)c.x), y((float)c.y) {}

	float       Length() { return (float)sqrt(x * x + y * y); }
	void        Normalize();
	GLvector2   Normalized() const;
	float       Angle();
	bool        IsZero() { if ((x == 0) && (y == 0)) return true; return false; }
	GLvector2   TurnedRight() { GLvector2  result; result.x = -y; result.y = x; return result; }
  GLvector2   Rotated (float angle) const;

	GLvector2 operator+ (const GLvector2& c) { return GLvector2(x + c.x, y + c.y); }
	GLvector2 operator+ (const float& c)  { return GLvector2(x + c, y + c); }
	GLvector2 operator-(const GLvector2& c) { return GLvector2(x - c.x, y - c.y); }
	GLvector2 operator-(const float& c)  { return GLvector2(x - c, y - c); }
	GLvector2 operator* (const GLvector2& c)  { return GLvector2(x * c.x, y * c.y); }
	GLvector2 operator* (const float& c)  { return GLvector2(x * c, y * c); }

	GLvector2 &operator+=(const GLvector2 &p)   { x += p.x;      y += p.y;  return *this; }
	//void operator+= (const GLvector2& c)   {  x += c.x;  y += c.y; }
	void operator+= (const float& c)  { x += c;  y += c; }
	void operator-= (const GLvector2& c)  { x -= c.x;    y -= c.y; }
	void operator-= (const float& c)  { x -= c;    y -= c; }
	void operator*= (const GLvector2& c)  { x *= c.x;    y *= c.y; }
	void operator*= (const float& c)  { x *= c;    y *= c; }

	GLvector2 operator/ (const GLvector2& c)  { return GLvector2(x / c.x, y / c.y); }
	GLvector2 operator/ (const float& c)  { return GLvector2(x / c, y / c); }
	void operator/= (const GLvector2& c)  { x /= c.x;    y /= c.y; }
	void operator/= (const float& c)  { x /= c;    y /= c; }
	bool operator== (const GLvector2& c)  { if (x == c.x && y == c.y)      return true;    return false; }
	bool operator!= (const GLvector2& c)  { if (x != c.x || y != c.y)      return true;    return false; }
};

/*-----------------------------------------------------------------------------
GLvector
-----------------------------------------------------------------------------*/

struct GLvector
{
	float x, y, z;

	GLvector() : x(0), y(0), z(0)
	{
	}

	GLvector(GLvector2 a) : x(a.x), y(a.y), z(0)
	{
	}

	GLvector(float x_, float y_, float z_) : x(x_), y(y_), z(z_)
	{
	}

	bool operator== (const GLvector& c) { if (x == c.x && y == c.y && z == c.z)  return true;  return false; }
	bool operator!= (const GLvector& c) { if (x == c.x && y == c.y && z == c.z)  return false;  return true; }

	GLvector operator-(const GLvector &p) const  { return GLvector(*this) -= p; }
	GLvector operator-(const float &p) const  { return GLvector(x - p, y - p, z - p); }
	GLvector operator*(float f) const  { return GLvector(*this) *= f; }
	GLvector operator/(float f) const    { return GLvector(*this) /= f; }
	GLvector &operator+=(const GLvector &p)  { x += p.x;  y += p.y;   z += p.z; return *this; }
	GLvector operator+(const GLvector &p) const   { return GLvector(x + p.x, y + p.y, z + p.z); }
	GLvector operator*(const GLvector &p) const   { return GLvector(x*p.x, y*p.y, z*p.z); }
	GLvector operator+ (const GLvector& c) { return GLvector(x + c.x, y + c.y, z + c.z); }
	GLvector operator+ (const float& c)  { return GLvector(x + c, y + c, z + c); }
	GLvector &operator-=(const GLvector &p)  { x -= p.x;   y -= p.y;       z -= p.z;  return *this; }
	GLvector &operator*=(const GLvector &p)  { x *= p.x;   y *= p.y;       z *= p.z;  return *this; }

	GLvector &operator*=(float f)   { x *= f;  y *= f;  z *= f;   return *this; }
	GLvector &operator/=(float f)   { x /= f;  y /= f;  z /= f;   return *this; }
	float &operator[](unsigned int index) { return (&x)[index]; }
	const float &operator[](unsigned int index) const { return (&x)[index]; }

	bool      IsZero() { if ((x == 0) && (y == 0) && (z == 0)) return true; return false; }
	float     Length() const { return sqrt(x * x + y *y + z*z); }
	void      Normalize();
	GLvector  Normalized() const;
	void      FromGLcoord(GLcoord3 c) { x = (float)c.x; y = (float)c.y; z = (float)c.z; }
	void      Clear() { x = 0; y = 0; z = 0; }
};

inline float GLdot(const GLvector &a, const GLvector &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

inline GLvector GLcross(const GLvector &a, const GLvector &b)
{
	return GLvector(a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x);
}

inline GLvector GLreflect2(const GLvector in, const GLvector normal)
{
	GLvector  result;

	result = in - (normal * GLdot(in, normal) * 2.0f);
	return result;
}

inline GLvector2 GLreflect2(const GLvector2 in, const GLvector2 norm)
{
	GLvector result;

	result = GLreflect2(GLvector(in.x, in.y, 0), GLvector(norm.x, norm.y, 0));
	return GLvector2(result.x, result.y);
}

GLvector GLvectorLerp(GLvector start, GLvector end, float delta);

inline GLvector GLvectorFromGLcoord3(GLcoord3 c)
{
	GLvector result;

	result.FromGLcoord(c);
	return result;
}

/*-----------------------------------------------------------------------------
GLbbox2
-----------------------------------------------------------------------------*/

struct GLbbox2
{
	GLvector2   pmin;
	GLvector2   pmax;

	GLvector2   Center() const {
		GLvector2  avg;
		avg = pmin;
		avg += pmax;
		return avg / 2.0;
	}
	void        ContainPoint(GLvector2 point)
	{
		pmin.x = MIN(pmin.x, point.x);
		pmin.y = MIN(pmin.y, point.y);
		pmax.x = MAX(pmax.x, point.x);
		pmax.y = MAX(pmax.y, point.y);
	}
	bool        Contains(GLvector2 pt) const
	{
		if (pt.x >= pmin.x && pt.x <= pmax.x && pt.y >= pmin.y && pt.y <= pmax.y)
			return true;
		return false;
	}
	void        Clear(void)
	{
		pmax = GLvector2(-MAX_VALUE, -MAX_VALUE);
		pmin = GLvector2(MAX_VALUE, MAX_VALUE);
	}
	void        Render()
	{
		glBegin(GL_LINE_STRIP);
		glVertex2f(pmin.x, pmin.y);
		glVertex2f(pmax.x, pmin.y);
		glVertex2f(pmax.x, pmax.y);
		glVertex2f(pmin.x, pmax.y);
		glVertex2f(pmin.x, pmin.y);
		glEnd();
	}
	GLvector2   Size() { return pmax - pmin; }
};

/*-----------------------------------------------------------------------------
GLbbox
-----------------------------------------------------------------------------*/

struct GLbbox
{
	GLvector    pmin;
	GLvector    pmax;

	GLvector    Center() {
		GLvector  avg;
		avg = pmin + pmax;
		return avg / 2.0;
		//return (pmin + pmax) / 2.0;
	}
	GLvector    Size() { return pmax - pmin; }
	void        ContainPoint(GLvector point)
	{
		pmin.x = MIN(pmin.x, point.x);
		pmin.y = MIN(pmin.y, point.y);
		pmin.z = MIN(pmin.z, point.z);
		pmax.x = MAX(pmax.x, point.x);
		pmax.y = MAX(pmax.y, point.y);
		pmax.z = MAX(pmax.z, point.z);
	}
	void        Clear(void)
	{
		pmax = GLvector(-MAX_VALUE, -MAX_VALUE, -MAX_VALUE);
		pmin = GLvector(MAX_VALUE, MAX_VALUE, MAX_VALUE);
	}
	void        Render()
	{
		//Bottom of box (Assuming z = up)
		glBegin(GL_LINE_STRIP);
		glVertex3f(pmin.x, pmin.y, pmin.z);
		glVertex3f(pmax.x, pmin.y, pmin.z);
		glVertex3f(pmax.x, pmax.y, pmin.z);
		glVertex3f(pmin.x, pmax.y, pmin.z);
		glVertex3f(pmin.x, pmin.y, pmin.z);
		glEnd();
		//Top of box
		glBegin(GL_LINE_STRIP);
		glVertex3f(pmin.x, pmin.y, pmax.z);
		glVertex3f(pmax.x, pmin.y, pmax.z);
		glVertex3f(pmax.x, pmax.y, pmax.z);
		glVertex3f(pmin.x, pmax.y, pmax.z);
		glVertex3f(pmin.x, pmin.y, pmax.z);
		glEnd();
		//Sides
		glBegin(GL_LINES);
		glVertex3f(pmin.x, pmin.y, pmin.z);
		glVertex3f(pmin.x, pmin.y, pmax.z);

		glVertex3f(pmax.x, pmin.y, pmin.z);
		glVertex3f(pmax.x, pmin.y, pmax.z);

		glVertex3f(pmax.x, pmax.y, pmin.z);
		glVertex3f(pmax.x, pmax.y, pmax.z);

		glVertex3f(pmin.x, pmax.y, pmin.z);
		glVertex3f(pmin.x, pmax.y, pmax.z);
		glEnd();
	}
};

/*-----------------------------------------------------------------------------
GLrect
-----------------------------------------------------------------------------*/

struct GLrect
{
	GLvector2 ul;
	GLvector2 lr;

	GLrect() : ul(GLvector2(0, 0)), lr(GLvector2(1, 1)) {}
	GLrect(GLvector2 upper_left, GLvector2 lower_right) { ul = upper_left; lr = lower_right; }
	GLvector2 UpperLeft() { return ul; }
	GLvector2 LowerRight() { return lr; }
	GLvector2 LowerLeft() { return GLvector2(ul.x, lr.y); }
	GLvector2 UpperRight() { return GLvector2(lr.x, ul.y); }
	void      UvFrame(int row, int col, int grid)
	{
		float   size;

		size = 1.0f / (float)grid;
		ul = GLvector2(row * size, col * size);
		lr = ul + size;
	}
};

/*-----------------------------------------------------------------------------
GLuvFrame - Good for keeping track of the four corners of a single section
on a sprite sheet. GLrect above was intended for the same purpose, but it was
too inconvenient to use.

The points are stored in clockwise order, assuming left-to-right,
top-to-bottom UV system. (OpenGL actually uses bottom-to-top, but that's
confusing and stupid so I'm always inverting it.)
-----------------------------------------------------------------------------*/

struct GLuvFrame
{
	GLvector2 uv[4];

	void    Set(GLvector2 upper_left, GLvector2 lower_right)
	{
		uv[0] = upper_left;
		uv[1] = GLvector2(lower_right.x, upper_left.y);
		uv[2] = lower_right;
		uv[3] = GLvector2(upper_left.x, lower_right.y);
		//Invert to openGL bottom-up
		for (int i = 0; i < 4; i++)
			uv[i].y = 1 - uv[i].y;
	}
	void     Mirror()
	{
		float   temp;

		temp = uv[1].x;
		uv[1].x = uv[0].x;
		uv[0].x = temp;
		temp = uv[3].x;
		uv[3].x = uv[2].x;
		uv[2].x = temp;
	};
	GLvector2  Size()
	{
		GLvector2 size = uv[0] - uv[2];
		return size;
	}
};

/*-----------------------------------------------------------------------------
Structurally identical to GLuvFrame, but the intended use is different
so I'm giving it its own type. This is for tracking the corners of a quad.
Unlike a bbox2, these corners are not aligned on an axis. (Not orthogonal.)
-----------------------------------------------------------------------------*/

struct GLquad
{
	GLvector2 corner[4];
};

/*-----------------------------------------------------------------------------
GLmatrix
-----------------------------------------------------------------------------*/

struct GLmatrix
{
	float       elements[4][4];
	void        Identity();
	void        Rotate(float theta, float x, float y, float z);
	void        Multiply(GLmatrix m);
	GLvector    TransformPoint(GLvector pt);
};

GLmatrix  glMatrixIdentity(void);
void      glMatrixElementsSet(GLmatrix* m, float* in);
GLmatrix  glMatrixMultiply(GLmatrix a, GLmatrix b);
GLmatrix  glMatrixScale(GLmatrix m, GLvector in);
GLvector  glMatrixTransformPoint(GLmatrix m, GLvector in);
GLmatrix  glMatrixTranslate(GLmatrix m, GLvector in);
GLmatrix  glMatrixRotate(GLmatrix m, float theta, float x, float y, float z);
GLvector  glMatrixToEuler(GLmatrix mat, int order);

/*-----------------------------------------------------------------------------
GLmesh
-----------------------------------------------------------------------------*/

struct GLmesh
{
	GLbbox            _bbox;
	vector<unsigned>  _index;
	vector<GLvector>  _vertex;
	vector<GLvector>  _normal;
	vector<GLrgba>    _color;
	vector<GLvector2> _uv;

	void              CalculateNormals();
	void              CalculateNormalsSeamless();
	void              Clear();
	void              PushTriangle(unsigned i1, unsigned i2, unsigned i3);
	void              PushQuad(unsigned i1, unsigned i2, unsigned i3, unsigned i4);
	void              PushQuad();
	void              PushVertex(GLvector vert, GLvector normal, GLvector2 uv);
	void              PushVertex(GLvector vert, GLvector normal, GLrgba color, GLvector2 uv);
	void              PushVertex(GLvector vert, GLrgba color, GLvector2 uv);
	void              PushVertex(GLvector vert, GLvector2 uv);
	void              RecalculateBoundingBox();
	void              Render();
	void              Normalize();
	void              Move(GLvector offset);
	unsigned          AdjacencyNeighbor(int i1, int i2);
	unsigned          AdjacencyNeighbor(int i1, int i2, int current);
	void              TrianglesToAdjacency();
	void              TrianglesToAdjacency(int* search_indicies);
	unsigned          Triangles() const { return _index.size() / 3; }
	unsigned          Vertices() const { return _vertex.size(); }

	void    operator+= (const GLmesh& c);
};

//Get an angle between two given points on a grid
float     Angle2D(float x1, float y1, float x2, float y2);
//difference between two angles
float     AngleDifference(float a1, float a2);
//Keep an angle in the 0-360 range.
int       AngleLimit(int angle);
float     AngleLimit(float angle);
//take values from 0.0 to 1.0 and convert them to values along a curve.
float     Curve(float val);
//interpolate between two values
float     Lerp(float start, float end, float delta);
GLvector2 Lerp(GLvector2 start, GLvector2 end, float delta);
GLvector  Lerp(GLvector start, GLvector end, float delta);
GLrgba    Lerp(GLrgba start, GLrgba end, float delta);
//interpolate between four values
float     Lerp2D(float up_left, float up_right, float down_left, float down_right, GLvector2 offset, bool left = false);
//Find the first power of two large enough to contain the given value.
int       PowerOf2(int a);
//return a scalar of 0.0 to 1.0, based an the given values position within a range
float     SmoothStep(float start, float end, float position);
//Keep a value within the given range.
int       Wrap(int v, int size);
float     Wrap(float v, float size);
GLcoord2  Wrap(GLcoord2 v, int size);

GLvector2 GLvectorFromAngle(float angle);
GLcoord3  GLcoordFromVector(struct GLvector c);
GLcoord2  GLcoordFromVector(struct GLvector2 c);

#endif // GLTYPES_H
