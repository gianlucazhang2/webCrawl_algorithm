#include <map>
#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <sstream>
#include <set>
#include <algorithm>
#include "file.h"

//inverted index
std::map<std::string, std::set<std::string>> idx; 

//vector of matching documents
std::vector<std::string> docs; 

//set of all the web pages
std::set<std::string> webPages; 

std::vector<std::string> queryList;
std::string query = ""; 

//density map
std::map<std::string, double> densities; 
//web crawl stage
void webCrawl(std::string seed) {
	File file(seed); 
	
	//get the directory of the file
	std::string directory;
	size_t lastSlashPos = seed.find_last_of('/');
	if (lastSlashPos != std::string::npos && lastSlashPos < seed.size()) {
		directory = seed.substr(0, lastSlashPos + 1);
	}
	
	//check if link has already been visited
	std::map<std::string, std::set<std::string>>::const_iterator it;
	for (it = idx.begin(); it != idx.end(); ++it) {
		for (std::string link: it->second) {
			size_t lastSlash = link.find_last_of('/');
			std::string docName = link.substr(lastSlash + 1);
			if (file.getDoc() == docName) {return;}
		}
	}
	webPages.insert(seed); 

	//add each term to the inverted index
	std::string body = file.getBody(); 
	std::string word = ""; 
	for (unsigned int i = 0; i < body.length(); i++) {
		if (body[i] == ' ' || i == (body.length()-1) || body[i] == '\t' || body[i] == '\n') {
			idx[word].insert(seed); 
			word = ""; 
		}
		else {word += body[i];}
	}
	
	if (file.getLinks().empty()) {return;}
	
	//iterate through all links on the file
	for (std::string link: file.getLinks()) {
		webCrawl(directory + link); 
	}
}

//query search stage (regular search)
void searchIndex_r(std::vector<std::string> queryList) {
	std::map<std::string, int> allDocs; 
	//phrase search case
	bool phrase_search = false; 
	if (queryList.size() == 1 && queryList[0].find(' ') != std::string::npos) {phrase_search = true;} 
	for (unsigned int i = 0; i < queryList.size(); i++) {
		//phrase_search case
		if (phrase_search) {
			for (std::string web: webPages) {
				File file(web); 
				if (file.getContent().find(queryList[i]) != std::string::npos) {
					docs.push_back(web); 
				}
			}
		}
		if (idx.find(queryList[i]) != idx.end()) {
			std::set<std::string> documents = idx[queryList[i]]; 
			for (std::string doc: documents) {
				++allDocs[doc]; 
			}
		}
	}

	//remove docs that dont contain all the keywords
	std::map<std::string, int>::const_iterator it;
	for (it = allDocs.begin(); it != allDocs.end(); ++it) {
		if (it->second == queryList.size()) {
			docs.push_back(it->first);
		}
	}
}

//calculate density across all docs
double densityScoreAll(std::string word) {
	double count = 0; //total occurences across all docs
	long double len = 0; //total length of docs
	for (std::string doc: docs) {
		File file(doc); 
		std::string content = file.getContent(); 
		size_t pos = content.find(word);
        while (pos != std::string::npos) {
            count++;
            pos = content.find(word, pos + 1); // Update the search position
        }
	} 
	for (std::string w: webPages) {
		File file(w); 
		len += file.getContent().length(); 
	}
	return count / len; 
}
//calculates the density score
double densityScore(std::string url, std::string word) {
	File doc(url); 
	double count = 0; 
	double length = doc.getContent().length(); 
	std::string content = doc.getContent();
	while (content.find(word) != std::string::npos) { 
		content = content.substr(content.find(word) + word.length()); 
		count++;
	}
	return count / (length * densities[word]); 
}

//calculates the backlinks score
double backlinksScore(std::string url) {
	//get all the web pages that point to the given url
	File URL(url); 
	std::set<std::string> outlinks; 
	for (std::string w: webPages) {
		File file(w); 
		std::list<std::string> links = file.getLinks(); 
		for (std::string link: links) {
			size_t lastSlash = link.find_last_of('/');
			if (lastSlash == std::string::npos) {
				if (link == URL.getDoc()) {
					outlinks.insert(w);
				}
			}
			else {
				if (link.substr(lastSlash+1) == URL.getDoc()) {
					outlinks.insert(w); 
				}
			}
		}
	}
	//calculate the score
	double score = 0.0; 
	for (std::string page:outlinks) {
		File file(page); 
		double count = (file.getLinks().size()); 
		score += (1.0 / (1 + count)); 
	}
	return score; 
}

//compares two strings according to their page score
bool compareURLS(std::string u1, std::string u2) {
	double d1 = 0.0; 
	double d2 = 0.0; 
	for (int i = 0; i < queryList.size(); i++) {
		d1 += densityScore(u1, queryList[i]); 
		d2 += densityScore(u2, queryList[i]); 
	}
	double b1 = backlinksScore(u1); 
	double b2 = backlinksScore(u2); 
	double pagescore1 = (0.5 * d1) + (0.5 * b1); 
	double pagescore2 = (0.5 * d2) + (0.5 * b2);
	return (pagescore1 > pagescore2); 
}

//page ranking stage - sorts doc vector by their page score
void pageRank() {
	std::sort(docs.begin(), docs.end(), compareURLS); 
}


int main(int argc, char* argv[]) {
	if (argc < 4 || argc >= 7) { //incorrect command arguments
		return 1; 
	}
	//set variables from command
	std::string seed = argv[1]; 
	std::string output = argv[2]; 
	std::ofstream ostr(output);  
    bool phrase_search = false; 	
	
	for (int i = 3; i < argc; i++) { 
		query += argv[i]; 
		query += " "; 
	}
	if(query.find('"') != std::string::npos){
		phrase_search = true; 
		query.erase(std::remove(query.begin(), query.end(), '"'), query.end());
		std::stringstream ss(query); 
		std::string word; 
		while (getline(ss, word, ' ')) {
			queryList.push_back(word); 
		}
	}
	else {
		for (int i = 3; i < argc; i++) { 
			queryList.push_back(argv[i]); 
		}
	}
	
	webCrawl(seed);
	searchIndex_r(queryList); 	
	
	for (std::string q: queryList) {
		densities[q] = densityScoreAll(q); 
	}
	
	pageRank(); 
	
	if(phrase_search){
		for (std::vector<std::string>::iterator it = docs.begin(); it != docs.end();) {
			File file(*it); 
			if (file.getContent().find(query) == std::string::npos) {
				it = docs.erase(it);  
			}
			else {
				it++;
			}
		}
	}
	
	if (docs.empty()) {
		ostr << "Your search - "; 
		for (std::string q: queryList) {
			ostr << q << " "; 
		}
		ostr << "- did not match any documents." << std::endl; 
	}
	else {
		ostr << "Matching documents: " << std::endl;
		ostr << std::endl; 
		for (int i = 0; i < docs.size(); i++) {
			File file(docs[i]);
			file.setSnippet(queryList);  
			ostr << file;
			if (i != docs.size() -1) {
				ostr << std::endl;
			}			
		}
	}

	return 0; 
}