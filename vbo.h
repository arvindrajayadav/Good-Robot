#ifndef VBO_H
#define VBO_H

class VBO
{
	unsigned  _id_vertex;
	unsigned  _id_index;
	unsigned  _index_count;
	intptr_t  _size_vertex;
	intptr_t  _size_uv;
	intptr_t  _size_normal;
	intptr_t  _size_buffer;
	int       _polygon;
	intptr_t  _size_color;
	bool      _ready;
	bool      _use_color;
	bool      _use_normal;

public:
	VBO();
	~VBO();
	void      Create(int polygon, int index_count, int vert_count, unsigned* index_list, GLvector* vert_list, GLvector* normal_list, GLrgba* color_list, GLvector2* uv_list);
	void      Create(GLmesh* m);
	void      Clear();
	void      Render();
	bool      Ready();
	bool      Valid();
};

#endif // VBO_H
