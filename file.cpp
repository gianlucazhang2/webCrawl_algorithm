#include <string>
#include <cctype>
#include <cstring>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>
#include <list>
#include <regex>
#include "file.h"

//default constructor
File::File(std::string filePath) {
	url = filePath; 
	std::ifstream fileStream(filePath); 
	//read HTML file into string
	if (fileStream.is_open()) {
		std::string fcontent((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());
		content = fcontent; 
	}
	//parse out document name
	size_t lastSlashPos = filePath.find_last_of('/');
	if (lastSlashPos != std::string::npos) {
		doc = filePath.substr(lastSlashPos+1);
	}
	//parse out title
	unsigned int first = content.find("<title>"); 
	unsigned int last = content.find("</title>"); 
	title = content.substr(first+7, last-(first+7)); 
	//parse out description
	first = content.find("content=\""); 
	last = content.find("\">"); 
	desc = content.substr(first+9, last-(first+9));
	//parse out body
	first = content.find("<body>"); 
	last = content.find("</body>"); 
	body = content.substr(first, last - first); 
	//parse out links
	// regular expression to match href attributes in anchor tags
    std::regex linkRegex("<a\\s+[^>]*href\\s*=\\s*['\"]([^'\"]+)['\"][^>]*>");
    std::smatch match;

    // search for links in the HTML content
    std::string::const_iterator start = content.cbegin();
    while (std::regex_search(start, content.cend(), match, linkRegex)) {
        if (match.size() > 1) {
            links.push_back(match[1].str());
        }
        start = match.suffix().first;
    }

}

//accessors
std::string File::getTitle() {
	return title; 
}

std::string File::getDoc() {
	return doc; 
}

std::string File::getDesc() {
	return desc; 
}

std::string File::getContent() {
	return content; 
}

std::list<std::string> File::getLinks() {
	return links; 
}

std::string File::getBody() {
	return body; 
}

void File::setSnippet(std::vector<std::string> queryList) {
	std::string queryString = ""; 
	for (int i = 0; i < queryList.size(); i++) {
		if (i != queryList.size() -1) {
			queryString = queryString + queryList[i] + " "; 
		}
		else {
			queryString = queryString + queryList[i]; 
		}
	} 
	int queryPos = body.find(queryString); 
	if (queryPos == std::string::npos) {
		queryPos = body.find(queryList[0]); 
	}
	size_t periodPos = body.rfind(".", queryPos); 
	if (periodPos == std::string::npos) {
		periodPos = body.rfind('\t', queryPos); 
	}
	while (body[periodPos] == '.' || isspace(body[periodPos])) {
		periodPos++; 
	}
	snippet = body.substr(periodPos, 120); 
}

//overload << operator
std::ostream& operator << (std::ostream &out, const File &f) {
	out << "Title: " << f.title << std::endl;
	out << "URL: " << f.url <<std::endl; 
	out << "Description: " << f.desc << std::endl; 
	out << "Snippet: " << f.snippet << std::endl;
	return out; 
}