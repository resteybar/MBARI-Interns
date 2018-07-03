//============================================================================
// Name        : locateCreatures.C
// Author      : Raymond Esteybar
// Version     :
// Copyright   : Your copyright notice
// Description : Parses through .xml to gather the values in <object> for the <name> & <bndbox> dimensions
//============================================================================

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include <vector>  // Change into a diff DS or make our own class
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <boost/algorithm/string.hpp>

#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOM.hpp>
#include <xercesc/parsers/XercesDOMParser.hpp>

#include "Media/FrameState.H"
#include "Image/OpenCVUtil.H"
#include "Channels/ChannelOpts.H"
#include "Component/GlobalOpts.H"
#include "Component/ModelManager.H"
#include "Component/JobServerConfigurator.H"
#include "Features/HistogramOfGradients.H"
#include "Image/FilterOps.H"    // for lowPass3y()
#include "Image/Kernels.H"      // for twofiftyfives()
#include "Image/ColorOps.H"
#include "Image/fancynorm.H"
#include "Image/MorphOps.H"
#include "Image/ShapeOps.H"   // for rescale()
#include "Raster/GenericFrame.H"
#include "Raster/PngWriter.H"
#include "Media/FrameRange.H"
#include "Media/FrameSeries.H"
#include "Media/SimFrameSeries.H"
#include "Media/MediaOpts.H"
#include "Neuro/SpatialMetrics.H"
#include "Neuro/StdBrain.H"
#include "Neuro/NeuroOpts.H"
#include "Neuro/Retina.H"
#include "Neuro/VisualCortex.H"
#include "Neuro/SimulationViewerStd.H"
#include "SIFT/Histogram.H"
#include "Simulation/SimEventQueueConfigurator.H"
#include "Util/sformat.H"
#include "Util/StringConversions.H"
#include "Util/Pause.H"
#include "Data/Logger.H"
#include "Data/MbariMetaData.H"
#include "Data/MbariOpts.H"
#include "DetectionAndTracking/FOEestimator.H"
#include "DetectionAndTracking/VisualEventSet.H"
#include "DetectionAndTracking/DetectionParameters.H"
#include "DetectionAndTracking/MbariFunctions.H"
#include "DetectionAndTracking/Segmentation.H"
#include "DetectionAndTracking/ColorSpaceTypes.H"
#include "DetectionAndTracking/ObjectDetection.H"
#include "DetectionAndTracking/Preprocess.H"
#include "Image/MbariImage.H"
#include "Image/MbariImageCache.H"
#include "Image/BitObject.H"
#include "Image/DrawOps.H"
#include "Image/MathOps.H"
#include "Image/IO.H"
#include "Learn/BayesClassifier.H"
#include "Media/MbariResultViewer.H"
#include "Motion/MotionEnergy.H"
#include "Motion/MotionOps.H"
#include "Motion/OpticalFlow.H"
#include "Util/StringConversions.H"
#include "Utils/Version.H"
#include "Component/ModelManager.H"
#include "DetectionAndTracking/DetectionParameters.H"
#include "Simulation/SimEventQueueConfigurator.H"

#define DEBUG

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

// Get .xml in directory
string getmyXML(const string& descrip, const uint& fnum);

// Reads .xml for values in <object> ... </object>
void getObjectValues(XercesDOMParser *itsParser, vector<Creature>& creaturesFound);


int main() {

	XMLPlatformUtils::Initialize();

	// Variables Utilized
	XercesDOMParser *itsParser = NULL; // Opens
	vector<Creature> creaturesFound;

	try {
		itsParser = new XercesDOMParser;
	} catch(const DOMException &err) {
		cout << "Caught Exception\n";
	}


	// File is Readable
	while(1) {

		// Mayra's Code will handle grabbing the files
		string inputXML = "/Users/mochoa/Documents/MBARI/avedac-mbarivision/data/f000004.xml";

		// Read File
		itsParser->resetDocumentPool();
		itsParser->parse(inputXML.c_str()); // Ensures the file is readable

		getFile();

		if (itsParser->getErrorCount() == 0) {
			getObjectValues(itsParser, creaturesFound);

		} else {
		  cout << "Error when attempting to parse the XML file : " << inputXML.c_str() << endl;
		  return -1;
		}
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

// Main Functions
// 	- Stores value in temporary 'Creature' Object
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

// 	- Get .xml in directory
string getmyXML(const string& descrip, const uint& fnum){
	vector<string>image_path;

	boost::split(image_path, descrip, boost::is_any_of("/"));

	vector<string>filename;
	boost::split(filename, image_path[image_path.size()-1], boost::is_any_of("#"));

	string fs = sformat("%s%06d.xml", filename[0].c_str() , fnum);

	string xml_path="";
	for(int i=1; i< image_path.size()-1; i++){
		xml_path.append(image_path[i]);
		xml_path.append("/");
	}
	xml_path.append(fs);
	return xml_path;
}

// 	- Reads .xml for values in <object> ... </object>
void getObjectValues(XercesDOMParser *itsParser, vector<Creature>& creaturesFound) {
	DOMNodeList *list = NULL;
	DOMDocument *domDocParser = itsParser->getDocument();

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
}
