/******************** REST disclaimer ***********************************
 * This file is part of the REST software framework.                     *
 *                                                                       *
 * Copyright (C) 2016 GIFNA/TREX (University of Zaragoza)                *
 * For more information see http://gifna.unizar.es/trex                  *
 *                                                                       *
 * REST is free software: you can redistribute it and/or modify          *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation, either version 3 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * REST is distributed in the hope that it will be useful,               *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have a copy of the GNU General Public License along with   *
 * REST in $REST_PATH/LICENSE.                                           *
 * If not, see http://www.gnu.org/licenses/.                             *
 * For the list of contributors see $REST_PATH/CREDITS.                  *
 *************************************************************************/

//////////////////////////////////////////////////////////////////////////
/// TRestAxionOptics is a class that allows to load externally
/// defined optics response files. This metadata class will be a generic,
/// abstract, class that will be inherited by other more specific metadata
/// classes. This class will define few common metadata members helping to
/// describe the optics alignment, position, and basic geometry specifications,
/// such as number of mirror rings, or additional entrance masks, such as
/// spider mask.
///
/// The derived metadata classes, such as TRestAxionGenericOptics or
/// TRestAxionMCPLOptics must implement the following virtual methods
/// TRestAxionOptics::GetPositionAtExit, TRestAxionOptics::GetDirectionAtExit
/// and TRestAxionOptics::GetEfficiency.
///
/// The following metadata parameters define the optics position, size and
/// alignment:
/// * **center**: It defines the center of the optics, entrance and exit
/// optics planes will be defined using the half lenght and the `center`
/// position.
/// * **axis**: It defines the optical axis direction.
/// * **length**: It defines the size of the optics, used to calculate
/// the optics plane entrance, and the optics plane exit.
///
/// A relevant parameter are the radii that define the mirror rings of
/// the optics. In practice we define the iner and outer radius of a ring
/// or corona (which is the space between two rings or mirrors). Thus, the
/// inner and outer radius of each ring defines the region where photons are
/// capable to go through. Photons hitting other regions should be ignored.
/// The method TRestAxionOptics::GetEntranceRing will return the ring
/// number where the photon entered. If the photon did not enter any
/// ring, then it will return -1.
///
/// The following metadata parameters define the ring entrances:
/// * **ringMinRadii**: It contains a list of lower radius values for
/// each ring.
/// * **ringMaxRadii**: It contains a list of higher radius values for
/// each ring.
///
/// On top of that we may define a spider mask which is usually present
/// at integrated x-ray optics as an structure to keep the rings in
/// position. The spider mask will prevent photons from entering inside
/// the optics mirroring system.
///
/// The following parameters are used to define the spider mask geometry:
/// * **spiderArmsSeparationAngle**: It defines the angular distance, measured in
/// radians, between two consecutive spider arms. If this parameter is equal
/// to 0, the spider net mask will be disabled. It is disabled by default.
/// * **spiderOffsetAngle**: It defines the angle at which the first arm
/// is located. The default is 0, being the first arm located at the
/// positive y-axis. It cannot take a negative value.
/// * **spiderWidth**: The width of each specific spider arm. Measured in
/// radians. Default is 2.5 degrees.
///
/// The number of arms will be determined by those parameters.
///
/// The following image is generated as a validation or a way to visualize the
/// TRestAxionOptics::GetEntranceRing method. Each color represents a particle
/// hitting in a different ring. The position is drawn at both, the generator
/// plane and the optics entrance plane. This creates an effect of diffusion at
/// the generator plane since the generator random direction is slightly tilted
/// respect to the optical axis.
///
/// \htmlonly <style>div.image img[src="opticsBasic.png"]{width:750px;}</style> \endhtmlonly
///
/// ![Basic optics validation for method TRestAxionOptics::GetEntranceRing](opticsBasic.png)
///
/// This image was generated using the pipeline/metadata/optics/basic.py script.
/// And the rings description can be found at the corresponding basic.rml file.
///
/// This class is an abstract class, to see an example of its implementation
/// inside a RML configuration file, check for TRestAxionGenericOptics or
/// TRestAxionMCPLOptics.
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2022-February: First concept and implementation of TRestAxionOptics class.
///            	  Javier Galan
///
/// \class      TRestAxionOptics
/// \author     Javier Galan <javier.galan@unizar.es>
///
/// <hr>
///

#include "TRestAxionOptics.h"

using namespace std;

#include "TRestPhysics.h"
using namespace REST_Physics;

ClassImp(TRestAxionOptics);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestAxionOptics::TRestAxionOptics() : TRestMetadata() { Initialize(); }

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
///
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or
/// relative.
///
/// The default behaviour is that the config file must be specified with
/// full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the
/// corresponding TRestAxionOptics section inside the RML.
///
TRestAxionOptics::TRestAxionOptics(const char* cfgFileName, string name) : TRestMetadata(cfgFileName) {
    RESTDebug << "Entering TRestAxionOptics constructor( cfgFileName, name )" << RESTendl;
    RESTDebug << "File: " << cfgFileName << " Name: " << name << RESTendl;
}

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestAxionOptics::~TRestAxionOptics() {
    if (fEntranceMask) delete fEntranceMask;
    if (fExitMask) delete fExitMask;
    if (fMiddleMask) delete fMiddleMask;
}

///////////////////////////////////////////////
/// \brief Initialization of TRestAxionOptics members
///
void TRestAxionOptics::Initialize() {
    std::cout << "Entering TRestAxionOptics::Initialize" << std::endl;
    SetLibraryVersion(LIBRARY_VERSION);

    if (fEntranceMask != nullptr) {
        delete fEntranceMask;
        fEntranceMask = nullptr;
    }
    fEntranceMask = new TRestCombinedMask();
    fEntranceMask->SetName("Entrance");
    fEntranceMask->SetTitle("Optics entrance mask");

    if (fExitMask != nullptr) {
        delete fExitMask;
        fExitMask = nullptr;
    }
    fExitMask = new TRestCombinedMask();
    fExitMask->SetName("Exit");
    fExitMask->SetTitle("Optics exit mask");

    if (fMiddleMask != nullptr) {
        delete fMiddleMask;
        fMiddleMask = nullptr;
    }
    fMiddleMask = new TRestCombinedMask();
    fMiddleMask->SetName("Middle");
    fMiddleMask->SetTitle("Optics middle mask");
}

///////////////////////////////////////////////
/// \brief Initialization of TRestAxionOptics field members through a RML file
///
void TRestAxionOptics::InitFromConfigFile() {
    TRestMetadata::InitFromConfigFile();

    /*
std::vector<Double_t> rMax = StringToElements(GetParameter("ringMaxRadii", "-1"), ",");
std::vector<Double_t> rMin = StringToElements(GetParameter("ringMinRadii", "-1"), ",");

if (rMax.size() != rMin.size())
    SetError(
        "TRestAxionOptics. The number of ring radii definitions do not match! Rings will not be "
        "initialized!");
else {
    fRingsRadii.clear();
    for (unsigned int n = 0; n < rMax.size(); n++) {
        std::pair<Double_t, Double_t> p(rMin[n], rMax[n]);
        fRingsRadii.push_back(p);
    }
}
    */

    // If we recover the metadata class from ROOT file we will need to call Initialize ourselves
    this->Initialize();
}

///////////////////////////////////////////////
/// \brief Prints on screen the information about the metadata members of TRestAxionOptics
///
void TRestAxionOptics::PrintMetadata() {
    TRestMetadata::PrintMetadata();

    RESTMetadata << "Entrance position in Z : " << GetEntranceZPosition() << " mm" << RESTendl;
    RESTMetadata << "Exit position in Z : " << GetExitZPosition() << " mm" << RESTendl;
}

void TRestAxionOptics::PrintMasks() {
    if (fEntranceMask) fEntranceMask->PrintMetadata();
    if (fMiddleMask) fMiddleMask->PrintMetadata();
    if (fExitMask) fExitMask->PrintMetadata();
}
