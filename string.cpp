/*-----------------------------------------------------------------------------

  String.cpp

  Just come code to cover the egregious deficiencies of std::string.

  2013 Shamus Young

  -----------------------------------------------------------------------------*/

#include "master.h"

/*-----------------------------------------------------------------------------

-----------------------------------------------------------------------------*/

vector<string> StringSplit(string source, const char* delimit)
{
	char*           buffer;
	char*           parse;
	vector<string>  result;

	buffer = (char*)malloc(source.size() + 1);
	strcpy(buffer, source.c_str());
	parse = strtok(buffer, delimit);
	while (parse) {
		result.push_back(parse);
		parse = strtok(NULL, delimit);
	}
	free(buffer);
	return result;
}

string StringToLower(string source)
{
	string result;

	result = source;
	std::transform(result.begin(), result.end(), result.begin(), ::tolower);
	return result;
}

string StringNumberFormat(int number)
{
	ostringstream   s1;
	ostringstream   s2;
	string          result;
	unsigned        len;
	int             comma;
	bool            negative;

	negative = false;
	if (number < 0) {
		negative = true;
		number = abs(number);
	}
	s1 << number;
	len = s1.str().length();
	comma = 0;
	for (unsigned i = 0; i < len; i++) {
		if (comma == 3) {
			comma = 0;
			s2 << ",";
		}
		s2 << s1.str()[len - 1 - i];
		comma++;
	}
	result = s2.str();
	result = string(result.rbegin(), result.rend());
	if (negative)
		result = string("-") + result;
	return result;
}

string StringSprintf(const char* message, ...)
{
	static char    msg_text[1024];
	va_list        marker;

	va_start(marker, message);
	vsprintf(msg_text, message, marker);
	va_end(marker);
	return string(msg_text);
}

float StringToFloat (string val)
{
  return (float)atof (val.c_str ());
}

int StringToInt (string val)
{
  return atoi (val.c_str ());
}

GLvector2 StringToVector2 (string val)
{
  vector<string>  fields;
  GLvector2       result;

  result = GLvector2 ();
  fields = StringSplit (val, " ,");
  if (fields.size () > 0)
    result.x = StringToFloat (fields[0]);
  if (fields.size () > 1)
    result.y = StringToFloat (fields[1]);
  return result;
}