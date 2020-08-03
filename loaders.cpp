#include "master.h"
#include "loaders.h"

namespace pyrodactyl
{
	bool NodeValid(rapidxml::xml_node<char> *node, const bool &echo)
	{
		if (node == NULL)
		{
			/*if (echo)
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", "node not found", NULL);*/

			return false;
		}

		return true;
	}
    
#if defined(__APPLE__)
    bool LoadNum(float &val, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo)
    {
        if (node->first_attribute(name.c_str()) != NULL)
        {
            val = atof(node->first_attribute(name.c_str())->value());
            return true;
        }
        
        return false;
    }
#endif
    
	bool NodeValid(const std::string &name, rapidxml::xml_node<char> *parent_node, const bool &echo)
	{
		if (parent_node == NULL)
		{
			/*if (echo)
			{
			std::string error_msg = "parent node of " + name + " not found \n";
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
			}*/
			return false;
		}
		else if (parent_node->first_node(name.c_str()) == NULL)
		{
			/*if (echo)
			{
			std::string error_msg = "child node " + name + " of parent node " + parent_node->name() + " not found \n";
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
			}*/
			return false;
		}

		return true;
	}

	bool LoadStr(std::string &val, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo)
	{
		if (node->first_attribute(name.c_str()) != NULL)
			val = node->first_attribute(name.c_str())->value();
		else
		{
			/*if (echo)
			{
			std::string error_msg = "string " + name + " not found in " + node->name() + "\n";
			SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "XML error", error_msg.c_str(), NULL);
			}*/
			return false;
		}

		return true;
	}

	bool LoadRect(SDL_Rect &rect, rapidxml::xml_node<char> *node, const bool &echo,
		const std::string &x_name, const std::string &y_name, const std::string &w_name, const std::string &h_name)
	{
		if (LoadNum(rect.x, x_name, node, echo) && LoadNum(rect.y, y_name, node, echo)
			&& LoadNum(rect.w, w_name, node, echo) && LoadNum(rect.h, h_name, node, echo))
			return true;
		return false;
	}

	bool LoadColor(GLrgba &col, rapidxml::xml_node<char> *node, const bool &echo,
		const std::string &r_name, const std::string &g_name, const std::string &b_name, const std::string &a_name)
	{
		int r = 0, g = 0, b = 0, a = 255;

		bool result = LoadNum(r, r_name, node, echo);
		result = result && LoadNum(g, g_name, node, echo);
		result = result && LoadNum(b, b_name, node, echo);
		result = result && LoadNum(a, a_name, node, echo);

		col.red = r / 255.0f;
		col.green = g / 255.0f;
		col.blue = b / 255.0f;
		col.alpha = a / 255.0f;

		return result;
	}

	bool LoadColor(int &col, rapidxml::xml_node<char> *node, const bool &echo)
	{
		return LoadNum(col, "color", node, echo);
	}

	bool LoadBool(bool &var, const std::string &name, rapidxml::xml_node<char> *node, const bool &echo)
	{
		std::string str;
		if (LoadStr(str, name, node, echo))
		{
			if (str == "true")
				var = true;
			else
				var = false;

			return true;
		}

		return false;
	}

	void SaveBool(const bool &var, const char *name, rapidxml::xml_document<> &doc, rapidxml::xml_node<char> *root)
	{
		if (var)
			root->append_attribute(doc.allocate_attribute(name, "true"));
		else
			root->append_attribute(doc.allocate_attribute(name, "false"));
	}

	bool LoadAlign(Align &align, rapidxml::xml_node<char> *node, const bool &echo, const std::string &name)
	{
		int num = 0;
		if (LoadNum(num, name, node, echo))
		{
			align = static_cast<Align>(num);
			return true;
		}

		return false;
	}

	unsigned int Version(const std::string &filename)
	{
		unsigned int version = 0;

		XMLDoc doc(filename);
		if (doc.ready())
		{
			rapidxml::xml_node<char> *node = doc.Doc()->first_node();
			if (NodeValid(node))
				LoadNum(version, "version", node);
		}

		return version;
	}
}