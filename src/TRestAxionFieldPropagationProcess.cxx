/*************************************************************************
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
/// TRestAxionFieldPropagationProcess TOBE documented
///
/// The axion is generated with intensity proportional to g_ag = 1.0 x g10
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2019-March:  First implementation of shared memory buffer to rawsignal conversion.
///             Javier Galan
///
/// \class      TRestAxionFieldPropagationProcess
/// \author     Javier Galan
///
/// <hr>
///
#include "TRestAxionFieldPropagationProcess.h"
using namespace std;

ClassImp( TRestAxionFieldPropagationProcess )

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestAxionFieldPropagationProcess::TRestAxionFieldPropagationProcess()
{
    Initialize();
}

///////////////////////////////////////////////
/// \brief Constructor loading data from a config file
/// 
/// If no configuration path is defined using TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or relative.
///
/// The default behaviour is that the config file must be specified with 
/// full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
///
TRestAxionFieldPropagationProcess::TRestAxionFieldPropagationProcess( char *cfgFileName )
{
    Initialize();

    LoadConfig( cfgFileName );
}

///////////////////////////////////////////////
/// \brief Default destructor 
/// 
TRestAxionFieldPropagationProcess::~TRestAxionFieldPropagationProcess()
{
    delete fInputAxionEvent;
    delete fOutputAxionEvent;
}


///////////////////////////////////////////////
/// \brief Function to load the default config in absence of RML input
/// 
void TRestAxionFieldPropagationProcess::LoadDefaultConfig( )
{
    SetName( this->ClassName() );
    SetTitle( "Default config" );
}

///////////////////////////////////////////////
/// \brief Function to load the configuration from an external configuration file.
/// 
/// If no configuration path is defined in TRestMetadata::SetConfigFilePath
/// the path to the config file must be specified using full path, absolute or relative.
///
/// \param cfgFileName A const char* giving the path to an RML file.
/// \param name The name of the specific metadata. It will be used to find the 
/// correspondig TRestGeant4AnalysisProcess section inside the RML.
///
void TRestAxionFieldPropagationProcess::LoadConfig( std::string cfgFilename, std::string name )
{
    if( LoadConfigFromFile( cfgFilename, name ) ) LoadDefaultConfig( );
}

///////////////////////////////////////////////
/// \brief Function to initialize input/output event members and define the section name
/// 
void TRestAxionFieldPropagationProcess::Initialize()
{
    SetSectionName( this->ClassName() );

    fInputAxionEvent = new TRestAxionEvent();
    fOutputAxionEvent = new TRestAxionEvent();

    fInputEvent = fInputAxionEvent;
    fOutputEvent = fOutputAxionEvent;
}

///////////////////////////////////////////////
/// \brief The main processing event function
/// 
TRestEvent* TRestAxionFieldPropagationProcess::ProcessEvent( TRestEvent *evInput )
{
	fInputAxionEvent = (TRestAxionEvent *) evInput;

	*fOutputAxionEvent = *fInputAxionEvent;

	//fOutputAxionEvent->SetGammaProbability( 0.1 );

	if( GetVerboseLevel() >= REST_Debug ) 
	{
		fOutputAxionEvent->PrintEvent();
		GetChar();
	}

    return fOutputEvent;
}

///////////////////////////////////////////////
/// \brief Function reading input parameters from the RML TRestAxionFieldPropagationProcess metadata section
/// 
void TRestAxionFieldPropagationProcess::InitFromConfigFile( )
{
}

