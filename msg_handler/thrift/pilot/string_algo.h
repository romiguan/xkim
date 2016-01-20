#ifndef __STRING_ALGO_H__
#define __STRING_ALGO_H__

#include <string>
#include <vector>
#include "boost/lexical_cast.hpp"
using boost::lexical_cast;
using boost::bad_lexical_cast;
using namespace std;

class StringAlgo
{
public:
	StringAlgo();
	~StringAlgo();
	static bool start_with(const string &str,const string &start);
	static bool end_with(const string &str,const string &end);
	static void to_lower(string &str,int len);
	static void tokenize(const string &s,const string &split,vector<string> &res_vec);
	template<typename T> static bool string_to(const string &s,T &v);
	template<typename T> static bool string_of(const T &v,string &s);
}; // end class StringAlgo

#include "string_algo.inl"

#endif //__STRING_ALGO_H__
