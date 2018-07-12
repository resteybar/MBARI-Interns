#include <iostream>
#include <sstream>
#include <string>
#include <signal.h>
#include <fstream>
#include <vector>
#include <boost/algorithm/string.hpp>


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

using namespace std;
#define DEBUG

string getmyXML(string descrip, uint fnum);

int main(const int argc, const char** argv) {
    
#ifdef DEBUG
    PauseWaiter pause;
    setPause(true);
#endif
    
    // View Class Passed
    string className = ">Class Name<";
    
    DetectionParameters dp = DetectionParametersSingleton::instance()->itsParameters;
    
    ModelManager manager("Read .xml Files");
    
    cout << "LINFO: ";
    LINFO("THIS IS LINFO");
    cout << endl;
    //    cout << "LDEBUG: ";
    //    cout << endl;
    //    LFATAL("THIS IS LFATAL");
    
    nub::soft_ref<OutputFrameSeries> ofs(new OutputFrameSeries(manager));
    manager.addSubComponent(ofs);
    
    nub::soft_ref<InputFrameSeries> ifs(new InputFrameSeries(manager));
    manager.addSubComponent(ifs);
    
    // parse the command line
    if (manager.parseCommandLine(argc, argv, "", 0, -1) == NULL)
    LFATAL("Invalid command line argument. Aborting program now !");
    
    // start all the ModelComponents
    manager.start();
    
    bool singleFrame = false;
    int numSpots = 0;
    uint frameNum = 0;
    Image< PixRGB<byte> > inputRaw, inputScaled;
    
    while(1)
    {
        // read new image in?
        FrameState is = FRAME_NEXT;
        
        if (!singleFrame)
        is = ifs->updateNext();
        else
        is = FRAME_FINAL;
        
        if (is == FRAME_COMPLETE) break; // done
        if (is == FRAME_NEXT || is == FRAME_FINAL) // new frame
        {
            LINFO("Reading new frame");
            
            // Output Frame #
            //    ____________
            //            numSpots = 0;
            // cache image
            inputRaw = ifs->readRGB();
            //            inputScaled = rescale(inputRaw, scaledDims);
            
            frameNum = ifs->frame();
            
            // Getting filename
            string description(manager.getOptionValString(&OPT_InputFrameSource).c_str());
            
            cout << "My new path: " << getmyXML(description, frameNum)<< endl;
            #ifdef DEBUG
            if ( pause.checkPause()) Raster::waitForKey();// || ifs->shouldWait() || ofs->shouldWait()) Raster::waitForKey();
            #endif
        }
    }
    
    manager.stop();
    
    return 0;
}
string getmyXML(string descrip, uint fnum){
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
