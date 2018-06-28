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

		// Grab item
		for(int i = 0; i < list->getLength(); ++i)
			nodes.push_back(list->item(i));

		cout << "Nodes Size: " << nodes.size() << endl;

		// Get Values
		DOMNodeList *list = nodes[0]->getChildNodes();
		cout << list->getLength() << endl;
		cout << *list->item(0)->getNodeName() << endl;

	} else {
	  cout << "Error when attempting to parse the XML file : " << inputXML.c_str() << endl;
	  return -1;
	}


	cout << "Reached END" << endl;

	delete itsParser;

	return EXIT_SUCCESS;
}
