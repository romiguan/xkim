#include "string_algo.h"

StringAlgo::StringAlgo()
{
}

StringAlgo::~StringAlgo()
{

}

bool StringAlgo::start_with(const string &str,const string &start)
{
	return (str.compare(0,start.size(),start)==0);
}

bool StringAlgo::end_with(const string &str,const string &end)
{
	return (str.compare(str.size()-end.size(),end.size(),end)==0);
}

void StringAlgo::to_lower(string &str,int len)
{
	char distance='A'-'a';
	for(int i=0;i<len;++i){
		if(str[i]>='A' && str[i]<='Z'){
			str[i]-=distance;
		}
	}
}

void StringAlgo::tokenize(const string &s,const string &split,vector<string> &res_vec){
	res_vec.clear();
	if(s.empty()){
		return;
	}   
	string::size_type pos(0),last_pos(0);
	pos=s.find(split);
	while(pos!=string::npos){
		string _t=s.substr(last_pos,pos-last_pos);
		last_pos=pos+split.length();
		pos=s.find(split,last_pos);
		if(_t.compare("")==0){
			continue;
		}   
		res_vec.push_back(_t);
	}   
	string _t=s.substr(last_pos);
	if(_t.compare("")!=0){
		res_vec.push_back(_t);
	}
}

