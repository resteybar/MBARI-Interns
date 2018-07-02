//============================================================================
// Name        : readXMLData.cpp
// Author      : Raymond Esteybar
// Version     :
// Copyright   : Your copyright notice
// Description : Parses through .xml to gather the values in <object> for the <name> & <bndbox> dimensions
//============================================================================

#include <stdio.h>
#include <stdlib.h>

#include <vector>  // Change into a diff DS or make our own class
#include <string>
#include <sstream>
#include <iostream>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

using namespace std;
using namespace xercesc;

// Object Values
struct BoundingBox {
    int xmin;
    int ymin;
    int xmax;
    int ymax;
};

struct Creature {
    string name;
    BoundingBox dim;
};

// Store's value in temporary Creature object
void storeValue(Creature& temp, const string& tagName, const string& tagValue);    // Stores tag value in temporary 'Creature' Object

int main() {

	XMLPlatformUtils::Initialize();

	// Mayra's Code will handle grabbing the files
	string inputXML = "/Users/Gray/eclipse-workspace/readXMLData/f001211.xml";

	// Variables Utilized
	XercesDOMParser *itsParser = NULL; // Opens
	DOMDocument *domDocParser = NULL;
	DOMNodeList *list = NULL;
	vector<Creature> creaturesFound;

	try {
		itsParser = new XercesDOMParser;
	} catch(const DOMException &err) {
		cout << "Caught Exception\n";
	}

	itsParser->resetDocumentPool();

	itsParser->parse(inputXML.c_str()); // Ensures the file is readable

	// File is Readable
	if (itsParser->getErrorCount() == 0) {

		domDocParser = itsParser->getDocument();

		// How many instances of the '<tag>' found
		XMLCh *source = XMLString::transcode("object"); 		// Tag wanted
		list = domDocParser->getElementsByTagName(source);		// Returns list of '<tag>' found

		// Parse through each object to grab values
		for(int i = 0; i < list->getLength(); ++i) {

			DOMNode *node = list->item(i); 						// Gets the ith <object> in the list
			DOMNodeList *length = node->getChildNodes(); 		// Lines counted, including: "<object> ... </object>" = 13 lines total

			Creature temp;

			// Iterate through ea. <tag> in <object> ... </object> to retrieve values
			for(int k = 0; k < length->getLength(); ++k) {
				DOMNode *childNode = length->item(k);

				if(childNode->getNodeType() == DOMNode::ELEMENT_NODE) {						// Ensures we found a <tag>
					string tagNameObj = XMLString::transcode(childNode->getNodeName());		// <Gets Tag Name>
					string tagValueObj = XMLString::transcode(childNode->getTextContent());	// <tag> Gets Value </tag>

					// Grab Bounding Box Dimensions
					//	- Otherwise, get the <name>
					if(tagNameObj == "bndbox") {
						DOMNodeList *dimensions = childNode->getChildNodes();						// Gets all the <tags> in <bndbox>

						for(int j = 0; j < dimensions->getLength(); ++j) {							// Iterate each for dim. value
							DOMNode *dim = dimensions->item(j);

							if(dim->getNodeType() == DOMNode::ELEMENT_NODE) {
								string tagNameBB = XMLString::transcode(dim->getNodeName());		// <Gets Tag Name>
								string tagValueBB = XMLString::transcode(dim->getTextContent());	// <tag> Gets Value </tag>

								storeValue(temp, tagNameBB, tagValueBB);							// Store the dim values 1 by 1
							}
						}

						creaturesFound.push_back(temp);												// Store creature found b/c dim are the last values to collect

						break;
					} else if(tagNameObj == "name"){
						storeValue(temp, tagNameObj, tagValueObj);									// Store <name> value
					}
				}
			}
		}
	} else {
	  cout << "Error when attempting to parse the XML file : " << inputXML.c_str() << endl;
	  return -1;
	}

	// View creatures found in .xml
	cout << creaturesFound.size() << endl;

	for(int i = 0; i < creaturesFound.size(); ++i) {
		cout << i << ")\n";
		cout << "Name: " << creaturesFound[i].name << endl
				<< "Dim:\n"
				<< "\txmin: " << creaturesFound[i].dim.xmin << endl
				<< "\tymin: " << creaturesFound[i].dim.ymin << endl
				<< "\txmax: " << creaturesFound[i].dim.xmax << endl
				<< "\tymax: " << creaturesFound[i].dim.ymax << endl << endl;
	}

	cout << "Reached END" << endl;

	delete itsParser;

	return EXIT_SUCCESS;
}

// Stores value in temporary 'Creature' Object
void storeValue(Creature& temp, const string& tagName, const string& tagValue) {    // Stores value in temporary 'Creature' Object
	if(tagName == "name") {
        temp.name = tagValue;
    } else {

    	// Convert Bounding Box Value from 'string to int'
    	istringstream is(tagValue);
    	int dim = 0;
    	is >> dim;

    	if(tagName == "xmin") {
			temp.dim.xmin = dim;
		} else if(tagName == "ymin") {
			temp.dim.ymin = dim;
		} else if(tagName == "xmax") {
			temp.dim.xmax = dim;
		} else if(tagName == "ymax") {
			temp.dim.ymax = dim;
		}
    }
}
