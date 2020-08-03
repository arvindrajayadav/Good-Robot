/*-----------------------------------------------------------------------------

  VBO.cpp

  -------------------------------------------------------------------------------

  This class manages vertex buffer objects.  Take a list of verticies and
  indexes, and store them in GPU memory for fast rendering.

  -----------------------------------------------------------------------------*/

#include "master.h"
#include "vbo.h"

// VBO Extension Definitions, From glext.h
#define GL_ARRAY_BUFFER_ARB 0x8892
#define GL_STATIC_DRAW_ARB 0x88E4

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

VBO::VBO()
{
	_id_vertex = _id_index = _size_vertex = _size_uv = _size_normal = _size_buffer = _index_count = 0;
	_ready = false;
	_id_vertex = 0;
	_id_index = 0;
	_use_color = false;
	_use_normal = 0;
	_size_color = 0;
	_polygon = 0;
}

VBO::~VBO()
{
	if (_id_index)
		glDeleteBuffersARB(1, &_id_index);
	if (_id_vertex)
		glDeleteBuffersARB(1, &_id_vertex);
}

void VBO::Clear()
{
	if (_id_vertex)
		glDeleteBuffersARB(1, &_id_vertex);
	if (_id_index)
		glDeleteBuffersARB(1, &_id_index);
	_id_vertex = 0;
	_id_index = 0;
	_use_color = false;
	_size_color = 0;
	_polygon = 0;
	_ready = false;
}

void VBO::Create(int polygon, int index_count, int vert_count, unsigned* index_list, GLvector* vert_list, GLvector* normal_list, GLrgba* color_list, GLvector2* uv_list)
{
	char*     buffer;

	if (_id_vertex)
		glDeleteBuffersARB(1, &_id_vertex);
	if (_id_index)
		glDeleteBuffersARB(1, &_id_index);
	_id_vertex = 0;
	_id_index = 0;
	if (!index_count || !vert_count)
		return;
	_polygon = polygon;
	_use_color = color_list != NULL;
	_use_normal = normal_list != NULL;
	_size_vertex = sizeof(GLvector) * vert_count;
	_size_uv = sizeof(GLvector2) * vert_count;
	_size_buffer = _size_vertex + _size_uv;
	if (_use_normal) {
		_size_normal = sizeof(GLvector) * vert_count;
		_size_buffer += _size_normal;
	}
	else
		_size_normal = 0;
	if (_use_color) {
		_size_color = sizeof(GLrgba) * vert_count;
		_size_buffer += _size_color;
	}
	else
		_size_color = 0;
	//Allocate the array and pack the bytes into it.
	buffer = new char[_size_buffer];
	memcpy(buffer, vert_list, _size_vertex);
	if (_use_color)
		memcpy(buffer + _size_vertex, normal_list, _size_normal);
	if (_use_color)
		memcpy(buffer + _size_vertex + _size_normal, color_list, _size_color);
	memcpy(buffer + _size_vertex + _size_normal + _size_color, uv_list, _size_uv);
	//Create and load the buffer
	glGenBuffersARB(1, &_id_vertex);
	glBindBufferARB(GL_ARRAY_BUFFER, _id_vertex);			// Bind The Buffer
	glBufferDataARB(GL_ARRAY_BUFFER, _size_buffer, buffer, GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ARRAY_BUFFER, 0);			// Unbind The Buffer
	//Create and load the indicies
	glGenBuffersARB(1, &_id_index);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, _id_index);
	glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof(unsigned), index_list, GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0); //Unbind
	_index_count = index_count;
	delete[] buffer;
	_ready = true;
}

void VBO::Create(GLmesh* m)
{
	GLvector*   normal_list;
	GLrgba*     color_list;

	if (_id_vertex)
		glDeleteBuffersARB(1, &_id_vertex);
	if (_id_index)
		glDeleteBuffersARB(1, &_id_index);
	_id_vertex = 0;
	_id_index = 0;

	if (m->_index.size() == 0)
		return;
	normal_list = NULL;
	color_list = NULL;
	if (!m->_color.empty())
		color_list = &m->_color[0];
	if (!m->_normal.empty())
		normal_list = &m->_normal[0];
	Create(GL_TRIANGLES, m->_index.size(), m->Vertices(), &m->_index[0], &m->_vertex[0], normal_list, color_list, &m->_uv[0]);
}

void VBO::Render()
{
	if (!_ready)
		return;
	if (!_id_index)
		return;
	if (!_id_vertex)
		return;
	/*
	if (!glIsBufferARB (_id_vertex)) {
	_ready = false;
	return;
	}
	*/
	// bind VBOs for vertex array and index array
	glBindBufferARB(GL_ARRAY_BUFFER, _id_vertex);
	glEnableClientState(GL_VERTEX_ARRAY);
	if (_use_normal)
		glEnableClientState(GL_NORMAL_ARRAY);
	else
		glDisableClientState(GL_NORMAL_ARRAY);
	if (_use_color)
		glEnableClientState(GL_COLOR_ARRAY);
	else
		glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	if (_use_normal)
		glNormalPointer(GL_FLOAT, 0, (void*)(_size_vertex));
	if (_use_color)
		glColorPointer(4, GL_FLOAT, 0, (void*)(_size_vertex + _size_normal));
	glTexCoordPointer(2, GL_FLOAT, 0, (void*)(_size_vertex + _size_normal + _size_color));
	//Draw it
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, _id_index); // for indices
	glEnableClientState(GL_VERTEX_ARRAY);             // activate vertex coords array
	glDrawElements(_polygon, _index_count, GL_UNSIGNED_INT, 0);
	glDisableClientState(GL_VERTEX_ARRAY);            // deactivate vertex array
	// bind with 0, so, switch back to normal pointer operation
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, 0);
	glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER, 0);
}

bool VBO::Ready()
{
	if (!_ready)
		return false;
	if (!_id_index)
		return false;
	if (!_id_vertex)
		return false;
	return true;
}

bool VBO::Valid()
{
	if (!_ready)
		return false;
	if (!_id_index)
		return false;
	if (!_id_vertex)
		return false;
	if (!glIsBufferARB(_id_vertex))
		return false;
	return true;
}