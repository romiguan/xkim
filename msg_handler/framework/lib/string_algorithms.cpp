#include "string_algorithms.h"

#include <cstring>
#include <cwchar>
#include <algorithm>

using std::copy;
using std::string;
using std::vector;
using std::wstring;

namespace util
{

int StringAlgorithms::_Min(int a, int b)
{
	return a > b ? b : a;
}

int StringAlgorithms::_Min(int a, int b, int c)
{
	int t = _Min(a, b);
	return t > c ? c : t;
}

int StringAlgorithms::_Max(int a, int b)
{
	return a > b ? a : b;
}

int StringAlgorithms::_Max(int a, int b, int c)
{
	int t = _Max(a, b);
	return t > c ? t : c;
}

int StringAlgorithms::Trim(char in[], char replace)
{
	int space = 0;
	char *p1, *p2;

	p1 = p2 = in;

	//略去前导空白
	while(isspace(*p2))
	{
		++p2;
	}

	while(*p2 != '\0')
	{
		if(!isspace(*p2))
		{
			if(space)
			{
				*p1++ = replace;
			}

			*p1++ = *p2++;
			space = 0;
		}
		else
		{
			++p2;
			++space;
		}
	}

	*p1 = '\0';

	return strlen(in);
}

int StringAlgorithms::Trim(string& in, char replace)
{
	char buff[in.length() + 1];
	copy(in.begin(), in.end(), buff);
	buff[in.length()] = '\0';
	Trim(buff, replace);
	in = buff;
	return in.size();
}

int StringAlgorithms::TrimCopy(const char in[], string& out, char replace)
{
	return TrimCopy(string(in), out, replace);
}

int StringAlgorithms::TrimCopy(const string& in, string& out, char replace)
{
	char buff[in.length() + 1];
	copy(in.begin(), in.end(), buff);
	buff[in.length()] = '\0';
	Trim(buff, replace);
	out = buff;
	return out.size();
}

template <typename T>
int StringAlgorithms::_EditDistance(const T tstr1[], size_t tstr1len,
		const T tstr2[], size_t tstr2len)
{
	int m[tstr1len + 1][tstr2len + 1];

	for(unsigned int i = 0; i < tstr1len + 1; ++i)
	{
		m[i][0] = i;
	}

	for(unsigned int i = 0; i < tstr2len + 1; ++i)
	{
		m[0][i] = i;
	}

	int cost;

	for(unsigned int i = 1; i < tstr1len + 1; ++i)
	{
		for(unsigned int j = 1; j < tstr2len + 1; ++j)
		{
			cost = tstr1[i - 1] == tstr2[j - 1] ? 0 : 1;

			m[i][j] = _Min(m[i - 1][j] + 1, m[i][j - 1] + 1, m[i - 1][j - 1] + cost);

			if(i > 2 && j > 2 && tstr1[i - 1] == tstr2[j - 2] && tstr1[i - 2] == tstr2[j - 1])
			{
				m[i][j] = _Min(m[i][j], m[i - 2][j - 2] + cost);
			}
		}
	}

	return m[tstr1len][tstr2len];
}

int StringAlgorithms::EditDistance(const char str1[], const char str2[])
{
	size_t str1len, str2len;

	str1len = strlen(str1);
	str2len = strlen(str2);

	return _EditDistance<char>(str1, str1len, str2, str2len);
}

int StringAlgorithms::EditDistance(const string& str1, const string& str2)
{
	return EditDistance(str1.c_str(), str2.c_str());
}

int StringAlgorithms::EditDistance(const wchar_t wstr1[], const wchar_t wstr2[])
{
	size_t wstr1len, wstr2len;

	wstr1len = wcslen(wstr1);
	wstr2len = wcslen(wstr2);

	return _EditDistance<wchar_t>(wstr1, wstr1len, wstr2, wstr2len);
}

int StringAlgorithms::EditDistance(const wstring& wstr1, const wstring& wstr2)
{
	return EditDistance(wstr1.c_str(), wstr2.c_str());
}

template <typename T>
int StringAlgorithms::_LCSS(const T tstr1[], size_t tstr1len, const T tstr2[],
		size_t tstr2len)
{
	int m[tstr1len + 1][tstr2len + 1];

	for(unsigned int i = 0; i < tstr1len + 1; ++i)
	{
		m[i][0] = 0;
	}

	for(unsigned int i = 0; i < tstr2len + 1; ++i)
	{
		m[0][i] = 0;
	}

	for(unsigned int i = 1; i < tstr1len + 1; ++i)
	{
		for(unsigned int j = 1; j < tstr2len + 1; ++j)
		{
			if(tstr1[i - 1] == tstr2[j - 1])
			{
				m[i][j] = m[i - 1][j - 1] + 1;
			}
			else
			{
				m[i][j] = _Max(m[i - 1][j], m[i][j - 1]);
			}
		}
	}

	return m[tstr1len][tstr2len];
}

int StringAlgorithms::LCSS(const char str1[], const char str2[])
{
	size_t str1len, str2len;

	str1len = strlen(str1);
	str2len = strlen(str2);

	return _LCSS<char>(str1, str1len, str2, str2len);
}

int StringAlgorithms::LCSS(const string& str1, const string& str2)
{
	return LCSS(str1.c_str(), str2.c_str());
}

int StringAlgorithms::LCSS(const wstring& wstr1, const wstring& wstr2)
{
	return LCSS(wstr1.c_str(), wstr2.c_str());
}

int StringAlgorithms::LCSS(const wchar_t wstr1[], const wchar_t wstr2[])
{
	size_t wstr1len, wstr2len;

	wstr1len = wcslen(wstr1);
	wstr2len = wcslen(wstr2);

	return _LCSS<wchar_t>(wstr1, wstr1len, wstr2, wstr2len);
}

int StringAlgorithms::Split(const char str[], char delim, vector<string>& results)
{
	const char *p1, *p2;

	p1 = p2 = str;
	results.clear();

	while(*p1 != '\0')
	{
		while(*p2 != '\0' && *p2 != delim)
		{
			++p2;
		}

		results.push_back(string(p1, p2));

		p1 = (*p2 == '\0' ? p2 : ++p2);
	}

	return results.size();
}

int StringAlgorithms::Split(const string& str, char delim, vector<string>& results)
{
	return Split(str.c_str(), delim, results);
}

int StringAlgorithms::Split(const char str[], const char *delim, vector<string>& results)
{
    results.clear();

    int len = strlen(delim);

    const char *l = str;
    const char *r = str;

    while(*l != '\0')
    {
        while(*r != '\0' && strncmp(r, delim, len) != 0)
        {
            r++;
        }

        results.push_back(string(l, r));

        if (*r != '\0')
        {
            r += len;
        }

        l = r;
    }

    if (strncmp(l - len, delim, len) == 0)
    {
        results.push_back("");
    }

    return results.size();
}

int StringAlgorithms::Split(const string& str, const char *delim, vector<string>& results)
{
	return Split(str.c_str(), delim, results);
}

}
