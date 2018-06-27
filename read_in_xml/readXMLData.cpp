//============================================================================
// Name        : readXMLData.cpp
// Author      : Raymond Esteybar
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C, Ansi-style
//============================================================================

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

int main(void) {
//	XercesDOMParser parser;

	cout << "Reached BEFORE STRING" << endl;

	string inputXML = "/Users/Gray/Documents/MBARI/avedac-mbarivision/data/f001211.xml";

	cout << "Reached AFTER STRING" << endl;

	// DOM parser instance
	cout << "Reached BEFORE PARSER" << endl;

	XercesDOMParser *itsParser;

	XMLPlatformUtils::Initialize();

	try {
		itsParser = new XercesDOMParser;
	} catch(const DOMException &err) {
		cout << "Caught Exception\n";
	}
	cout << "Reached AFTER PARSER" << endl;
  /*itsParser->setDoNamespaces(true);
  itsParser->setDoSchema(true);
  itsParser->setValidationScheme(XercesDOMParser::Val_Always);
  itsParser->setExternalNoNamespaceSchemaLocation(inputSchema.c_str());
  */

  // Error handler instance for the parser
//ErrReporter itsErrHandler = new ErrReporter();
//  itsParser->setErrorHandler(itsErrHandler);

//	itsParser->resetDocumentPool();
//	itsParser->parse(inputXML.c_str());
//
//	if (itsParser->getErrorCount() == 0) {
//	  xercesc::DOMDocument *domDocParser = itsParser->getDocument();
//	}
//	else {
//	  cout << "Error when attempting to parse the XML file : " << inputXML.c_str() << endl;
//	  return -1;
//	}


	cout << "Reached END" << endl;

//	parser.parse("/Users/Gray/Documents/MBARI/avedac-mbarivision/data/f001211.xml");

	delete itsParser;

	return EXIT_SUCCESS;
}
