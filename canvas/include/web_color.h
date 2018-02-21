#ifndef _WEB_COLOR_H_
#define _WEB_COLOR_H_

#include "web_string.h"

using namespace std;

namespace canvas
{
	struct def_color
	{
		const char*	name;
		const char*	rgb;
	};

	extern def_color g_def_colors[];

    class document_container;

	struct web_color
	{
		unsigned char b;
		unsigned char g;
		unsigned char r;
		unsigned char a;

		web_color(unsigned char r, unsigned char g, unsigned char b, unsigned char a = 0xFF)
		{
			this->b = b;
			this->g = g;
			this->r = r;
			this->a = a;
		}

		web_color() : b(0), g(0), r(0), a(0xFF) {}

		web_color(const web_color& val)
		{
			this->b = val.b;
			this->g = val.g;
			this->r = val.r;
			this->a = val.a;
		}

		web_color& operator=(const web_color& val)
		{
			this->b = val.b;
		  this->g = val.g;
			this->r = val.r;
			this->a = val.a;
			return *this;
		}
        static web_color            from_string(const char* str);
		    static string    resolve_name(const char* name);
        static bool                 is_color(const char* str);
	};
}

#endif