#ifndef INI_H
#define INI_H

struct iniValue
{
	int                 line_number;
	string              key;
	string              original_key;
	string              value;
};

struct iniSection
{
	int                 line_number;
	string              name;
	string              original_name;
	vector<iniValue>    keyval;
};

class iniFile
{
	string              _filename;
	string              _original;
	vector<iniSection>  _section;
	vector<string>      _file_lines;
  int                 _last_section_lookup;

	int                 SectionGet(string section, int line_number = -1);
	int                 SectionKey(int sect, string key);
	void                ReloadFile();
public:
	string              Filename() { return _filename; }
	void                Open(string filename);
	string              Contents() { return _original; }
	unsigned            SectionCount() { return _section.size(); }
	string              SectionName(unsigned index) { return _section[index].name; }
	unsigned            SectionKeys(string section);
	string              SectionKey(string section, unsigned index);
	string              SectionValue(string section, unsigned index);

	string              StringGet(string section, string entry);
	float               FloatGet(string section, string entry);
	int                 IntGet(string section, string entry);
  long                LongGet (string section, string entry);
	bool                BoolGet(string section, string entry);
	GLvector2           Vector2Get(string section, string entry);
	GLvector            VectorGet(string section, string entry);

	void                StringSet(string section, string entry, string value);
	void                IntSet(string section, string entry, int value);
	void                BoolSet(string section, string entry, bool value);
};

#endif // INI_H