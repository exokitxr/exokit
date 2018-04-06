#include <web_string.h>

string::size_type canvas::find_close_bracket(const string &s, string::size_type off, char open_b, char close_b)
{
	int cnt = 0;
	for(string::size_type i = off; i < s.length(); i++)
	{
		if(s[i] == open_b)
		{
			cnt++;
		} else if(s[i] == close_b)
		{
			cnt--;
			if(!cnt)
			{
				return i;
			}
		}
	}
	return string::npos;
}

void canvas::split_string(const string &str, vector<string> &tokens, const string &delims, const string &delims_preserve, const string &quote)
{
	if(str.empty() || (delims.empty() && delims_preserve.empty()))
	{
		return;
	}

	string all_delims = delims + delims_preserve + quote;

	string::size_type token_start	= 0;
	string::size_type token_end	= str.find_first_of(all_delims, token_start);
	string::size_type token_len	= 0;
	string token;
	while(true)
	{
		while( token_end != string::npos && quote.find_first_of(str[token_end]) != string::npos )
		{
			if(str[token_end] == '(')
			{
				token_end = find_close_bracket(str, token_end, '(', ')');
			} else if(str[token_end] == '[')
			{
				token_end = find_close_bracket(str, token_end, '[', ']');
			} else if(str[token_end] == '{')
			{
				token_end = find_close_bracket(str, token_end, '{', '}');
			} else
			{
				token_end = str.find_first_of(str[token_end], token_end + 1);
			}
			if(token_end != string::npos)
			{
				token_end = str.find_first_of(all_delims, token_end + 1);
			}
		}

		if(token_end == string::npos)
		{
			token_len = string::npos;
		} else
		{
			token_len = token_end - token_start;
		}

		token = str.substr(token_start, token_len);
		if(!token.empty())
		{
			tokens.push_back( token );
		}
		if(token_end != string::npos && !delims_preserve.empty() && delims_preserve.find_first_of(str[token_end]) != string::npos)
		{
			tokens.push_back( str.substr(token_end, 1) );
		}

		token_start = token_end;
		if(token_start == string::npos) break;
		token_start++;
		if(token_start == str.length()) break;
		token_end = str.find_first_of(all_delims, token_start);
	}
}

int canvas::value_index(const string &val, const string &strings, int defValue, char delim)
{
	if(val.empty() || strings.empty() || !delim)
	{
		return defValue;
	}

	int idx = 0;
	string::size_type delim_start	= 0;
	string::size_type delim_end	= strings.find(delim, delim_start);
	string::size_type item_len		= 0;
	while(true)
	{
		if(delim_end == string::npos)
		{
			item_len = strings.length() - delim_start;
		} else
		{
			item_len = delim_end - delim_start;
		}
		if(item_len == val.length())
		{
			if(val == strings.substr(delim_start, item_len))
			{
				return idx;
			}
		}
		idx++;
		delim_start = delim_end;
		if(delim_start == string::npos) break;
		delim_start++;
		if(delim_start == strings.length()) break;
		delim_end = strings.find(delim, delim_start);
	}
	return defValue;
}

bool canvas::value_in_list(const string& val, const string& strings, char delim)
{
	int idx = value_index(val, strings, -1, delim);
	if(idx >= 0)
	{
		return true;
	}
	return false;
}