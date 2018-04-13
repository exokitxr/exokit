#include <web_font.h>

const string font_style_strings("normal;italic");
const string font_weight_strings("normal;bold;bolder;lighter100;200;300;400;500;600;700");
const string font_variant_strings("normal;small-caps");

canvas::FontDeclaration canvas::parse_short_font(const string &val)
{
  FontDeclaration declaration;

  declaration.fontStyle = "normal";
  declaration.fontVariant = "normal";
  declaration.fontWeight = "normal";
  declaration.fontSize = "normal";
  declaration.lineHeight = "normal";

	vector<string> tokens;
	split_string(val, tokens, string(" "), string(""), string("\""));

	int idx = 0;
	bool was_normal = false;
	bool is_family = false;
	string font_family;
	for(vector<string>::iterator tok = tokens.begin(); tok != tokens.end(); tok++)
	{
		idx = value_index(tok->c_str(), font_style_strings);
		if(!is_family)
		{
			if(idx >= 0)
			{
				if(idx == 0 && !was_normal)
				{
          declaration.fontWeight = *tok;
          declaration.fontVariant = *tok;
          declaration.fontStyle = *tok;
				} else
				{
          declaration.fontStyle = *tok;
				}
			} else
			{
				if(value_in_list(tok->c_str(), font_weight_strings))
				{
          declaration.fontWeight = *tok;
				} else
				{
					if(value_in_list(tok->c_str(), font_variant_strings))
					{
            declaration.fontVariant = *tok;
					} else if( iswdigit((*tok)[0]) )
					{
						vector<string> szlh;
						split_string(*tok, szlh, "/");

						if(szlh.size() == 1)
						{
              declaration.fontSize = std::to_string(std::stof(szlh[0]));
						} else	if(szlh.size() >= 2)
						{
              declaration.fontSize = std::to_string(std::stof(szlh[0]));
              declaration.lineHeight = szlh[1];
						}
					} else
					{
						is_family = true;
						font_family += *tok;
					}
				}
			}
		} else
		{
			font_family += *tok;
		}
	}
  declaration.fontFamily = font_family;

  return declaration;
}