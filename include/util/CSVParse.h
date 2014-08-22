
/**
 *  解析完的数据放到data这个二维数组中，通过getData(int m,int n)
 *  获取数据
 **/

#ifndef _CSVPARSE_
#define _CSVPARSE_

#include "cocos2d.h"
#include <stdio.h>
#include <iostream>
#include <fstream>

class CSVParse {
    
public:
    CSVParse(istream& fin = cin, string sep = ","):
    fieldsep(sep) {}
    ~CSVParse();
private:
    string                      fieldsep;        // separator characters
    vector<vector<string> >     data;
    void    split(vector<string>& field,string line);
    int     advplain(const string& line, string& fld, int);
    int     advquoted(const string& line, string& fld, int);
    
public: 
    bool            openFile(const char* fileName);
    const   char*   getData(int m,int n);
};

#endif

