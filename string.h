#ifndef STRING_H
#define STRING_H

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

// trim from start
static inline std::string StringTrimL(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
	return s;
}

// trim from end
static inline std::string StringTrimR(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
	return s;
}

// trim from both ends
static inline std::string StringTrim(std::string& s) {
    StringTrimR(s);
    StringTrimL(s);
	return s;
}

static inline int   StringComparei (string& s1, char *c_s2)
{
  return stricmp (s1.c_str (), c_s2);
}

float           StringToFloat (string val);
int             StringToInt (string val);
string          StringToLower (string source);
string          StringNumberFormat (int number);
vector<string>  StringSplit (string source, const char* delimit);
string          StringSprintf(const char* message, ...);
GLvector2       StringToVector2 (string val);

#endif // STRING_H
