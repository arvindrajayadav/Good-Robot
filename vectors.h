#pragma once

#include "common_header.h"
#include "loaders.h"
#include "stringpool.h"

namespace pyrodactyl
{
	//------------------------------------------------------------------------
	// Purpose: A simple 2D vector class
	//------------------------------------------------------------------------
	template <typename T>
	class Vector2D
	{
	public:
		T x, y;

		Vector2D(T X = 0, T Y = 0){ Set(X, Y); }
		void Set(T X = 0, T Y = 0){ x = X; y = Y; }

		bool Load(rapidxml::xml_node<char> *node, const bool &echo = true, const char* x_name = "x", const char* y_name = "y")
		{
			return pyrodactyl::LoadNum(x, x_name, node, echo) && pyrodactyl::LoadNum(y, y_name, node, echo);
		}

		void SaveState(rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root, const char* name, const char* x_name = "x", const char* y_name = "y")
		{
			rapidxml::xml_node<char> *child = doc.allocate_node(rapidxml::node_element, name);
			child->append_attribute(doc.allocate_attribute(x_name, pyrodactyl::gStrPool.Get((int)x)));
			child->append_attribute(doc.allocate_attribute(y_name, pyrodactyl::gStrPool.Get((int)y)));
			root->append_node(child);
		}

		double Magnitude() { return sqrt(x * x + y * y); }
		double MagSqr(){ return (x * x + y * y); }
		void Normalize() { double m = Magnitude(); x = x / m; y = y / m; }

		float DotProduct(const Vector2D<T> &v) const { return x * v.x + y * v.y; }

		//operators
		void operator+= (const Vector2D &v) { x += v.x; y += v.y; }
		void operator-= (const Vector2D &v) { x -= v.x; y -= v.y; }
		bool operator== (const Vector2D &v) { return x == v.x && y == v.y; }
		bool operator!= (const Vector2D &v) { return x != v.x || y != v.y; }
		friend Vector2D operator- (const Vector2D &v1, const Vector2D &v2)
		{
			return Vector2D(v1.x - v2.x, v1.y - v2.y);
		}
	};

	typedef Vector2D<int> Vector2i;
	typedef Vector2D<float> Vector2f;
}