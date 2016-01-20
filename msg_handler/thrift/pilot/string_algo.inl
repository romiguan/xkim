/*
 * Copyright (c) 2013 www.360buy.com
 * All rights reserved.
 *
 * @FILE: 		string_algo.inl
 * @AUTHOR: 	liyongqiang (yfliyongqiang@360buy.com)
 * @DATE: 		2013-01-24
 * @VERSION: 	1.0
 *
 * @DESCRIPTION:Implementation of class StringAlgo
 */
template<class T>
bool StringAlgo::string_to(const string &s,T &v){
	try{
		v=lexical_cast<T>(s);
		return true;
	}catch(bad_lexical_cast &){ 
		return false;
	}   
}

template<class T>
bool StringAlgo::string_of(const T &v,string &s){
	try{
		s=lexical_cast<string>(v);
		return true;
	}catch(bad_lexical_cast &){ 
		return false;
	}   
}
