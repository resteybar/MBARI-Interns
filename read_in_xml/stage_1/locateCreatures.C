//============================================================================
// Name        : locateCreatures.cpp
// Author      : Mayra Ochoa & Raymond Esteybar
// Version     :
// Copyright   : Your copyright notice
// Description : Parses through .xml to gather the values in <object> 
//		 for the <name> & <bndbox> dimensions
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

using namespace std;
using namespace xercesc;

#define DEBUG

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

// Functions
void storeValue(Creature& temp, const string& tagName, const string& tagValue);    		// Stores tag value in temporary 'Creature' Object
string getmyXML(const string& descrip, const uint& fnum);								// Get .xml in directory
void getObjectValues(XercesDOMParser *itsParser, vector<Creature>& creaturesFound); 	// Reads .xml for values in <object> ... </object>

int main(const int argc, const char** argv) {

	#ifdef DEBUG
    PauseWaiter pause;
    setPause(true);
    #endif

	XMLPlatformUtils::Initialize();

	// Variables Utilized
	XercesDOMParser *itsParser = new XercesDOMParser;	// Ensures File is readable
	vector<Creature> creaturesFound;					// Store 'Creature' values

	bool singleFrame = false;
	int numSpots = 0;
	uint frameNum = 0;
	Image< PixRGB<byte> > inputRaw, inputScaled;

	ModelManager manager("Read .xml Files");
	DetectionParameters dp = DetectionParametersSingleton::instance()->itsParameters;

	nub::soft_ref<OutputFrameSeries> ofs(new OutputFrameSeries(manager));
	nub::soft_ref<InputFrameSeries> ifs(new InputFrameSeries(manager));

	manager.addSubComponent(ofs);
	manager.addSubComponent(ifs);

	if (manager.parseCommandLine(argc, argv, "", 0, -1) == NULL)
		LFATAL("Invalid command line argument. Aborting program now !");

	manager.start();
	Dims scaledDims = ifs->peekDims();

	// Read all .xml
	while(1) {

		// Read new image in
		FrameState is = FRAME_NEXT;

		if (!singleFrame)
			is = ifs->updateNext();
		else
			is = FRAME_FINAL;

		if (is == FRAME_COMPLETE )	break; // done
		if (is == FRAME_NEXT || is == FRAME_FINAL) { // new frame

			LINFO("Reading new frame");

			// Get Frame
			inputRaw = ifs->readRGB();
			frameNum = ifs->frame();
			string description(manager.getOptionValString(&OPT_InputFrameSource).c_str());

			description = "/" + getmyXML(description, frameNum);

			// Read File
			itsParser->resetDocumentPool();
			itsParser->parse(description.c_str()); // Ensures the file is readable

			// 	- Extract Values
			if (itsParser->getErrorCount() == 0) {
				getObjectValues(itsParser, creaturesFound);
			} else {
			  cout << "Error when attempting to parse the XML file : " << description.c_str() << endl;
			  return -1;
			}

			#ifdef DEBUG
			if ( pause.checkPause()) Raster::waitForKey();// || ifs->shouldWait() || ofs->shouldWait()) Raster::waitForKey();
			#endif
		}
	}
	manager.stop();

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
