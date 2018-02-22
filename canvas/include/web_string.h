#ifndef _WEB_STRING_H_
#define _WEB_STRING_H_

#include "web_string.h"
#include <string>
#include <vector>
#include <string.h>

#if _WIN32
  #define snprintf _snprintf
  #define vsnprintf _vsnprintf
  #define strcasecmp _stricmp
  #define strncasecmp _strnicmp
#else
  #include <strings.h>
#endif

using namespace std;

namespace canvas
{
  string::size_type find_close_bracket(const string &s, string::size_type off, char open_b, char close_b);
  void split_string(const string &str, vector<string> &tokens, const string &delims, const string &delims_preserve = string(""), const string &quote = string("\""));
  int value_index(const string &val, const string &strings, int defValue = -1, char delim = ';');
	bool value_in_list(const string &val, const string &strings, char delim = ';');
}

#endif