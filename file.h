#include <string>
#include <iostream>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <regex>

class File {
public: 
	//default constructor
	File(std::string filePath);
	
	//accessors
	std::string getTitle();
	
	std::string getDoc();
	
	std::string getDesc();
	
	std::string getContent();
	
	std::list<std::string> getLinks(); 
	
	std::string getBody();
	
	void setSnippet(std::vector<std::string> queryList); 
	
	//overload << operator
	friend std::ostream& operator << (std::ostream &out, const File &f); 
	
private: 
	std::string title, doc, desc, content, body, snippet, url; 
	std::list<std::string> links; 

};