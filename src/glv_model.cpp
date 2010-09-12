/*
 *  glv_model.cpp
 *  GLV
 *
 *  Created by Lance on 8/19/10.
 *  Copyright 2010 UCSB. All rights reserved.
 *
 */

#include "glv_model.h"
#include <cctype>	// isalnum, isblank
#include <stdio.h>	// sscanf
#include <string.h>	// strchr, strpbrk

namespace glv{

template<> int toString<bool>(std::string& dst, const bool& src){
	dst = src ? "1" : "0";
	return 1;
}
template<> int toString<int>(std::string& dst, const int& src){
	char buf[20]; snprintf(buf, sizeof(buf), "%i", src);
	dst = buf;
	return 1;
}
template<> int toString<float>(std::string& dst, const float& src){
	char buf[32]; snprintf(buf, sizeof(buf), "%g", src); // "%.24g"
	dst = buf;
	return 1;
}
template<> int toString<double>(std::string& dst, const double& src){
	char buf[40]; snprintf(buf, sizeof(buf), "%lg", src); // "%.32lg"
	dst = buf;
	return 1;
}
template<> int toString<std::string>(std::string& dst, const std::string& src){
	dst = src; return 1;
}
int toString(std::string& dst, const char * src){ return toString(dst, std::string(src)); }

template<> int fromToken<bool>(bool& dst, const std::string& src){
	char v = dst;
	int r = sscanf(src.c_str(), "%hhi", &v) > 0;
	dst = v;
	return r;
}
template<> int fromToken<int>(int& dst, const std::string& src){
	return sscanf(src.c_str(), "%i", &dst) > 0;
}
template<> int fromToken<float>(float& dst, const std::string& src){
	return sscanf(src.c_str(), "%g", &dst) > 0;
}
template<> int fromToken<double>(double& dst, const std::string& src){
	return sscanf(src.c_str(), "%lg", &dst) > 0;
}
template<> int fromToken<std::string>(std::string& dst, const std::string& src){
	int b = src.find_first_not_of("\"");
	int e = src.find_last_not_of("\"");
	dst = src.substr(b, e-b+1);
	return 1;
}
template<class T>
static int fromToken(T * dst, int size, const std::string& src, const char * match, const char * format){
	const char * s = src.c_str();
	int i=-1;
	while(++i<size && s){
		if(!(s = strpbrk(s, match))) break;
		T v; sscanf(s, format, &v);
		dst[i] = v;
		s = strpbrk(s, " ,");
	}
	return i;
}
template<> int fromToken<bool>(bool * dst, int size, const std::string& src){
	return fromToken(dst,size,src, "01","%hhi");
}
template<> int fromToken<int>(int * dst, int size, const std::string& src){
	return fromToken(dst,size,src, "0123456789-+","%i");
}
template<> int fromToken<float>(float * dst, int size, const std::string& src){
	return fromToken(dst,size,src, "0123456789-+.","%g");
}
template<> int fromToken<double>(double * dst, int size, const std::string& src){
	return fromToken(dst,size,src, "0123456789-+.","%lg");
}
template<> int fromToken<std::string>(std::string * dst, int size, const std::string& src){
	const char * s = src.c_str();
	int i=-1;
	while(++i<size && s){
		if(!(s = strchr(s, '\"'))) break;
		++s;
		const char * e = strchr(s, '\"');
		if(!e) break;
		dst[i].assign(s, e-s);
		s = strpbrk(e, " ,");
	}
	return i;
}

template<> int toToken<std::string>(std::string& dst, const std::string& src){
	dst = "\"" + src + "\"";
	return 1;
}
int toToken(std::string& dst, const char * src){ return toToken(dst, std::string(src)); }


int fromToken(Data& d, const std::string& s){
	switch(d.type()){
	case Data::BOOL:	return glv::fromToken(       d.elems<bool>(), d.size(), s);
	case Data::INT:		return glv::fromToken(        d.elems<int>(), d.size(), s);
	case Data::FLOAT:	return glv::fromToken(      d.elems<float>(), d.size(), s);
	case Data::DOUBLE:	return glv::fromToken(     d.elems<double>(), d.size(), s);
	case Data::STRING:	return glv::fromToken(d.elems<std::string>(), d.size(), s);
	default: return 0;
	}
}
int toString(std::string& s, const Data& d){
	if(!d.hasData()) return 0;
	switch(d.type()){
	case Data::BOOL:	return glv::toString(s,        d.elems<bool>(), d.size());
	case Data::INT:		return glv::toString(s,         d.elems<int>(), d.size());
	case Data::FLOAT:	return glv::toString(s,       d.elems<float>(), d.size());
	case Data::DOUBLE:	return glv::toString(s,      d.elems<double>(), d.size());
	case Data::STRING:	return glv::toString(s, d.elems<std::string>(), d.size());
	default: return 0;
	}
}
int toToken(std::string& s, const Data& d){
	if(!d.hasData()) return 0;
	switch(d.type()){
	case Data::BOOL:	return glv::toToken(s,        d.elems<bool>(), d.size());
	case Data::INT:		return glv::toToken(s,         d.elems<int>(), d.size());
	case Data::FLOAT:	return glv::toToken(s,       d.elems<float>(), d.size());
	case Data::DOUBLE:	return glv::toToken(s,      d.elems<double>(), d.size());
	case Data::STRING:	return glv::toToken(s, d.elems<std::string>(), d.size());
	default: return 0;
	}
}



Data::Data()
:	mData(0), mOffset(0), mStride(1), mType(Data::VOID)
{
	shapeAll(0);
}

Data::Data(Data& v){ *this = v; }
Data::Data(const Data& v){ *this = v; }

Data::Data(Data::Type type, int n1, int n2, int n3, int n4)
:	mData(0), mOffset(0), mStride(1), mType(type)
{
	int s[] = {n1,n2,n3,n4};
	shape(s,4);
}

Data::~Data(){
	free();
}

Data& Data::operator= (const Data& v){
	if(&v != this){
		setRaw(v.mData, v.offset(), v.type());
		stride(v.stride());
		for(int i=0;i<maxDim();++i) mSizes[i]=v.mSizes[i];
	}
	return *this;
}

bool Data::operator==(const Data& v) const {
	if(hasData() && v.hasData()){
		// TODO: how do we compare numbers and strings?

		// compare maximum possible number of elements
		int n = size() < v.size() ? size() : v.size();

		#define OP(t1, t2)\
			for(int i=0; i<n; ++i){\
				if(elem<t1>(i) != v.elem<t2>(i)) return false;\
			} return true

		#define OPALL(t)\
			switch(v.type()){\
			case Data::BOOL:	OP(t, bool);\
			case Data::INT:		OP(t, int);\
			case Data::FLOAT:	OP(t, float);\
			case Data::DOUBLE:	OP(t, double);\
			default: return false;\
			}

		switch(type()){
		case Data::BOOL:	OPALL(bool);
		case Data::INT:		OPALL(int);
		case Data::FLOAT:	OPALL(float);
		case Data::DOUBLE:	OPALL(double);
		case Data::STRING:	OP(std::string, std::string);
		default:;
		}
	}
	return false;
	#undef OP
	#undef OPALL
}

Data& Data::assign(const Data& v, int ind1, int ind2){

	if(hasData() && v.hasData()){
		int id= indexFlat(ind1,ind2);	// starting index of destination
		int nd= size()-id;				// number of destination elements to assign
		int n = nd < v.size() ? nd : v.size();

		#define OP(t1, t2)\
			for(int i=0; i<n; ++i){\
				elem<t1>(i+id) = v.elem<t2>(i);\
			} break

		#define OPALL(t)\
			switch(v.type()){\
			case BOOL:			OP(t, bool);\
			case INT:			OP(t, int);\
			case FLOAT:			OP(t, float);\
			case DOUBLE:		OP(t, double);\
			case STRING:\
			default:;\
			} break

		switch(type()){
		case Data::BOOL:	OPALL(bool);
		case Data::INT:		OPALL(int);
		case Data::FLOAT:	OPALL(float);
		case Data::DOUBLE:	OPALL(double);
		case Data::STRING: //printf("copy strings: %p -> %p\n", &v, this);
			switch(v.type()){
//			case Data::BOOL:	v.toString(data<std::string>());
//			case Data::FLOAT:	OP(t, float);
//			case Data::DOUBLE:	OP(t, double);
			case Data::STRING:	OP(std::string, std::string);
			default:;
			} break;
		default:;
		}
	}
	return *this;
	#undef OP
	#undef OPALL
}

void Data::clone(){
	if(hasData()){
		//int cnt = references(mData);
		
		// cnt == 0		data points to external, unmanaged data
		// cnt  > 1		data points to another Data's cloned data
		// cnt == 1		data points to my own cloned data
//		if(cnt!=1){
			Data old(*this);
			realloc(type());
			assign(old);
//		}
//		else{
//			realloc(typeKind(), size());
//		}
	}
	else{
		realloc(type());
	}
}

void Data::free(){

	if(release(mData)){
		//printf("(%p) Data::free %p\n", this, mData);
		switch(type()){
		case Data::BOOL:	delete[] data<bool>(); break;
		case Data::INT:		delete[] data<int>(); break;
		case Data::FLOAT:	delete[] data<float>(); break;
		case Data::DOUBLE:	delete[] data<double>(); break;
		case Data::STRING:	delete[] data<std::string>(); break;
		default:;
		}
//		::free(mData);
	}
	mData=0;
	mOffset=0;
	mStride=1;
	shapeAll(0);
	mType=Data::VOID;
}

int Data::order() const {
	int i=maxDim()-1;
	for(; i>=0; --i){
		if(size(i) > (i?1:0)) break;
	}
	return i+1;
}

void Data::print() const{
	printf("%p + %d, %d", mData, offset(), size(0));
	for(int i=1;i<order();++i){
		printf(" x %d", mSizes[i]);
	}
	printf(", %s %s\n", typeToString(type()).c_str(), toToken().c_str());
}

// if sizes==0, then keep current shape
void Data::realloc(Data::Type t, const int * sizes, int n){
	if(sizes){
		free();
		shape(sizes, n);
	}
	else{
		Data old(*this);
		free();
		shape(old.mSizes, old.order());
	}	

	if(size()){
		mType   = t;
		mOffset = 0;
		mStride = 1;

		switch(type()){
		case Data::BOOL:	mData = pointer(new bool[size()]); break;
		case Data::INT:		mData = pointer(new int[size()]); break;
		case Data::FLOAT:	mData = pointer(new float[size()]); break;
		case Data::DOUBLE:	mData = pointer(new double[size()]); break;
		case Data::STRING:	mData = pointer(new std::string[size()]); break;
		default:			return;
		}
		acquire(mData);
	}
}

Data& Data::resize(const int * sizes, int n){
	if(product(sizes,n) != size()){
		realloc(type(), sizes, n);
	}
	return *this;
}

Data& Data::set(Data::Type t, const int * sizes, int n){
	if(type()!=t || size()!=product(sizes,n)){
		if(hasData()){
			realloc(t, sizes,n);
		}
		else{
			mType=t;
			shape(sizes,n);
		}
	}
	return *this;
}

void Data::setRaw(void * dt, int off, Type ty){
	free();
	mData  = pointer(dt);
	mOffset= off;
	mType  = ty;
	incrementCount(mData);
}

Data& Data::shape(const int * sizes, int n){
	if(n){
		if(n <= maxDim()){
			for(int i=0;i<n;++i) mSizes[i]=sizes[i];
			for(int i=n;i<maxDim();++i) mSizes[i]=1;
		}
		else{
			for(int i=0;i<maxDim();++i) mSizes[i]=sizes[i];
		}
	}
	return *this; 
}

Data& Data::shapeAll(int n){
	for(int i=0;i<maxDim();++i) mSizes[i]=n;
	return *this;
}

int Data::sizeType() const {
	switch(type()){
	case Data::BOOL:	return sizeof(bool);
	case Data::INT:		return sizeof(int);
	case Data::FLOAT:	return sizeof(float);
	case Data::DOUBLE:	return sizeof(double);
	case Data::STRING:	return sizeof(std::string);
	default:			return 0;
	}
}

Data Data::slice(int beg, int sz) const {
	Data r(*this);
	int b = r.offset() + beg;
	
	// constrain offset and size to limits of parent data
	if(b < 0) b = 0;

	// TODO: constrain size
	int s = sz;
	
	r.mOffset = b;
	r.size(s);
	return r;
}

std::string Data::typeToString(Type t){
	switch(t){
	case BOOL:	return "bool";
	case INT:	return "int";
	case FLOAT:	return "float";
	case DOUBLE:return "double";
	case STRING:return "std::string";
	default:	return "void";
	}
}

// TODO:	need to delete allocated Models
//			need to be able to remove Data
void ModelManager::add(const std::string& name, Data& v){
	if(isIdentifier(name)) mNameVal[name] = new Model(v);
}

void ModelManager::remove(const std::string& name){
	if(mNameVal.count(name)){
//		Model * m = mNameVal[name];
		mNameVal.erase(name);
//		if(allocated(m)){
//			delete m;
//		}
	}
}

void ModelManager::add(const std::string& name, Model& v){
	if(isIdentifier(name)) mNameVal[name] = &v;
}
void ModelManager::add(const std::string& name, const Model& v){
	if(isIdentifier(name)) mNameConstVal[name] = &v;
}

bool ModelManager::toToken(std::string& dst, const std::string& modelName) const {
	#define NEWLINE "\r\n"
	if(modelName.size())	dst = "[\"" + modelName + "\"] = {"NEWLINE;
	else					dst = "{"NEWLINE;

	std::string t;
	{
		NameValMap::const_iterator i = mNameVal.begin();
		for(; i!=mNameVal.end(); ++i){
			dst += "\t" + i->first + " = ";
			if(i->second->model().toToken(t)) dst += t+","NEWLINE;
		}
	}
	{
		NameConstValMap::const_iterator i = mNameConstVal.begin();
		for(; i!=mNameConstVal.end(); ++i){
			dst += "\t" + i->first + " = ";
			if(i->second->model().toToken(t)) dst += t+","NEWLINE;
		}
	}
	dst += "}"NEWLINE;
	#undef NEWLINE
	return true;
}

int ModelManager::fromToken(const std::string& src){

	unsigned p = src.find_first_of("{");
	if(p == std::string::npos) return 0;		// no table found, so return
	
	std::string key, val;
	const char * b = &src[p+1];	// start 1 character after opening '{'

	while(*b && *b!='}'){
		if(isalpha(*b) || *b=='_'){	// is character valid start of identifier?
			const char * e = b+1;
			while(isalnum(*e) || *e=='_') ++e;	// go to end of key name
			key.assign(b, e-b);

			while(isblank(*e) || *e=='=') ++e;	// go to '='
			
			// find next valid token
			b=e=strpbrk(e, "\"{0123456789.-+");

			if(!b) b=e=&src[src.size()];	// no more valid tokens, so go to end of string

			if(*b){
				// munch characters until end of token
				if(*e == '\"'){
					++e; while(*e!='\"') ++e; ++e;
				}
				else if(*e == '{'){
					while(*e!='}') ++e; ++e;
				}
				else{
					while((isdigit(*e) || *e=='.' || *e=='-' || *e=='+')) ++e;
				}

				val.assign(b,e-b);

				if(mNameVal.count(key)){
					//printf("%s = %s\n", key.c_str(), val.c_str());
					mNameVal[key]->modelFromToken(val);
				}
			}
			b=e;
		}
		++b;
	}
	//printf("%d\n", b-&v[0]);
	return b-&src[0];
}

bool ModelManager::isIdentifier(const std::string& s){
	if(isalpha(s[0]) || s[0]=='_'){
		unsigned i=1;
		for(; i<s.size(); ++i){
			if(!isalnum(s[i])) break;
		}
		if(s.size()==i) return true;
	}
	return false;
}



bool StateSpace::stateToString(std::string& dst, const std::string& name){
	if(name.size())	dst = "[\"" + name + "\"] = {\r\n";
	else			dst = "{\r\n";

	NameToIndex::iterator i = mNameToIndex.begin();
	
	std::string t;
	DataVector& v = hasState(name) ? mStates[name] : mState;

	for(; i!=mNameToIndex.end(); ++i){
		dst += "\t" + i->first + " = ";
		if(toString(t, v[i->second])) dst += t+",\r\n";
	}
	
	dst += "}\r\n";
	return true;
}

} // glv::
