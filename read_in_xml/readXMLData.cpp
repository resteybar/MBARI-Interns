//============================================================================
// Name        : readXMLData.cpp
// Author      : Raymond Esteybar
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

using namespace std;
using namespace xercesc;

int main() {

	XMLPlatformUtils::Initialize();

	string inputXML = "/Users/Gray/eclipse-workspace/readXMLData/f001211.xml";

	// Variables Utilized
	XercesDOMParser *itsParser = NULL; // Opens
	DOMDocument *domDocParser = NULL;
	DOMNodeList *list = NULL;



	try {
		itsParser = new XercesDOMParser;
	} catch(const DOMException &err) {
		cout << "Caught Exception\n";
	}
	cout << "Reached AFTER PARSER" << endl;


	itsParser->resetDocumentPool();

	itsParser->parse(inputXML.c_str()); // Ensures the file is readable

	vector<DOMNode*> nodes;

	if (itsParser->getErrorCount() == 0) {

		cout << "XML = Good\n";

		domDocParser = itsParser->getDocument();

		// How many instances of the '<tag>' found
		XMLCh *source = XMLString::transcode("object"); 		// Tag wanted
		list = domDocParser->getElementsByTagName(source);		// Returns list of '<tag>' found
		cout << "Tags Found: " << list->getLength() << endl;	// How many <tags> found

		// Bounding Box
		DOMNodeList *bndbox = NULL;
		XMLCh *srcBndBox = NULL;
		XMLCh *srcName = NULL;

		srcName = XMLString::transcode("name");
		srcBndBox = XMLString::transcode("bndbox");

		for(int i = 0; i < list->getLength(); ++i) {

			DOMNode *node = list->item(i); // Gets the ith <object>

			node->normalize();

			DOMNamedNodeMap *map = node->getAttributes();

			DOMNodeList *length = node->getChildNodes();


			for(int k = 0; k < length->getLength(); ++k) {
				DOMNode *childNode = length->item(k);

				if(childNode->getNodeType() == DOMNode::ELEMENT_NODE) {
					char *nodeName = XMLString::transcode(childNode->getNodeName());
					char *nodeValue = XMLString::transcode(childNode->getTextContent());
					cout << "Name of Node " << k + 1 << ") " << nodeName << endl;
					cout << "Name of Prefix:" << k + 1 << ") " << nodeValue << endl;
				}
			}

			cout << "\n\n";


//			char *nodeValue = XMLString::transcode(length->item(5)->getNodeName());

//			if() {
//				cout << "Found POSE\n";
//
//
//			}

//			cout << "Node Tag " << i + 1 << ": "<< nodeValue << endl;
//
//			cout << "Node Length " << i + 1 << ": "<< length->getLength() << endl << endl;

//			char* object = XMLString::transcode(map->item(2)->getNodeName());

//			cout << "Length of Map " << i + 1 << ": "<< map->getLength() << endl;
//			cout << "Item " << i + 1 << ": "<< object << endl;






//			char* object = XMLString::transcode(node->getNodeName());

//			cout << object << endl;

//			DOMNodeList *objectChilds = node->getChildNodes();
//
//			cout << "Children: " << objectChilds->getLength() << endl;

//			DOMNode *child = objectChilds->item(0);
//
//			char* childVal = XMLString::transcode(child->getNodeName());
//
//			cout << "Child Value: " << childVal << endl;

//			char* object = XMLString::transcode(node->getNodeName());

//			cout << "# Name: " << object << endl;
//			bndbox = domDocParser->getElementsByTagName(srcBndBox);
		}

//		cout << *list->item(0)->getTextContent() << endl;

//		// Grab item
//		for(int i = 0; i < list->getLength(); ++i)
//			nodes.push_back(list->item(i));
//
//		cout << "Nodes Size: " << nodes.size() << endl;

		// Get Values
//		XMLCh *source = XMLString::transcode("name"); 		// Tag wanted


	} else {
	  cout << "Error when attempting to parse the XML file : " << inputXML.c_str() << endl;
	  return -1;
	}


	cout << "Reached END" << endl;

	delete itsParser;

	return EXIT_SUCCESS;
}
