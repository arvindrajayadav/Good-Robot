/*-----------------------------------------------------------------------------

  glTypes.cpp

  This module began as a container for various special GL data types used in
  rendering: Vectors, meshes, matrices, etc. It's evolved into a toolkit
  of functions and types used in rendering.

  (c) 2014 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include <math.h>

//This is a list of the integers from 0 to 511, in random order. Used for
//scrambling the unique colors function.

static int    color_mix[] =
{
	0x17D, 0x1DA, 0x008, 0x00F, 0x12C, 0x110, 0x09F, 0x157, 0x1CF, 0x1A6, 0x17C, 0x1A4, 0x035, 0x141, 0x174, 0x119,
	0x0A7, 0x0B0, 0x1BE, 0x0D2, 0x14D, 0x007, 0x13F, 0x09E, 0x108, 0x030, 0x058, 0x1D0, 0x1F4, 0x0F3, 0x0C0, 0x18C,
	0x0A5, 0x03B, 0x070, 0x1C1, 0x061, 0x0CD, 0x14F, 0x1D3, 0x1DC, 0x08F, 0x1A4, 0x183, 0x0E6, 0x0FB, 0x1E0, 0x1A5,
	0x192, 0x17B, 0x19A, 0x1F0, 0x068, 0x032, 0x0CE, 0x145, 0x1E3, 0x1E2, 0x1BA, 0x102, 0x0EA, 0x002, 0x155, 0x1E9,
	0x170, 0x19C, 0x0E8, 0x1A7, 0x065, 0x14B, 0x1CC, 0x123, 0x03D, 0x1D8, 0x1CD, 0x021, 0x0ED, 0x1A3, 0x0C0, 0x10F,
	0x198, 0x07C, 0x1AC, 0x0A0, 0x1BC, 0x0FE, 0x147, 0x0AB, 0x1D6, 0x186, 0x111, 0x158, 0x11B, 0x0C7, 0x158, 0x0B3,
	0x133, 0x1BE, 0x11F, 0x0BE, 0x193, 0x088, 0x1EE, 0x1F6, 0x06B, 0x169, 0x166, 0x154, 0x0D0, 0x0D8, 0x15F, 0x0BD,
	0x088, 0x00E, 0x161, 0x097, 0x1F2, 0x18F, 0x192, 0x1FB, 0x0DA, 0x0B4, 0x0AF, 0x1A5, 0x179, 0x0DD, 0x1E8, 0x028,
	0x18A, 0x01D, 0x117, 0x1F1, 0x1AC, 0x086, 0x13C, 0x159, 0x0A9, 0x113, 0x09A, 0x186, 0x0DC, 0x143, 0x19D, 0x052,
	0x0D5, 0x061, 0x0BA, 0x0E9, 0x13E, 0x1B4, 0x1FD, 0x0C2, 0x0F6, 0x1DD, 0x150, 0x157, 0x13A, 0x1AD, 0x012, 0x09F,
	0x095, 0x151, 0x036, 0x1E1, 0x0EF, 0x08D, 0x18D, 0x1B9, 0x1E6, 0x176, 0x1C2, 0x0CF, 0x1B6, 0x1B8, 0x1DE, 0x05E,
	0x04E, 0x183, 0x152, 0x078, 0x01B, 0x16A, 0x1FD, 0x0DB, 0x1ED, 0x051, 0x0FE, 0x116, 0x14E, 0x1D1, 0x09B, 0x189,
	0x056, 0x137, 0x091, 0x09D, 0x0E3, 0x1DB, 0x0FC, 0x0F0, 0x1D0, 0x1FE, 0x163, 0x1E9, 0x16F, 0x0AE, 0x05C, 0x1BC,
	0x1E3, 0x10C, 0x1DE, 0x11B, 0x149, 0x0BB, 0x18A, 0x1B5, 0x11D, 0x0AD, 0x1F7, 0x020, 0x119, 0x1D9, 0x108, 0x1AF,
	0x0CC, 0x15C, 0x17D, 0x1F3, 0x118, 0x1CA, 0x168, 0x03F, 0x1A2, 0x0AF, 0x0A1, 0x0F6, 0x0D4, 0x05C, 0x0EC, 0x1FF,
	0x01C, 0x015, 0x1C1, 0x16C, 0x199, 0x100, 0x160, 0x14E, 0x14C, 0x126, 0x095, 0x1EB, 0x083, 0x14A, 0x0CB, 0x1A3,
	0x16B, 0x182, 0x1C2, 0x1C8, 0x13A, 0x170, 0x1E0, 0x04B, 0x1D6, 0x0CB, 0x1B8, 0x14C, 0x02B, 0x179, 0x1B2, 0x1C4,
	0x168, 0x017, 0x18B, 0x19E, 0x0FD, 0x0A5, 0x148, 0x04D, 0x0B1, 0x0B6, 0x1F7, 0x046, 0x1BF, 0x12B, 0x1A8, 0x084,
	0x156, 0x0C4, 0x131, 0x080, 0x0E2, 0x13D, 0x1CB, 0x151, 0x1D9, 0x1F3, 0x02E, 0x116, 0x01E, 0x167, 0x1C3, 0x125,
	0x132, 0x190, 0x199, 0x1FC, 0x1D5, 0x03A, 0x18B, 0x0C5, 0x08E, 0x0EA, 0x039, 0x18E, 0x099, 0x048, 0x001, 0x12D,
	0x1AA, 0x10D, 0x02A, 0x0A9, 0x164, 0x0F5, 0x0EB, 0x1A8, 0x11E, 0x15D, 0x1C7, 0x1AF, 0x188, 0x048, 0x0D1, 0x1E2,
	0x098, 0x1FE, 0x19F, 0x1A1, 0x0A8, 0x050, 0x1B5, 0x06D, 0x160, 0x1EE, 0x15C, 0x16E, 0x163, 0x154, 0x196, 0x1FF,
	0x0F3, 0x038, 0x059, 0x064, 0x181, 0x1D2, 0x133, 0x190, 0x125, 0x0BD, 0x16F, 0x1E5, 0x1AB, 0x101, 0x17F, 0x140,
	0x060, 0x1ED, 0x194, 0x0FF, 0x0C1, 0x1CB, 0x0CA, 0x069, 0x1C6, 0x1D4, 0x1C0, 0x0D9, 0x171, 0x1F2, 0x1D3, 0x128,
	0x173, 0x1D7, 0x17E, 0x191, 0x05A, 0x19F, 0x0DF, 0x189, 0x1E1, 0x1FA, 0x1A2, 0x03E, 0x0FC, 0x1EA, 0x1BB, 0x102,
	0x12C, 0x185, 0x13F, 0x114, 0x191, 0x0DB, 0x1FB, 0x1BF, 0x1A9, 0x066, 0x174, 0x0E8, 0x0A3, 0x197, 0x0CD, 0x134,
	0x18D, 0x0F4, 0x0DC, 0x0FA, 0x1F8, 0x1DF, 0x03D, 0x1D7, 0x000, 0x1D4, 0x1C9, 0x072, 0x055, 0x1C5, 0x1DA, 0x1DB,
	0x173, 0x10A, 0x080, 0x1E6, 0x148, 0x19A, 0x1B9, 0x177, 0x1F6, 0x1B7, 0x1CE, 0x1FC, 0x1C7, 0x175, 0x1DC, 0x1B0,
	0x0FB, 0x107, 0x1EF, 0x111, 0x054, 0x122, 0x1EB, 0x15E, 0x110, 0x129, 0x0BA, 0x162, 0x0C2, 0x136, 0x146, 0x1B6,
	0x184, 0x1F5, 0x1F9, 0x0C8, 0x196, 0x177, 0x07F, 0x112, 0x17F, 0x094, 0x120, 0x0DA, 0x143, 0x109, 0x198, 0x16D,
	0x0D3, 0x1C8, 0x195, 0x10F, 0x06B, 0x1EA, 0x035, 0x031, 0x0D7, 0x12A, 0x14B, 0x0E4, 0x1F9, 0x0BC, 0x152, 0x074,
	0x15D, 0x060, 0x1A1, 0x0C9, 0x043, 0x10E, 0x121, 0x194, 0x1C0, 0x1E4, 0x079, 0x02C, 0x1A9, 0x178, 0x086, 0x1A6,
};

static float lerp(float n1, float n2, float delta)
{
	return n1 * (1.0f - delta) + n2 * delta;
}

/*-----------------------------------------------------------------------------
RGBA type
-----------------------------------------------------------------------------*/

static bool is_hex_digit(char c)
{
	if (c >= '0' && c <= '9')
		return true;
	if (c >= 'a' && c <= 'f')
		return true;
	if (c >= 'A' && c <= 'F')
		return true;
	return false;
}

GLrgba::GLrgba(struct GLrgbi c)
{
	red = (float)c.red / 255.0f;
	green = (float)c.green / 255.0f;
	blue = (float)c.blue / 255.0f;
	alpha = 1.0f;
}

void GLrgba::Clamp()
{
	red = UNIT(red);
	green = UNIT(green);
	blue = UNIT(blue);
}

void GLrgba::Negative()
{
	Clamp();
	red = 1.0f - red;
	green = 1.0f - green;
	blue = 1.0f - blue;
}

std::string GLrgbaToHex(GLrgba c)
{
	std::string   hex;
	char          tmp[4];

	c.Clamp();
	sprintf(tmp, "%x", (int)(c.red * 15.0f));
	hex += tmp;
	sprintf(tmp, "%x", (int)(c.green * 15.0f));
	hex += tmp;
	sprintf(tmp, "%x", (int)(c.blue * 15.0f));
	hex += tmp;
	return hex;
}

GLrgba  GLrgbaFromHex(string hex)
{
	string      red, green, blue;
	GLrgba      result;
	float       scale;

  if (hex.size () > 1 && hex.c_str ()[0] == '#') {
    hex.erase (0, 1);
  }
	if (hex.length() >= 6) {
		red = hex.substr(0, 2);
		green = hex.substr(2, 2);
		blue = hex.substr(4, 2);
		scale = 255.0f;
	} else if (hex.length() >= 3) {
		red = hex.substr(0, 1);
		green = hex.substr(1, 1);
		blue = hex.substr(2, 1);
		scale = 15.0f;
	}	else
		return GLrgba();
	result.red = (float)stoi(red, nullptr, 16) / scale;
  result.green = (float)stoi(green, nullptr, 16) / scale;
	result.blue = (float)stoi(blue, nullptr, 16) / scale;
	result.alpha = 1.0f;
	return result;
}

GLrgba GLrgbaUnique(unsigned i)
{
	GLrgba    c;

	i = color_mix[i % 512];
	c.alpha = 1.0f;
	c.red = 0.3f + ((i & 1) ? 0.15f : 0.0f) + ((i & 8) ? 0.2f : 0.0f) - ((i & 64) ? 0.35f : 0.0f);
	c.green = 0.3f + ((i & 2) ? 0.15f : 0.0f) + ((i & 32) ? 0.2f : 0.0f) - ((i & 128) ? 0.35f : 0.0f);
	c.blue = 0.3f + ((i & 4) ? 0.15f : 0.0f) + ((i & 16) ? 0.2f : 0.0f) - ((i & 256) ? 0.35f : 0.0f);
	return c;
}

GLrgba GLrgbaLerp(GLrgba c1, GLrgba c2, float delta)
{
	GLrgba     result;

	result.red = lerp(c1.red, c2.red, delta);
	result.green = lerp(c1.green, c2.green, delta);
	result.blue = lerp(c1.blue, c2.blue, delta);
	result.alpha = lerp(c1.alpha, c2.alpha, delta);
	return result;
}

/*-----------------------------------------------------------------------------
Coord types, both 2D and 3D. (For grid-walking and such.)
-----------------------------------------------------------------------------*/

bool  GLcoord2::Walk(int x_size, int y_size)
{
	x++;
	if (x >= x_size) {
		y++;
		x = 0;
		if (y >= y_size) {
			y = 0;
			return true;
		}
	}
	return false;
}

bool GLcoord3::Walk(int size_x, int size_y, int size_z)
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
		Clear();
		return true;
	}
	return false;
}

/*-----------------------------------------------------------------------------
Vector types.
-----------------------------------------------------------------------------*/

GLvector GLvectorLerp(GLvector start, GLvector end, float delta)
{
	GLvector   result;

	delta = clamp(delta, 0.0f, 1.0f);
	result.x = Lerp(start.x, end.x, delta);
	result.y = Lerp(start.y, end.y, delta);
	result.z = Lerp(start.z, end.z, delta);
	return result;
}

void GLvector2::Normalize()
{
	float length = Length();

	if (length != 0) {
		length = 1.0f / length;
		x *= length;
		y *= length;
	}
}

GLvector2 GLvector2::Normalized() const
{
	GLvector2   result;

	result = GLvector2(x, y);
	result.Normalize();
	return result;
}

GLvector2 GLvector2::Rotated (float angle) const
{
  GLvector2   result;
  GLvector2   sincos;

  if (angle == 0.0f)
    return *this;
  angle *= DEGREES_TO_RADIANS;
  sincos.x = sin (angle);
  sincos.y = cos (angle);
  result = GLvector2 (x * sincos.y + y * -sincos.x, x * sincos.x + y * sincos.y);
  return result;
}


float GLvector2::Angle()
{
	return 360 - Angle2D(x, y, 0, 0);
}

GLvector2 GLvectorFromAngle(float angle)
{
	GLvector2 result;

	angle -= 180;
	result.x = sin(angle * DEGREES_TO_RADIANS);
	result.y = -cosf(angle * DEGREES_TO_RADIANS);
	return result;
}

GLvector GLvector::Normalized() const
{
	float len = Length();
	if (len == 0.0f)
		return GLvector(0.0f, 0.0f, 0.0f);
	return GLvector(x / len, y / len, z / len);
}

void GLvector::Normalize()
{
	float len = Length();
	if (len == 0.0f)
		return;
	x /= len;
	y /= len;
	z /= len;
}

/*-----------------------------------------------------------------------------
GLmesh
-----------------------------------------------------------------------------*/

//We begin with a triangle of i0, i1, i2. Each segment should (if the mesh
//is fully enclosed) be shared by a neighboring triangle. Our goal is to build
//this:
//                 1-----2-----3
//                  \   / \   /
//                   \ /   \ /
//                    0-----4
//                     \   /
//                      \ /\
//                       5
//Where i0, i1, i2 become i0, i2, and i4 in the above triangle.
//Note: This is not the Triforce.

//This version does a brute-force search of the whole mesh, for every triangle.
//Not very efficient.
void GLmesh::TrianglesToAdjacency()
{
	vector<unsigned>  final_list;

	for (unsigned i = 0; i < _index.size(); i += 3) {
		final_list.push_back(_index[i + 0]);
		final_list.push_back(AdjacencyNeighbor(_index[i + 1], _index[i + 0]));
		final_list.push_back(_index[i + 1]);
		final_list.push_back(AdjacencyNeighbor(_index[i + 2], _index[i + 1]));
		final_list.push_back(_index[i + 2]);
		final_list.push_back(AdjacencyNeighbor(_index[i + 0], _index[i + 2]));
	}
	_index.clear();
	_index = final_list;
}

//This version takes a list of ints, marking the location in the tri list
//where it should begin looking for an adjacent poly. search_indicies[]
//must therefore have an int for every triangle in the mesh.
//This is a pain to set up, but can save enormous amounts of time
//compared to brute-force searching.
void GLmesh::TrianglesToAdjacency(int* search_marker)
{
	vector<unsigned>  final_list;

	for (unsigned i = 0; i < _index.size(); i += 3) {
		final_list.push_back(_index[i + 0]);
		final_list.push_back(AdjacencyNeighbor(_index[i + 1], _index[i + 0], search_marker[i / 3]));
		final_list.push_back(_index[i + 1]);
		final_list.push_back(AdjacencyNeighbor(_index[i + 2], _index[i + 1], search_marker[i / 3]));
		final_list.push_back(_index[i + 2]);
		final_list.push_back(AdjacencyNeighbor(_index[i + 0], _index[i + 2], search_marker[i / 3]));
	}
	_index.clear();
	_index = final_list;
}

//For the given two indicies, find the triangle that has the same two points
//in the same order, and return the remaining index for that triangle.
inline unsigned GLmesh::AdjacencyNeighbor(int i1, int i2)
{
	for (unsigned i = 0; i < _index.size(); i += 3) {
		if (_index[i + 0] == i1 && _index[i + 1] == i2)
			return _index[i + 2];
		if (_index[i + 1] == i1 && _index[i + 2] == i2)
			return _index[i + 0];
		if (_index[i + 2] == i1 && _index[i + 0] == i2)
			return _index[i + 1];
	}
	return 0;//Someone messed up.
}

//As above, but this version will begin at the specified index instead of
//searching from the beginning index. Note that it wraps around,
//so it WILL find a match if one is available.
inline unsigned GLmesh::AdjacencyNeighbor(int i1, int i2, int begin)
{
	for (unsigned ii = 0; ii < _index.size(); ii += 3) {
		int i = (ii + begin * 3) % _index.size();

		if (_index[i + 0] == i1 && _index[i + 1] == i2)
			return _index[i + 2];
		if (_index[i + 1] == i1 && _index[i + 2] == i2)
			return _index[i + 0];
		if (_index[i + 2] == i1 && _index[i + 0] == i2)
			return _index[i + 1];
	}
	return 0;//Someone messed up.
}

void GLmesh::Normalize()
{
	if (_normal.size() == 0)
		return;
	for (unsigned i = 0; i < Vertices(); i++)
		_normal[i].Normalize();
}

void GLmesh::Render()
{
	unsigned      i;

	glBegin(GL_TRIANGLES);
	for (i = 0; i < _index.size(); i++) {
		if (!_normal.empty())
			glNormal3fv(&_normal[_index[i]].x);
		glTexCoord2fv(&_uv[_index[i]].x);
		if (!_color.empty())
			glColor4fv(&_color[_index[i]].red);
		glVertex3fv(&_vertex[_index[i]].x);
	}
	glEnd();
}

void GLmesh::operator+= (const GLmesh& c)
{
	unsigned      index;

	index = Vertices();
	if (!c._color.empty() && !c._normal.empty() && !c._uv.empty()) {
		for (unsigned i = 0; i < c.Vertices(); i++)
			PushVertex(c._vertex[i], c._normal[i], c._color[i], c._uv[i]);
	} else if (!c._color.empty() && !c._uv.empty()) {
		for (unsigned i = 0; i < c.Vertices(); i++)
			PushVertex(c._vertex[i], c._color[i], c._uv[i]);
  } else if (!c._uv.empty ()) {
    for (unsigned i = 0; i < c.Vertices (); i++)
      PushVertex (c._vertex[i], c._uv[i]);
  }
  for (unsigned i = 0; i < c.Triangles (); i++) {
    PushTriangle (c._index[i * 3 + 2] + index, c._index[i * 3 + 1] + index, c._index[i * 3] + index);
  }
}

void GLmesh::PushTriangle(unsigned i1, unsigned i2, unsigned i3)
{
	_index.push_back(i3);
	_index.push_back(i2);
	_index.push_back(i1);
}

void GLmesh::PushQuad(unsigned i1, unsigned i2, unsigned i3, unsigned i4)
{
	PushTriangle(i1, i2, i3);
	PushTriangle(i1, i3, i4);
}

//For convenience: This version just shoves the last 4 verts into a quad,
//in the order they were added. If there aren't 4 verts yet? Your problem.
void GLmesh::PushQuad()
{
	PushQuad(_vertex.size() - 4, _vertex.size() - 3, _vertex.size() - 2, _vertex.size() - 1);
}

void GLmesh::PushVertex(GLvector vert, GLvector normal, GLvector2 uv)
{
	_bbox.ContainPoint(vert);
	_vertex.push_back(vert);
	_normal.push_back(normal);
	_uv.push_back(uv);
}

void GLmesh::PushVertex(GLvector vert, GLvector normal, GLrgba color, GLvector2 uv)
{
	_bbox.ContainPoint(vert);
	_vertex.push_back(vert);
	_normal.push_back(normal);
	_color.push_back(color);
	_uv.push_back(uv);
}

void GLmesh::PushVertex(GLvector vert, GLrgba color, GLvector2 uv)
{
	_bbox.ContainPoint(vert);
	_vertex.push_back(vert);
	_color.push_back(color);
	_uv.push_back(uv);
}

void GLmesh::PushVertex(GLvector vert, GLvector2 uv)
{
	_bbox.ContainPoint(vert);
	_vertex.push_back(vert);
	_uv.push_back(uv);
}

void GLmesh::Clear()
{
	_bbox.Clear();
	_vertex.clear();
	_normal.clear();
	_uv.clear();
	_index.clear();
	_color.clear();
}

void GLmesh::RecalculateBoundingBox()
{
	_bbox.Clear();
	for (unsigned i = 0; i < Vertices(); i++)
		_bbox.ContainPoint(_vertex[i]);
}

void GLmesh::CalculateNormals()
{
	GLvector    edge[3];
	unsigned    i;
	float       dot;
	float       angle[3];
	unsigned    index;
	unsigned    i0, i1, i2;
	GLvector    normal;

	//Clear any existing normals
	for (i = 0; i < _normal.size(); i++)
		_normal[i] = GLvector(0.0f, 0.0f, 0.0f);
	//For each triangle...
	for (i = 0; i < Triangles(); i++) {
		index = i * 3;
		i0 = _index[index];
		i1 = _index[index + 1];
		i2 = _index[index + 2];
		// Convert the 3 edges of the polygon into vectors
		edge[0] = _vertex[i0] - _vertex[i1];
		edge[1] = _vertex[i1] - _vertex[i2];
		edge[2] = _vertex[i2] - _vertex[i0];
		// normalize the vectors
		edge[0].Normalize();
		edge[1].Normalize();
		edge[2].Normalize();
		// now get the normal from the cross product of any two of the edge vectors
		normal = GLcross(edge[2], edge[0] * -1);
		normal.Normalize();
		//calculate the 3 internal angles of this triangle.
		dot = GLdot(edge[2], edge[0]);
		angle[0] = acos(-dot);
		if (_isnan(angle[0]))
			continue;
		angle[1] = acos(-GLdot(edge[0], edge[1]));
		if (_isnan(angle[1]))
			continue;
		angle[2] = PI - (angle[0] + angle[1]);
		//Now weight each normal by the size of the angle so that the triangle
		//with the largest angle at that vertex has the most influence over the
		//direction of the normal.
		_normal[i0] += normal * angle[0];
		_normal[i1] += normal * angle[1];
		_normal[i2] += normal * angle[2];
	}
	//Re-normalize. Done.
	for (i = 0; i < _normal.size(); i++)
		_normal[i].Normalize();
}

void GLmesh::Move(GLvector offset)
{
	unsigned    i;

	for (i = 0; i < _vertex.size(); i++)
		_vertex[i] += offset;
}

void GLmesh::CalculateNormalsSeamless()
{
	GLvector          edge[3];
	unsigned          i, j;
	float             dot;
	float             angle[3];
	unsigned          index;
	unsigned          i0, i1, i2;
	GLvector          normal;
	vector<unsigned>  merge_index;
	vector<GLvector>  verts_merged;
	vector<GLvector>  normals_merged;
	int               found;

	//Clear any existing normals
	for (i = 0; i < _normal.size(); i++)
		normals_merged.push_back(GLvector(0.0f, 0.0f, 0.0f));

	// scan through the vert list, and make an alternate list where
	// verticies that share the same location are merged
	for (i = 0; i < _vertex.size(); i++) {
		found = -1;
		//see if there is another vertex in the same position in the merged list
		for (j = 0; j < merge_index.size(); j++) {
			if (_vertex[i] == _vertex[merge_index[j]]) {
				merge_index.push_back(j);
				verts_merged.push_back(_vertex[i]);
				found = j;
				break;
			}
		}
		//vertex not found, so add another
		if (found == -1) {
			merge_index.push_back(verts_merged.size());
			verts_merged.push_back(_vertex[i]);
		}
	}
	//For each triangle...
	for (i = 0; i < Triangles(); i++) {
		index = i * 3;
		i0 = merge_index[_index[index]];
		i1 = merge_index[_index[index + 1]];
		i2 = merge_index[_index[index + 2]];
		// Convert the 3 edges of the polygon into vectors
		edge[0] = verts_merged[i0] - verts_merged[i1];
		edge[1] = verts_merged[i1] - verts_merged[i2];
		edge[2] = verts_merged[i2] - verts_merged[i0];
		// normalize the vectors
		edge[0].Normalize();
		edge[1].Normalize();
		edge[2].Normalize();
		// now get the normal from the cross product of any two of the edge vectors
		normal = GLcross(edge[2], edge[0] * -1);
		normal.Normalize();
		//calculate the 3 internal angles of this triangle.
		dot = GLdot(edge[2], edge[0]);
		angle[0] = acos(-dot);
		if (_isnan(angle[0]))
			continue;
		angle[1] = acos(-GLdot(edge[0], edge[1]));
		if (_isnan(angle[1]))
			continue;
		angle[2] = PI - (angle[0] + angle[1]);
		//Now weight each normal by the size of the angle so that the triangle
		//with the largest angle at that vertex has the most influence over the
		//direction of the normal.
		normals_merged[i0] += normal * angle[0];
		normals_merged[i1] += normal * angle[1];
		normals_merged[i2] += normal * angle[2];
	}
	//Re-normalize. Done.
	for (i = 0; i < _normal.size(); i++) {
		_normal[i] = normals_merged[merge_index[i]];
		_normal[i].Normalize();
	}
}

#define M(e,x,y)                (e.elements[x][y])
#define E(x,y)                  (elements[x][y])

/*** Order type constants, constructors, extractors ***/

/* There are 24 possible conventions, designated by:    */
/*	  o EulAxI = axis used initially		    */
/*	  o EulPar = parity of axis permutation		    */
/*	  o EulRep = repetition of initial axis as last	    */
/*	  o EulFrm = frame from which axes are taken	    */
/* Axes I,J,K will be a permutation of X,Y,Z.	    */
/* Axis H will be either I or K, depending on EulRep.   */
/* Frame S takes axes from initial static frame.	    */
/* If ord = (AxI=X, Par=Even, Rep=No, Frm=S), then	    */
/* {a,b,c,ord} means Rz(c)Ry(b)Rx(a), where Rz(c)v	    */
/* rotates v around Z by c radians.			    */

#define EulFrmS	     0
#define EulFrmR	     1
#define EulFrm(ord)  ((unsigned)(ord)&1)
#define EulRepNo     0
#define EulRepYes    1
#define EulRep(ord)  (((unsigned)(ord)>>1)&1)
#define EulParEven   0
#define EulParOdd    1
#define EulPar(ord)  (((unsigned)(ord)>>2)&1)
#define EulSafe	     "\000\001\002\000"
#define EulNext	     "\001\002\000\001"
#define EulAxI(ord)  ((int)(EulSafe[(((unsigned)(ord)>>3)&3)]))
#define EulAxJ(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)==EulParOdd)]))
#define EulAxK(ord)  ((int)(EulNext[EulAxI(ord)+(EulPar(ord)!=EulParOdd)]))
#define EulAxH(ord)  ((EulRep(ord)==EulRepNo)?EulAxK(ord):EulAxI(ord))
/* EulGetOrd unpacks all useful information about order simultaneously. */
#define EulGetOrd(ord,i,j,k,h,n,s,f) {unsigned o=ord;f=o&1;o>>=1;s=o&1;o>>=1;\
    n=o&1;o>>=1;i=EulSafe[o&3];j=EulNext[i+n];k=EulNext[i+1-n];h=s?k:i;}
/* EulOrd creates an order value between 0 and 23 from 4-tuple choices. */
#define EulOrd(i,p,r,f)	   (((((((i)<<1)+(p))<<1)+(r))<<1)+(f))
/* Static axes */
#define EulOrdXYZs    EulOrd(X,EulParEven,EulRepNo,EulFrmS)
#define EulOrdXYXs    EulOrd(X,EulParEven,EulRepYes,EulFrmS)
#define EulOrdXZYs    EulOrd(X,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdXZXs    EulOrd(X,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdYZXs    EulOrd(Y,EulParEven,EulRepNo,EulFrmS)
#define EulOrdYZYs    EulOrd(Y,EulParEven,EulRepYes,EulFrmS)
#define EulOrdYXZs    EulOrd(Y,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdYXYs    EulOrd(Y,EulParOdd,EulRepYes,EulFrmS)
#define EulOrdZXYs    EulOrd(Z,EulParEven,EulRepNo,EulFrmS)
#define EulOrdZXZs    EulOrd(Z,EulParEven,EulRepYes,EulFrmS)
#define EulOrdZYXs    EulOrd(Z,EulParOdd,EulRepNo,EulFrmS)
#define EulOrdZYZs    EulOrd(Z,EulParOdd,EulRepYes,EulFrmS)
/* Rotating axes */
#define EulOrdZYXr    EulOrd(X,EulParEven,EulRepNo,EulFrmR)
#define EulOrdXYXr    EulOrd(X,EulParEven,EulRepYes,EulFrmR)
#define EulOrdYZXr    EulOrd(X,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdXZXr    EulOrd(X,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdXZYr    EulOrd(Y,EulParEven,EulRepNo,EulFrmR)
#define EulOrdYZYr    EulOrd(Y,EulParEven,EulRepYes,EulFrmR)
#define EulOrdZXYr    EulOrd(Y,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdYXYr    EulOrd(Y,EulParOdd,EulRepYes,EulFrmR)
#define EulOrdYXZr    EulOrd(Z,EulParEven,EulRepNo,EulFrmR)
#define EulOrdZXZr    EulOrd(Z,EulParEven,EulRepYes,EulFrmR)
#define EulOrdXYZr    EulOrd(Z,EulParOdd,EulRepNo,EulFrmR)
#define EulOrdZYZr    EulOrd(Z,EulParOdd,EulRepYes,EulFrmR)

#include <math.h>
#include <float.h>

static float      identity_matrix[4][4] =
{
	{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 0.0f, 1.0f },
};

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void* glMatrixCreate(void)
{
	GLmatrix*       m;
	int             x;
	int             y;

	m = new GLmatrix;
	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			m->elements[x][y] = identity_matrix[x][y];
		}
	}
	return (void*)m;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLmatrix glMatrixIdentity(void)
{
	GLmatrix        m;
	int             x;
	int             y;

	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			M(m, x, y) = identity_matrix[x][y];
		}
	}
	return m;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

void glMatrixElementsSet(GLmatrix* m, float* in)
{
	m->elements[0][0] = in[0];
	m->elements[0][1] = in[1];
	m->elements[0][2] = in[2];
	m->elements[0][3] = in[3];

	m->elements[1][0] = in[4];
	m->elements[1][1] = in[5];
	m->elements[1][2] = in[6];
	m->elements[1][3] = in[7];

	m->elements[2][0] = in[8];
	m->elements[2][1] = in[9];
	m->elements[2][2] = in[10];
	m->elements[2][3] = in[11];

	m->elements[3][0] = in[12];
	m->elements[3][1] = in[13];
	m->elements[3][2] = in[14];
	m->elements[3][3] = in[15];
}

/*---------------------------------------------------------------------------
A matrix multiplication (dot product) of two 4x4 matrices.
---------------------------------------------------------------------------*/

GLmatrix glMatrixMultiply(GLmatrix a, GLmatrix b)
{
	GLmatrix        result;

	M(result, 0, 0) = M(a, 0, 0) * M(b, 0, 0) + M(a, 1, 0) * M(b, 0, 1) + M(a, 2, 0) * M(b, 0, 2);
	M(result, 1, 0) = M(a, 0, 0) * M(b, 1, 0) + M(a, 1, 0) * M(b, 1, 1) + M(a, 2, 0) * M(b, 1, 2);
	M(result, 2, 0) = M(a, 0, 0) * M(b, 2, 0) + M(a, 1, 0) * M(b, 2, 1) + M(a, 2, 0) * M(b, 2, 2);
	M(result, 3, 0) = M(a, 0, 0) * M(b, 3, 0) + M(a, 1, 0) * M(b, 3, 1) + M(a, 2, 0) * M(b, 3, 2) + M(a, 3, 0);

	M(result, 0, 1) = M(a, 0, 1) * M(b, 0, 0) + M(a, 1, 1) * M(b, 0, 1) + M(a, 2, 1) * M(b, 0, 2);
	M(result, 1, 1) = M(a, 0, 1) * M(b, 1, 0) + M(a, 1, 1) * M(b, 1, 1) + M(a, 2, 1) * M(b, 1, 2);
	M(result, 2, 1) = M(a, 0, 1) * M(b, 2, 0) + M(a, 1, 1) * M(b, 2, 1) + M(a, 2, 1) * M(b, 2, 2);
	M(result, 3, 1) = M(a, 0, 1) * M(b, 3, 0) + M(a, 1, 1) * M(b, 3, 1) + M(a, 2, 1) * M(b, 3, 2) + M(a, 3, 1);

	M(result, 0, 2) = M(a, 0, 2) * M(b, 0, 0) + M(a, 1, 2) * M(b, 0, 1) + M(a, 2, 2) * M(b, 0, 2);
	M(result, 1, 2) = M(a, 0, 2) * M(b, 1, 0) + M(a, 1, 2) * M(b, 1, 1) + M(a, 2, 2) * M(b, 1, 2);
	M(result, 2, 2) = M(a, 0, 2) * M(b, 2, 0) + M(a, 1, 2) * M(b, 2, 1) + M(a, 2, 2) * M(b, 2, 2);
	M(result, 3, 2) = M(a, 0, 2) * M(b, 3, 0) + M(a, 1, 2) * M(b, 3, 1) + M(a, 2, 2) * M(b, 3, 2) + M(a, 3, 2);
	return result;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLvector glMatrixTransformPoint(GLmatrix m, GLvector in)
{
	GLvector              out;

	out.x = (M(m, 0, 0) * in.x + M(m, 1, 0) * in.y + M(m, 2, 0) * in.z + M(m, 3, 0));
	out.y = (M(m, 0, 1) * in.x + M(m, 1, 1) * in.y + M(m, 2, 1) * in.z + M(m, 3, 1));
	out.z = (M(m, 0, 2) * in.x + M(m, 1, 2) * in.y + M(m, 2, 2) * in.z + M(m, 3, 2));
	/*
	  out.x = (M(m,0,0) * in.x + M(m,0,1) * in.y + M(m,0,2) * in.z + M(m,0,3));
	  out.y = (M(m,1,0) * in.x + M(m,1,1) * in.y + M(m,1,2) * in.z + M(m,1,3));
	  out.z = (M(m,2,0) * in.x + M(m,2,1) * in.y + M(m,2,2) * in.z + M(m,2,3));
	  */
	//out.x = (M(m,0,0) * in.x + M(m,1,0) * in.y + M(m,2,0) * in.z + M(m,0,3));
	//out.y = (M(m,0,1) * in.x + M(m,1,1) * in.y + M(m,2,1) * in.z + M(m,1,3));
	//out.z = (M(m,0,2) * in.x + M(m,1,2) * in.y + M(m,2,2) * in.z + M(m,2,3));
	return out;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLmatrix glMatrixScale(GLmatrix m, GLvector in)
{
	M(m, 0, 3) *= in.x;
	M(m, 1, 3) *= in.y;
	M(m, 2, 3) *= in.z;
	return m;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLmatrix glMatrixTranslate(GLmatrix m, GLvector in)
{
	GLvector  old;

	old.x = M(m, 3, 0);
	old.y = M(m, 3, 1);
	old.z = M(m, 3, 2);
	M(m, 3, 0) = 0.0f;
	M(m, 3, 1) = 0.0f;
	M(m, 3, 2) = 0.0f;
	in = glMatrixTransformPoint(m, in);
	M(m, 3, 0) = old.x;
	M(m, 3, 1) = old.y;
	M(m, 3, 2) = old.z;
	M(m, 3, 0) += in.x;
	M(m, 3, 1) += in.y;
	M(m, 3, 2) += in.z;
	return m;
}

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

GLmatrix glMatrixRotate(GLmatrix m, float theta, float x, float y, float z)
{
	GLmatrix              r;
	float                 length;
	float                 s, c, t;
	GLvector              in;

	theta *= DEGREES_TO_RADIANS;
	r = glMatrixIdentity();
	length = (float)sqrt(x * x + y * y + z * z);
	if (length < 0.00001f)
		return m;
	x /= length;
	y /= length;
	z /= length;
	s = (float)sin(theta);
	c = (float)cos(theta);
	t = 1.0f - c;

	in.x = in.y = in.z = 1.0f;
	M(r, 0, 0) = t*x*x + c;
	M(r, 1, 0) = t*x*y - s*z;
	M(r, 2, 0) = t*x*z + s*y;
	M(r, 3, 0) = 0;

	M(r, 0, 1) = t*x*y + s*z;
	M(r, 1, 1) = t*y*y + c;
	M(r, 2, 1) = t*y*z - s*x;
	M(r, 3, 1) = 0;

	M(r, 0, 2) = t*x*z - s*y;
	M(r, 1, 2) = t*y*z + s*x;
	M(r, 2, 2) = t*z*z + c;
	M(r, 3, 2) = 0;

	m = glMatrixMultiply(m, r);
	return m;
}

/* Convert matrix to Euler angles (in radians). */
GLvector glMatrixToEuler(GLmatrix mat, int order)
{
	GLvector    ea;
	int         i, j, k, h, n, s, f;

	EulGetOrd(order, i, j, k, h, n, s, f);
	if (s == EulRepYes) {
		float sy = (float)sqrt(mat.elements[i][j] * mat.elements[i][j] + mat.elements[i][k] * mat.elements[i][k]);
		if (sy > 16 * FLT_EPSILON) {
			ea.x = (float)atan2(mat.elements[i][j], mat.elements[i][k]);
			ea.y = (float)atan2(sy, mat.elements[i][i]);
			ea.z = (float)atan2(mat.elements[j][i], -mat.elements[k][i]);
		}
		else {
			ea.x = (float)atan2(-mat.elements[j][k], mat.elements[j][j]);
			ea.y = (float)atan2(sy, mat.elements[i][i]);
			ea.z = 0;
		}
	}
	else {
		float cy = (float)sqrt(mat.elements[i][i] * mat.elements[i][i] + mat.elements[j][i] * mat.elements[j][i]);
		if (cy > 16 * FLT_EPSILON) {
			ea.x = (float)atan2(mat.elements[k][j], mat.elements[k][k]);
			ea.y = (float)atan2(-mat.elements[k][i], cy);
			ea.z = (float)atan2(mat.elements[j][i], mat.elements[i][i]);
		}
		else {
			ea.x = (float)atan2(-mat.elements[j][k], mat.elements[j][j]);
			ea.y = (float)atan2(-mat.elements[k][i], cy);
			ea.z = 0;
		}
	}
	if (n == EulParOdd) {
		ea.x = -ea.x;
		ea.y = -ea.y;
		ea.z = -ea.z;
	}
	if (f == EulFrmR) {
		float t = ea.x;
		ea.x = ea.z;
		ea.z = t;
	}
	//ea.w = order;
	return (ea);
}

void GLmatrix::Identity()
{
	int             x;
	int             y;

	for (x = 0; x < 4; x++) {
		for (y = 0; y < 4; y++) {
			E(x, y) = identity_matrix[x][y];
		}
	}
}

void GLmatrix::Rotate(float theta, float x, float y, float z)
{
	GLmatrix              r;
	float                 length;
	float                 s, c, t;
	GLvector              in;

	r = *this;
	theta *= DEGREES_TO_RADIANS;
	//Identity ();
	length = (float)sqrt(x * x + y * y + z * z);
	if (length < 0.00001f)
		return;
	x /= length;
	y /= length;
	z /= length;
	s = (float)sin(theta);
	c = (float)cos(theta);
	t = 1.0f - c;

	in.x = in.y = in.z = 1.0f;
	E(0, 0) = t*x*x + c;
	E(1, 0) = t*x*y - s*z;
	E(2, 0) = t*x*z + s*y;
	E(3, 0) = 0;

	E(0, 1) = t*x*y + s*z;
	E(1, 1) = t*y*y + c;
	E(2, 1) = t*y*z - s*x;
	E(3, 1) = 0;

	E(0, 2) = t*x*z - s*y;
	E(1, 2) = t*y*z + s*x;
	E(2, 2) = t*z*z + c;
	E(3, 2) = 0;

	Multiply(r);
}

/*---------------------------------------------------------------------------
A matrix multiplication (dot product) of two 4x4 matrices.
---------------------------------------------------------------------------*/

void GLmatrix::Multiply(GLmatrix a)
{
	GLmatrix        b;

	b = *this;
	E(0, 0) = M(a, 0, 0) * M(b, 0, 0) + M(a, 1, 0) * M(b, 0, 1) + M(a, 2, 0) * M(b, 0, 2);
	E(1, 0) = M(a, 0, 0) * M(b, 1, 0) + M(a, 1, 0) * M(b, 1, 1) + M(a, 2, 0) * M(b, 1, 2);
	E(2, 0) = M(a, 0, 0) * M(b, 2, 0) + M(a, 1, 0) * M(b, 2, 1) + M(a, 2, 0) * M(b, 2, 2);
	E(3, 0) = M(a, 0, 0) * M(b, 3, 0) + M(a, 1, 0) * M(b, 3, 1) + M(a, 2, 0) * M(b, 3, 2) + M(a, 3, 0);

	E(0, 1) = M(a, 0, 1) * M(b, 0, 0) + M(a, 1, 1) * M(b, 0, 1) + M(a, 2, 1) * M(b, 0, 2);
	E(1, 1) = M(a, 0, 1) * M(b, 1, 0) + M(a, 1, 1) * M(b, 1, 1) + M(a, 2, 1) * M(b, 1, 2);
	E(2, 1) = M(a, 0, 1) * M(b, 2, 0) + M(a, 1, 1) * M(b, 2, 1) + M(a, 2, 1) * M(b, 2, 2);
	E(3, 1) = M(a, 0, 1) * M(b, 3, 0) + M(a, 1, 1) * M(b, 3, 1) + M(a, 2, 1) * M(b, 3, 2) + M(a, 3, 1);

	E(0, 2) = M(a, 0, 2) * M(b, 0, 0) + M(a, 1, 2) * M(b, 0, 1) + M(a, 2, 2) * M(b, 0, 2);
	E(1, 2) = M(a, 0, 2) * M(b, 1, 0) + M(a, 1, 2) * M(b, 1, 1) + M(a, 2, 2) * M(b, 1, 2);
	E(2, 2) = M(a, 0, 2) * M(b, 2, 0) + M(a, 1, 2) * M(b, 2, 1) + M(a, 2, 2) * M(b, 2, 2);
	E(3, 2) = M(a, 0, 2) * M(b, 3, 0) + M(a, 1, 2) * M(b, 3, 1) + M(a, 2, 2) * M(b, 3, 2) + M(a, 3, 2);
}

GLvector GLmatrix::TransformPoint(GLvector in)
{
	GLvector              out;

	out.x = (E(0, 0) * in.x + E(1, 0) * in.y + E(2, 0) * in.z + E(3, 0));
	out.y = (E(0, 1) * in.x + E(1, 1) * in.y + E(2, 1) * in.z + E(3, 1));
	out.z = (E(0, 2) * in.x + E(1, 2) * in.y + E(2, 2) * in.z + E(3, 2));
	return out;
}

/*---------------------------------------------------------------------------
General utility functions.
---------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
return a scalar of 0.0 to 1.0, based an the given values position within a range
-----------------------------------------------------------------------------*/

float SmoothStep(float start, float end, float position)
{
	if (end == start)
		return 0.0f;
	position -= start;
	position /= (end - start);
	return clamp(position, 0.0f, 1.0f);
}

/*-----------------------------------------------------------------------------
interpolate between two values
-----------------------------------------------------------------------------*/

float Lerp(float start, float end, float delta)
{
	return start * (1.0f - delta) + end * delta;
}

GLvector2 Lerp(GLvector2 start, GLvector2 end, float delta)
{
	GLvector2 result;

	result.x = Lerp(start.x, end.x, delta);
	result.y = Lerp(start.y, end.y, delta);
	return result;
}

GLvector Lerp(GLvector start, GLvector end, float delta)
{
	GLvector result;

	result.x = Lerp(start.x, end.x, delta);
	result.y = Lerp(start.y, end.y, delta);
	result.z = Lerp(start.z, end.z, delta);
	return result;
}

GLrgba Lerp(GLrgba start, GLrgba end, float delta)
{
	GLrgba result;

	result.red = Lerp(start.red, end.red, delta);
	result.green = Lerp(start.green, end.green, delta);
	result.blue = Lerp(start.blue, end.blue, delta);
	result.alpha = Lerp(start.alpha, end.alpha, delta);
	return result;
}

/*-----------------------------------------------------------------------------
  This will take linear input values from 0.0 to 1.0 and convert them to
  values along a curve.  This could also be acomplished with sin (), but this
  way avoids converting to radians and back.
  -----------------------------------------------------------------------------*/

float Curve(float val)
{
	float   sign;

	val = (val - 0.5f) * 2.0f;
	if (val < 0.0f)
		sign = -1.0f;
	else
		sign = 1.0f;
	if (val < 0.0f)
		val = -val;
	val = 1.0f - val;
	val *= val;
	val = 1.0f - val;
	val *= sign;
	val = (val + 1.0f) / 2.0f;
	return val;
}

/*-----------------------------------------------------------------------------
difference between two angles
-----------------------------------------------------------------------------*/

float AngleDifference(float a1, float a2)

{
	float         result;

	result = (float)fmod(a1 - a2, 360.0f);
	if (result > 180.0)
		return result - 360.0F;
	if (result < -180.0)
		return result + 360.0F;
	return result;
}

int Wrap(int v, int size)
{
	v %= size;
	if (v < 0)
		v += size;
	return v;
}

float Wrap(float v, float size)
{
	v = fmod(v, size);
	if (v < 0)
		v += size;
	return v;
}

static GLcoord2 wrap(GLcoord2 v, int size)
{
	return GLcoord2(Wrap(v.x, size), Wrap(v.y, size));
}

/*-----------------------------------------------------------------------------
Get an angle between two given points on a grid
-----------------------------------------------------------------------------*/

float Angle2D(float x1, float y1, float x2, float y2)
{
	float   x_delta;
	float   z_delta;
	float   angle;

	z_delta = (y1 - y2);
	x_delta = (x1 - x2);
	if (x_delta == 0) {
		if (z_delta > 0)
			return 0.0f;
		else
			return 180.0f;
	}
	if (fabs(x_delta) < fabs(z_delta)) {
		angle = 90 - (float)atan(z_delta / x_delta) * RADIANS_TO_DEGREES;
		if (x_delta < 0)
			angle -= 180.0f;
	}
	else {
		angle = (float)atan(x_delta / z_delta) * RADIANS_TO_DEGREES;
		if (z_delta < 0.0f)
			angle += 180.0f;
	}
	if (angle < 0.0f)
		angle += 360.0f;
	return angle;
}
/*
float GLmathAngleDelta (float a1, float a2)
{
float         result;

result = (float)fmod (a1 - a2, 360.0f);
if (result > 180.0)
return result - 360.0F;
if (result < -180.0)
return result + 360.0F;
return result;
}
*/

int AngleLimit(int angle)
{
	angle %= 360;
	if (angle < 0)
		angle = 360 + angle;
	angle = clamp(angle, 0, 359);
	return angle;
}

float AngleLimit(float angle)
{
	angle = fmod(angle, 360.0f);
	if (angle < 0)
		angle = 360 + angle;
	angle = clamp(angle, 0.0f, 359.0f);
	return angle;
}

int Angle(int angle)
{
	angle %= 360;
	if (angle < 0)
		angle = 360 + angle;
	angle = clamp(angle, 0, 359);
	return angle;
}

float Angle(float angle)
{
	angle = fmod(angle, 360.0f);
	if (angle < 0)
		angle = 360 + angle;
	angle = clamp(angle, 0.0f, 359.9f);
	return angle;
}

int PowerOf2(int a)
{
	int rval;

	rval = 1;
	while (rval < a)
		rval <<= 1;
	return rval;
}

/*-----------------------------------------------------------------------------

  This forms a theoretical quad with the four elevation values.  Given the
  offset from the upper-left corner, it determines what the elevation
  should be at that point in the center area.  left" determines if the
  quad is cut from y2 to y1, or from y0 to y3.

  y0-----y1
  |     |
  |     |
  y2-----y3

  -----------------------------------------------------------------------------*/

float Lerp2D(float y0, float y1, float y2, float y3, GLvector2 offset, bool left)
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

GLcoord3 GLcoordFromVector(struct GLvector c)
{
	GLcoord3  result;

	result.x = (int)c.x;
	result.y = (int)c.y;
	result.z = (int)c.z;
	return result;
}

GLcoord2  GLcoordFromVector(struct GLvector2 c)
{
	GLcoord2  result;

	result.x = (int)c.x;
	result.y = (int)c.y;
	return result;
}