/*-----------------------------------------------------------------------------

  Ini.cpp

  2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

#include <stdio.h>
#include "file.h"
#include "ini.h"
#include "main.h"

#define FORMAT_VECTOR       "%f %f %f"
#define FORMAT_VECTOR2      "%f %f"
#define MAX_RESULT          2048
#define FORMAT_FLOAT        "%1.2f"
#define DEFAULT_SECTION     "Settings"

struct IniFile
{
	char                  filename[MAX_RESULT];
};

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

int iniFile::SectionKey(int sect, string key)
{
	for (unsigned i = 0; i < _section[sect].keyval.size(); i++) {
		if (key == _section[sect].keyval[i].key)
			return i;//_section[sect].keyval[i].line_number;
	}
	return -1;
}

void iniFile::StringSet(string section, string key, string value)
{
	int     sect;
	int     line;
	int     key_num;
	string  out;

	key = StringToLower(key);
	sect = SectionGet(section);
	key_num = SectionKey(sect, key);
	if (key_num >= 0) { //This key/value pair exists in the file and must be replaced.
		line = _section[sect].keyval[key_num].line_number;
		for (unsigned i = 0; i < _file_lines.size(); i++) {
			if ((int)i == line) {
				out += _section[sect].keyval[key_num].original_key;
				out += "=";
				out += value;
			}
			else {
				out += _file_lines[i];
			}
			out += "\n";
		}
	}
	else { //Key/value pair is new to file and should be inserted.
		line = _section[sect].line_number + 1;
		for (unsigned i = 0; i <= _file_lines.size(); i++) {
			if ((int)i == line) {
				out += key;
				out += "=";
				out += value;
				out += "\n";
			}
			if (i < _file_lines.size()) {
				out += _file_lines[i];
				out += "\n";
			}
		}
	}
	FileSave(_filename, out.c_str(), out.size());
	ReloadFile();
}

void iniFile::IntSet(string section, string key, int value)
{
	char    temp[16];

	sprintf(temp, "%d", value);
	StringSet(section, key, string(temp));
}

void iniFile::BoolSet(string section, string key, bool value)
{
	if (value)
		IntSet(section, key, 1);
	else
		IntSet(section, key, 0);
}

GLvector2 iniFile::Vector2Get(string section, string entry)
{
	string      result = StringGet(section, entry);
	GLvector2   v;

	v.x = v.y = 0.0f;
	sscanf(result.c_str(), FORMAT_VECTOR2, &v.x, &v.y);
	return v;
}

GLvector iniFile::VectorGet(string section, string entry)
{
	string      result = StringGet(section, entry);
	GLvector    v;

	v.x = v.y = v.z = 0.0f;
	sscanf(result.c_str(), FORMAT_VECTOR, &v.x, &v.y, &v.z);
	return v;
}

string iniFile::StringGet(string section, string entry)
{
	int   sect;

	sect = SectionGet(section);
	for (unsigned i = 0; i < _section[sect].keyval.size(); i++) {
		if (!_stricmp (_section[sect].keyval[i].key.c_str (), entry.c_str ()))
			return _section[sect].keyval[i].value;
	}
	return "";
}

unsigned iniFile::SectionKeys(string section)
{
	int   sect;

	sect = SectionGet(section);
	return _section[sect].keyval.size();
}

string iniFile::SectionKey(string section, unsigned index)
{
	int   sect;

	sect = SectionGet(section);
	return _section[sect].keyval[index].key;
}

string iniFile::SectionValue(string section, unsigned index)
{
	int   sect;

	sect = SectionGet(section);
	return _section[sect].keyval[index].value;
}

float iniFile::FloatGet(string section, string entry)
{
	string  result = StringGet(section, entry);
	return (float)atof(result.c_str());
}

int iniFile::IntGet(string section, string entry)
{
	string  result = StringGet(section, entry);
	return atoi(result.c_str());
}

long iniFile::LongGet (string section, string entry)
{
  string  result = StringGet (section, entry);
  return atol (result.c_str ());
}

bool iniFile::BoolGet(string section, string entry)
{
	string  result = StringGet(section, entry);
	return atoi(result.c_str()) != 0;
}

int iniFile::SectionGet(string section_in, int line_number)
{
  //The most common case is to query for many properties in a row from the same section.
  //Rather than walk the list every time, we store the previously used one 
  //and check that first.
  if (_last_section_lookup < _section.size ()) {
    if (!stricmp (_section[_last_section_lookup].name.c_str (), section_in.c_str ()))
      return _last_section_lookup;
  }
  for (unsigned i = 0; i < _section.size (); i++) {
    if (!stricmp (_section[i].name.c_str (), section_in.c_str ())) {
      _last_section_lookup = i;
      return i;
    }
  }

	iniSection  new_section;

  new_section.name = StringToLower (section_in);
	new_section.original_name = section_in;

	if (line_number == -1) {
    string head = string ("\n[") + new_section.name + string ("]");
		_file_lines.push_back(head);
		_file_lines.push_back("");
		new_section.line_number = _file_lines.size() - 1;
	}	else {
		new_section.line_number = line_number;
	}
	_section.push_back(new_section);
	return _section.size() - 1;
}

void iniFile::ReloadFile()
{
	string          contents;
	int             equals;
	int             end_bracket;
	int             open_bracket;
	int             current_section;
	iniSection      default_section;

	_section.clear();
  _last_section_lookup = 0;
	_original = FileContents(_filename);
	_file_lines = StringSplit(_original, "\r\n");
	//A default section to catch any entries not under a [Heading]
	default_section.line_number = 0;
	_section.push_back(default_section);
	current_section = 0;
	for (unsigned line = 0; line < _file_lines.size(); line++) {
		contents = _file_lines[line];
		open_bracket = contents.find("[");
		end_bracket = contents.find("]");
		equals = contents.find("=");
		if (open_bracket != string::npos && end_bracket != string::npos && open_bracket < end_bracket) {
			string  section_name = contents;

			section_name = section_name.substr(open_bracket + 1);
			end_bracket = section_name.find("]");
			section_name.erase(end_bracket);
			StringTrim(section_name);
			current_section = SectionGet(section_name, line);
		}	else if (equals != string::npos) {
			iniValue    new_value;

			new_value.original_key = contents.substr(0, equals);
			StringTrim(new_value.original_key);
			new_value.key = StringToLower(new_value.original_key);
			new_value.value = contents.substr(equals + 1);
			StringTrim(new_value.value);
			new_value.line_number = line;
			_section[current_section].keyval.push_back(new_value);
		}
	}
}

void iniFile::Open(string filename)
{
	_filename = filename;
	ReloadFile();
}