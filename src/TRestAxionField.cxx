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
////
/// TRestAxionField is a class used to calculate the axion-photon mixing
/// and determine the probability of the particle being in a photon
/// state after propagating inside a magnetic field.
///
/// The class allows to assign a buffer gas using a TRestAxionBufferGas and
/// a magnetic field map loaded through TRestAxionMagneticField. If these
/// objects have been assigned, the methods implemented in this class
/// may consider those - if needed - inside the calculation.
///
/// In practice this class provides different versions of the method
/// GammaTransmissionProbability that allows to calculate the axion-photon
/// probability using different strategies.
///
/// ## Calculating axion-photon probability in a constant field.
///
/// ### 1. In vacuum
///
/// For calculations inside a constant magnetic field one may simply
/// invoke the following code which will launch the calculation in *vacuum*,
/// for a field of 2T, a coherence length of 10000mm, and an axion energy
/// and mass of 4.2keV and 0.1eV, respectively.
///
/// \code
/// TRestAxionField field;
/// field->GammaTransmissionProbability( 2, 10000, 4.2, 0.1 );
/// \endcode
///
/// It is possible to reduce the number of arguments we are giving to this
/// function by assigning few data member values present inside TRestAxionField
/// as follows:
///
/// \code
/// TRestAxionField field;
/// field.SetMagneticField(2);
/// field.SetCoherenceLength(10000);
/// field.SetAxionEnergy(4.2);
/// field.GammaTransmissionProbability( 0.1 );
/// \endcode
///
/// Indeed, the complete version with all arguments will also update the
/// data member values, so that next calls to this method will use the same
/// magnetic field, coherence length and energy. In the following example
/// the second call to GammaTransmissionProbability will use again the same
/// values for magnetic field, coherence length and energy from the
/// preceding call.
///
/// \code
/// TRestAxionField field;
/// field.GammaTransmissionProbability( 2, 10000, 4.2, 0.1 );
/// field.GammaTransmissionProbability( 0.01 );
/// \endcode
///
/// ### 2. In a buffer gas medium
///
/// The axion-photon probability can also be calculated in the sin of a
/// gaseous medium. For that we need to assign a buffer gas instance containing
/// the relevant gas properties in the form of a TRestAxionBufferGas.
///
/// The following piece of code will assign the gas properties to the field
/// calculation as defined inside the `bufferGases.rml` official file. The gas
/// provides the absorption and equivalent photon mass as a funtion of the
/// energy.
///
/// \code
/// TRestAxionField field;
/// TRestAxionBufferGas gas("bufferGases.rml", "helium");
/// field.AssignBufferGas( &gas );
/// field.SetMagneticField(2);
/// field.SetCoherenceLength(10000);
/// field.SetAxionEnergy(4.2);
/// field.GammaTransmissionProbability( 0.1 );
/// \endcode
///
/// Once we have assigned a buffer gas to the class we may return back to the
/// vacuum state by just assigning a nullptr to the gas.
///
/// \code
/// field.AssignBufferGas(nullptr);
/// \endcode
///
/// ## Calculating axion-photon probability in an unhomogeneous field.
///
/// There are two main strategies presently implemented inside this class to integrate
/// the axion field along an unhomogeneous magnetic field. The first one uses a simple
/// trapezoidal integration method with the advantage that the computing time is controlled
/// by length of the integration step, but no error calculation is given. The second
/// method advantage is that in principle it allows to fix the accuracy of our calculation
/// and an error of the integration is returned by the GSL integration method used. The
/// main disadvantage is that by fixing the accuracy we lose control on the computation
/// time required to reach such accuracy. Here we describe both methods.
///
/// ### 1. Using external field profile
///
/// Using a external field profile requires to prepare the field profile and pass it
/// along to TRestAxionField::GammaTransmissionProbability method performing the
/// integration. The profile will be placed in a `std::vector<Double_t>` that describes
/// the magnetic field profile with a fixed length step.
/// The method TRestAxionMagneticField::GetTransversalComponentAlongPath can be used
/// to generate such profile and calculate the field using the first method, as in
/// the following example:
///
/// \code
/// Double_t deltaL = 100; // mm
/// Double_t Ea = 4.2; // keV
/// Double_t ma = 0.001; // eV
///
/// // Recovering the B-field profile along the axis
/// TRestAxionMagneticField mField("fields.rml", "babyIAXO_2024");
/// std::vector<Double_t> bField = mField.GetTransversalComponentAlongPath( TVector3(0,0,-6000),
/// TVector3(0,0,6000), deltaL );
///
/// TRestAxionField aField;
/// aField.GammaTransmissionProbability( bField, deltaL, Ea, ma );
/// \endcode
///
/// In a similar way to the constant field case, it would suffice to assign a buffer gas to
/// the TRestAxionField instance to have the calculation performed in a buffer gas.
///
/// ### 2. Using a field map instance
///
/// A new method has been added in which the integration method is given a pointer to the
/// function that will allow evaluation of the magnetic field along the B-profile
/// parameterized track. Thus, it is the integration method the one requesting the points
/// on the parametric curve that will be necessary to perform the integration. The required
/// field evaluation is encoded inside TRestAxionField::Integrand which accesses the
/// magnetic field instance TRestAxionField::fMagneticField in order to recover a parametric
/// track along the path.
///
/// The method name using this technique requires thus direct access to a
/// TRestAxionMagneticField instance that must be assigned beforehand. For computing
/// the integral following this strategy one needs to write the following code where we
/// consider the most generic case including a gas buffer medium.
///
/// \code
/// TRestAxionField aField;
///
/// TRestAxionBufferGas gas("bufferGases.rml", "helium");
/// aField.AssignBufferGas( &gas );
///
/// TRestAxionMagneticField mField("fields.rml", "babyIAXO_2024");
/// aField.AssignMagneticField( &mField );
///
/// Double_t ma = 0.001;
/// Double_t Ea = 4.2;
/// Double_t accuracy = 0.1;
/// // The track that will be integrated must be predefined before inside the magnetic field class
/// mField.SetTrack( TVector3(0,0,-6000), TVector3(0,0,1) );
/// std::pair<Double_t, Double_t> prob = aField.GammaTransmissionFieldMapProbability(Ea, ma, accuracy);
/// \endcode
///
/// Where the later call will return a `std::pair` with the probability and its
/// corresponding error.
///
/// ## Determining density steps for continuous scanning
///
/// A method may be used to determine the masses and gas densities to achieve
/// a continuous scanning TRestAxionField::GetMassDensityScanning. This method
/// will place a new mass or gas density setting at FWHM/2 till it reaches
/// the maximum axion mass specified in the argument.
///
/// The following code recovers the axion masses and density settings required
/// for a continuous scan till 0.2eV.
///
/// \code
/// std::pair <Double_t,Double_t> aField.GetMassDensityScanning( "He", 0.2 );
/// \endcode
///
///--------------------------------------------------------------------------
///
/// RESTsoft - Software for Rare Event Searches with TPCs
///
/// History of developments:
///
/// 2019-March: First concept and implementation of TRestAxionField class.
///             Javier Galan
/// 2023-December: Implementing methods to recover the axion mass scanning nodes
///             Fran Candon
///
/// \class      TRestAxionField
/// \author     Javier Galan
/// \author     Fran Candon
///
/// <hr>
///
#include "TRestAxionField.h"

#include <TComplex.h>
#include <TVectorD.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>

#include <numeric>

#include "TH1F.h"

using namespace std;

ClassImp(TRestAxionField);

///////////////////////////////////////////////
/// \brief Default constructor
///
TRestAxionField::TRestAxionField() { Initialize(); }

///////////////////////////////////////////////
/// \brief Default destructor
///
TRestAxionField::~TRestAxionField() {}

///////////////////////////////////////////////
/// \brief Initialization of TRestAxionField class
///
void TRestAxionField::Initialize() {
    fBufferGas = NULL;

    /// MOVED TO TRestAxionFieldPropagationProcess class
    /// faxion = SetComplexReal(1, 0);
    /// fAem = SetComplexReal(0, 0);
}

///////////////////////////////////////////////
/// \brief Performs the calculation of (BL) factor in natural units.
///
/// `Lcoh` should be expressed in `mm`, and `Bmag` in `T`.
/// The result will be given for an axion-photon coupling of 10^{-10} GeV^{-1}
///
double TRestAxionField::BL(Double_t Bmag, Double_t Lcoh) {
    Double_t lengthInMeters = Lcoh * units("m");

    Double_t tm =
        REST_Physics::lightSpeed / REST_Physics::naturalElectron * units("GeV") / units("eV");  // eV --> GeV
    Double_t sol = lengthInMeters * Bmag * tm;
    sol = sol * 1.0e-10;

    return sol;
}

///////////////////////////////////////////////
/// \brief Performs the calculation of (BL/2)^2 factor in natural units.
///
/// `Lcoh` should be expressed in `mm`, and `Bmag` in `T`.
/// The result will be given for an axion-photon coupling of 10^{-10} GeV^{-1}
///
double TRestAxionField::BLHalfSquared(Double_t Bmag, Double_t Lcoh)  // (BL/2)**2
{
    Double_t lengthInMeters = Lcoh * units("m");

    Double_t tm =
        REST_Physics::lightSpeed / REST_Physics::naturalElectron * units("GeV") / units("eV");  // eV --> GeV
    Double_t sol = lengthInMeters * Bmag * tm / 2;
    sol = sol * sol * 1.0e-20;

    return sol;
}

///////////////////////////////////////////////
/// \brief Performs the calculation of axion-photon conversion probability using directly
/// equation (11) from van Bibber, Phys Rev D Part Fields. 1989.
///
/// If m_gamma (mg) is not given as an argument, i.e. it is equal to zero, then m_gamma
/// will be obtainned from the buffer gas definition. If no buffer gas has been assigned
/// then the medium will be assumed to be vacuum.
///
/// Ea in keV, ma in eV, mgamma in eV, Lcoh in mm, Bmag in T
///
/// mg in eV, absLength in cm-1
///
/// The returned value is given for g_ag = 10^-10 GeV-1
///
Double_t TRestAxionField::GammaTransmissionProbability(Double_t ma, Double_t mg, Double_t absLength) {
    Double_t cohLength = fLcoh * units("m");  // Default REST units are mm;

    Double_t photonMass = mg;

    if (mg == 0 && fBufferGas) photonMass = fBufferGas->GetPhotonMass(fEa);

    RESTDebug << "+--------------------------------------------------------------------------+" << RESTendl;
    RESTDebug << " TRestAxionField::GammaTransmissionProbability. Parameter summary" << RESTendl;
    RESTDebug << " Photon mass : " << photonMass << " eV" << RESTendl;
    RESTDebug << " Axion mass : " << ma << " eV" << RESTendl;
    RESTDebug << " Axion energy : " << fEa << " keV" << RESTendl;
    RESTDebug << " Lcoh : " << fLcoh << " mm" << RESTendl;
    RESTDebug << " Bmag : " << fBmag << " T" << RESTendl;
    RESTDebug << "+--------------------------------------------------------------------------+" << RESTendl;

    if (ma == 0.0 && photonMass == 0.0) return BLHalfSquared(fBmag, fLcoh);

    Double_t q = (ma * ma - photonMass * photonMass) / 2. / (fEa * units("eV"));
    Double_t l = cohLength * REST_Physics::PhMeterIneV;
    Double_t phi = q * l;

    Double_t Gamma = absLength;
    if (absLength == 0 && fBufferGas) Gamma = fBufferGas->GetPhotonAbsorptionLength(fEa);  // cm-1
    Double_t GammaL = Gamma * cohLength * units("cm") / units("m");                        // m --> cm

    if (fDebug) {
        std::cout << "+------------------------+" << std::endl;
        std::cout << " Intermediate calculations" << std::endl;
        std::cout << " q : " << q << " eV" << std::endl;
        std::cout << " l : " << l << " eV-1" << std::endl;
        std::cout << " phi : " << phi << std::endl;
        std::cout << "Gamma : " << Gamma << std::endl;
        std::cout << "GammaL : " << GammaL << std::endl;
        std::cout << "+------------------------+" << std::endl;
    }

    Double_t MFactor = phi * phi + GammaL * GammaL / 4.0;
    MFactor = 1.0 / MFactor;

    if (fDebug) {
        std::cout << "Mfactor : " << MFactor << std::endl;
        std::cout << "(BL/2)^2 : " << BLHalfSquared(fBmag, fLcoh) << std::endl;
        std::cout << "cos(phi) : " << cos(phi) << std::endl;
        std::cout << "Exp(-GammaL) : " << exp(-GammaL) << std::endl;
    }

    double sol = (double)(MFactor * BLHalfSquared(fBmag, fLcoh) *
                          (1 + exp(-GammaL) - 2 * exp(-GammaL / 2) * cos(phi)));

    if (fDebug) std::cout << "Axion-photon transmission probability : " << sol << std::endl;

    return sol;
}

///////////////////////////////////////////////
/// \brief On top of calculating the gamma transmission probability it will assign new values
/// for the magnetic field (Bmag/T), coherence length (Lcoh/mm) and axion energy (Ea/keV).
///
Double_t TRestAxionField::GammaTransmissionProbability(Double_t Bmag, Double_t Lcoh, Double_t Ea, Double_t ma,
                                                       Double_t mg, Double_t absLength) {
    fBmag = Bmag;
    fLcoh = Lcoh;
    fEa = Ea;

    return GammaTransmissionProbability(ma, mg, absLength);
}

///////////////////////////////////////////////
/// \brief Performs the calculation of axion-photon conversion probability using directly
/// equation (28) from J. Redondo and A. Ringwald, Light shinning through walls.
/// https://arxiv.org/pdf/1011.3741.pdf
///
/// If m_gamma (mg) is not given as an argument, i.e. it is equal to zero, then m_gamma
/// will be obtainned from the buffer gas definition. If no buffer gas has been assigned
/// then the medium will be assumed to be vacuum.
///
/// Ea in keV, ma in eV, mgamma in eV, deltaL in mm, Bmag in T
///
/// mg in eV, absLength in cm-1
///
/// The returned value is given for g_ag = 10^-10 GeV-1
///
/// \note The density is for the moment homogeneous. We would need to implemnent a double integral
/// to solve the problem with a density profile. TOBE implemented in a new method if needed, where
/// Gamma is not constant and \integral{q(z)} is integrated at each step.
///
Double_t TRestAxionField::GammaTransmissionProbability(std::vector<Double_t> Bmag, Double_t deltaL,
                                                       Double_t Ea, Double_t ma, Double_t mg,
                                                       Double_t absLength) {
    // Default REST units are mm. We express cohLength in m.
    Double_t Lcoh = (Bmag.size() - 1) * deltaL;  // in mm
    Double_t cohLength = Lcoh * units("m");      // in m

    Double_t photonMass = mg;

    if (mg == 0 && fBufferGas) photonMass = fBufferGas->GetPhotonMass(Ea);

    Double_t fieldAverage = 0;
    if (Bmag.size() > 0) fieldAverage = std::accumulate(Bmag.begin(), Bmag.end(), 0.0) / Bmag.size();

    if (fDebug) {
        std::cout << "+--------------------------------------------------------------------------+"
                  << std::endl;
        std::cout << " TRestAxionField::GammaTransmissionProbability. Parameter summary" << std::endl;
        std::cout << " Photon mass : " << photonMass << " eV" << std::endl;
        std::cout << " Axion mass : " << ma << " eV" << std::endl;
        std::cout << " Axion energy : " << Ea << " keV" << std::endl;
        std::cout << " Lcoh : " << cohLength << " m" << std::endl;
        std::cout << " Bmag average : " << fieldAverage << " T" << std::endl;
        std::cout << "+--------------------------------------------------------------------------+"
                  << std::endl;
    }

    // In vacuum
    if (ma == 0.0 && photonMass == 0.0) return BLHalfSquared(fieldAverage, Lcoh);

    Double_t q = (ma * ma - photonMass * photonMass) / 2. / (Ea * units("eV"));
    Double_t l = cohLength * REST_Physics::PhMeterIneV;
    Double_t phi = q * l;

    Double_t Gamma = absLength;
    if (absLength == 0 && fBufferGas) Gamma = fBufferGas->GetPhotonAbsorptionLength(Ea);  // cm-1
    Double_t GammaL = Gamma * cohLength * units("cm") / units("m");                       // m --> cm

    if (fDebug) {
        std::cout << "+------------------------+" << std::endl;
        std::cout << " Intermediate calculations" << std::endl;
        std::cout << " q : " << q << " eV" << std::endl;
        std::cout << " l : " << l << " eV-1" << std::endl;
        std::cout << " phi : " << phi << std::endl;
        std::cout << "Gamma : " << Gamma << std::endl;
        std::cout << "GammaL : " << GammaL << std::endl;
        std::cout << "+------------------------+" << std::endl;
    }

    Double_t MFactor = phi * phi + GammaL * GammaL / 4.0;
    MFactor = 1.0 / MFactor;

    if (fDebug) {
        std::cout << "Mfactor : " << MFactor << std::endl;
        std::cout << "(BL/2)^2 : " << BLHalfSquared(fieldAverage, Lcoh) << std::endl;
        std::cout << "cos(phi) : " << cos(phi) << std::endl;
        std::cout << "Exp(-GammaL) : " << exp(-GammaL) << std::endl;
    }

    /// We integrate following the Midpoint rule method. (Other potential options : Trapezoidal, Simpsons)
    Double_t deltaIneV = deltaL * units("m") * REST_Physics::PhMeterIneV;
    TComplex sum(0, 0);
    for (unsigned int n = 0; n < Bmag.size() - 1; n++) {
        Double_t Bmiddle = 0.5 * (Bmag[n] + Bmag[n + 1]);

        Double_t lStepIneV = ((double)n + 0.5) * deltaIneV;
        Double_t lStepInCm = ((double)n + 0.5) * deltaL * units("cm");

        TComplex qCgC(0.5 * Gamma * lStepInCm, -q * lStepIneV);
        qCgC = TComplex::Exp(qCgC);

        TComplex integrand = Bmiddle * deltaL * qCgC;  // The integrand is in T by mm

        sum += integrand;
    }

    Double_t sol = exp(-GammaL) * sum.Rho2() * BLHalfSquared(1, 1);
    // Now T and mm have been recalculated in natural units using BLHalfSquared(1,1).

    if (fDebug) std::cout << "Axion-photon transmission probability : " << sol << std::endl;

    return (Double_t)sol;
}

///////////////////////////////////////////////
/// \brief Performs the calculation of axion-photon conversion probability using directly
/// equation (28) from J. Redondo and A. Ringwald, Light shinning through walls.
/// https://arxiv.org/pdf/1011.3741.pdf
///
/// m_gamma will be obtainned from the buffer gas definition. If no buffer gas has been
/// assigned then the medium will be assumed to be vacuum.
///
/// Ea in keV, ma in eV
///
/// The mgamma and absorption lengths are the ones defined by the gas.
///
/// The returned value is given for g_ag = 10^-10 GeV-1
///
/// \note The density is for the moment homogeneous. We would need to implemnent a double integral
/// to solve the problem with a density profile.
///
std::pair<Double_t, Double_t> TRestAxionField::GammaTransmissionFieldMapProbability(Double_t Ea, Double_t ma,
                                                                                    Double_t accuracy,
                                                                                    Int_t num_intervals,
                                                                                    Int_t qawo_levels) {
    gsl_set_error_handler_off();

    if (!fMagneticField) {
        RESTError << "TRestAxionField::GammaTransmissionFieldMapProbability requires a magnetic field map!"
                  << RESTendl;
        RESTError << "Use TRestAxionField::AssignMagneticField method to assign one" << RESTendl;
        return {0.0, 0.0};
    }

    if (fMagneticField->GetTrackLength() <= 0) return {0.0, 0.0};

    double photonMass = 0;  // Vacuum
    if (fBufferGas) photonMass = fBufferGas->GetPhotonMass(Ea);

    if (fDebug) {
        std::cout << "+--------------------------------------------------------------------------+"
                  << std::endl;
        std::cout << " TRestAxionField::GammaTransmissionProbability. Parameter summary" << std::endl;
        std::cout << " Photon mass : " << photonMass << " eV" << std::endl;
        std::cout << " Axion mass : " << ma << " eV" << std::endl;
        std::cout << " Axion energy : " << Ea << " keV" << std::endl;
        std::cout << "+--------------------------------------------------------------------------+"
                  << std::endl;
    }

    double q = (ma * ma - photonMass * photonMass) / 2. / (Ea * units("eV"));
    q = q * REST_Physics::PhMeterIneV * units("m") / units("mm");  // mm-1

    double Gamma = 0;
    if (fBufferGas) Gamma = fBufferGas->GetPhotonAbsorptionLength(Ea) * units("cm") / units("mm");  // mm-1

    if (fDebug) {
        std::cout << "+------------------------+" << std::endl;
        std::cout << " Intermediate calculations" << std::endl;
        std::cout << " q : " << q << " eV" << std::endl;
        std::cout << "Gamma : " << Gamma << std::endl;
        std::cout << "+------------------------+" << std::endl;
    }

    if (q == 0)
        return ComputeResonanceIntegral(Gamma, accuracy, num_intervals);
    else
        return ComputeOffResonanceIntegral(q, Gamma, accuracy, num_intervals, qawo_levels);

    return {0.0, 0.0};
}

///////////////////////////////////////////////
/// \brief Performs the calculation of axion-photon conversion probability using directly
/// equation (28) from J. Redondo and A. Ringwald, Light shinning through walls.
/// https://arxiv.org/pdf/1011.3741.pdf
///
/// It integrates the Integrand function defined on the header of TRestAxionField.
///
/// This method uses the GSL QAG integration method. We use this method when the cosine
/// function vanishes when q = 0.
///
/// See https://www.gnu.org/software/gsl/doc/html/integration.html for more details on
/// the integration parameters.
///
/// The Gamma function should be expressed in mm-1 since the field map accessed in the integrand
/// is evaluated using mm.
///
std::pair<Double_t, Double_t> TRestAxionField::ComputeResonanceIntegral(Double_t Gamma, Double_t accuracy,
                                                                        Int_t num_intervals) {
    double reprob, rerr;

    std::pair<TRestAxionMagneticField*, double> params = {fMagneticField, Gamma};

    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(num_intervals);

    gsl_function F;
    F.function = &Integrand;
    F.params = &params;

    auto start = std::chrono::system_clock::now();

    int status = gsl_integration_qag(&F, 0, fMagneticField->GetTrackLength(), accuracy, accuracy,
                                     num_intervals, GSL_INTEG_GAUSS61, workspace, &reprob, &rerr);

    if (status > 0) return {0, status};

    auto end = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double GammaL = Gamma * fMagneticField->GetTrackLength();
    double C = exp(-GammaL) * BLHalfSquared(1, 1);

    double prob = C * reprob * reprob;
    double proberr = 2 * C * reprob * rerr;

    if (fDebug) {
        std::cout << " ---- TRestAxionField::ComputeResonanceIntegral (QAG) ----" << std::endl;
        std::cout << "Gamma: " << Gamma << " mm-1" << std::endl;
        std::cout << "accuracy: " << accuracy << std::endl;
        std::cout << "num_intervals: " << num_intervals << std::endl;
        std::cout << " -------" << std::endl;
        std::cout << "Probability: " << prob << std::endl;
        std::cout << "Probability error: " << proberr << std::endl;
        std::cout << "Computing time: " << seconds.count() << std::endl;
        std::cout << " -------" << std::endl;
    }

    std::pair<Double_t, Double_t> sol = {prob, proberr};
    return sol;
}

///////////////////////////////////////////////
/// \brief Performs the calculation of axion-photon conversion probability using directly
/// equation (28) from J. Redondo and A. Ringwald, Light shinning through walls.
/// https://arxiv.org/pdf/1011.3741.pdf
///
/// It integrates the Integrand function defined on the header of TRestAxionField.
///
/// This method uses the GSL QAWO method for oscillatory functions. That is the case when
/// the q is not zero, in the off-resonance case.
///
/// See https://www.gnu.org/software/gsl/doc/html/integration.html for more details on
/// the integration parameters.
///
/// The Gamma function should be expressed in mm-1 since the field map accessed in the integrand
/// is evaluated using mm.
///
std::pair<Double_t, Double_t> TRestAxionField::ComputeOffResonanceIntegral(Double_t q, Double_t Gamma,
                                                                           Double_t accuracy,
                                                                           Int_t num_intervals,
                                                                           Int_t qawo_levels) {
    double reprob, rerr;
    double improb, imerr;

    std::pair<TRestAxionMagneticField*, double> params = {fMagneticField, Gamma};

    gsl_integration_workspace* workspace = gsl_integration_workspace_alloc(num_intervals);

    gsl_function F;
    F.function = &Integrand;
    F.params = &params;

    auto start = std::chrono::system_clock::now();

    gsl_integration_qawo_table* wf =
        gsl_integration_qawo_table_alloc(q, fMagneticField->GetTrackLength(), GSL_INTEG_COSINE, qawo_levels);
    int status =
        gsl_integration_qawo(&F, 0, accuracy, accuracy, num_intervals, workspace, wf, &reprob, &rerr);
    if (status > 0) {
        gsl_integration_qawo_table_free(wf);
        return {0, status};
    }

    gsl_integration_qawo_table_set(wf, q, fMagneticField->GetTrackLength(), GSL_INTEG_SINE);
    status = gsl_integration_qawo(&F, 0, accuracy, accuracy, num_intervals, workspace, wf, &improb, &imerr);
    if (status > 0) {
        gsl_integration_qawo_table_free(wf);
        return {0, status};
    }

    auto end = std::chrono::system_clock::now();
    auto seconds = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    double GammaL = Gamma * fMagneticField->GetTrackLength();
    double C = exp(-GammaL) * BLHalfSquared(1, 1);

    double prob = C * (reprob * reprob + improb * improb);
    double proberr = 2 * C * TMath::Sqrt(reprob * reprob * rerr * rerr + improb * improb * imerr * imerr);

    if (fDebug) {
        std::cout << " ---- TRestAxionField::ComputeOffResonanceIntegral (QAWO) ----" << std::endl;
        std::cout << "Gamma: " << Gamma << " mm-1" << std::endl;
        std::cout << "q: " << q << "mm-1" << std::endl;
        std::cout << "accuracy: " << accuracy << std::endl;
        std::cout << "num_intervals: " << num_intervals << std::endl;
        std::cout << "qawo_levels: " << qawo_levels << std::endl;
        std::cout << " -------" << std::endl;
        std::cout << "Probability: " << prob << std::endl;
        std::cout << "Probability error: " << proberr << std::endl;
        std::cout << "Computing time: " << seconds.count() << std::endl;
        std::cout << " -------" << std::endl;
    }

    std::pair<Double_t, Double_t> sol = {prob, proberr};
    return sol;
}

///////////////////////////////////////////////
/// \brief Performs the calculation of axion-photon absorption probability using directly
/// equation (18) from van Bibber, Phys Rev D Part Fields. 1989.
///
/// If not provided as argument, m_gamma it will be attempted to obtain the value from the buffer gas
/// definition. If no buffer gas has been assigned the medium will be assumed to be vacuum.
///
/// Ea in keV, ma in eV, mgamma in eV, Lcoh in mm, Bmag in T
///
/// mg in eV, absLength in cm-1
///
/// The returned value is given for g_ag = 10^-10 GeV-1
///
Double_t TRestAxionField::AxionAbsorptionProbability(Double_t ma, Double_t mg, Double_t absLength) {
    Double_t cohLength = fLcoh * units("m");  // Default REST units are mm;

    Double_t photonMass = mg;
    if (mg == 0 && fBufferGas) photonMass = fBufferGas->GetPhotonMass(fEa);

    if (fDebug) {
        RESTDebug << "+--------------------------------------------------------------------------+"
                  << RESTendl;
        RESTDebug << " TRestAxionField::AxionAbsorptionProbability. Parameter summary" << RESTendl;
        RESTDebug << " Photon mass : " << photonMass << " eV" << RESTendl;
        RESTDebug << " Axion mass : " << ma << " eV" << RESTendl;
        RESTDebug << " Axion energy : " << fEa << " keV" << RESTendl;
        RESTDebug << " Lcoh : " << fLcoh << " mm" << RESTendl;
        RESTDebug << " Bmag : " << fBmag << " T" << RESTendl;
        RESTDebug << "+--------------------------------------------------------------------------+"
                  << RESTendl;
    }

    if (ma == 0.0 && photonMass == 0.0) return BLHalfSquared(fBmag, fLcoh);

    Double_t q = (ma * ma - photonMass * photonMass) / 2. / (fEa * units("eV"));
    Double_t l = cohLength * REST_Physics::PhMeterIneV;
    Double_t phi = q * l;

    Double_t Gamma = absLength;
    if (absLength == 0 && fBufferGas) Gamma = fBufferGas->GetPhotonAbsorptionLength(fEa);  // cm-1
    Double_t GammaL = Gamma * cohLength * units("cm") / units("m");

    if (fDebug) {
        RESTDebug << "+------------------------+" << RESTendl;
        RESTDebug << " Intermediate calculations" << RESTendl;
        RESTDebug << " q : " << q << " eV" << RESTendl;
        RESTDebug << " l : " << l << " eV-1" << RESTendl;
        RESTDebug << " phi : " << phi << RESTendl;
        RESTDebug << "Gamma : " << Gamma << RESTendl;
        RESTDebug << "GammaL : " << GammaL << RESTendl;
        RESTDebug << "+------------------------+" << RESTendl;
    }

    Double_t MFactor = phi * phi + GammaL * GammaL / 4.0;
    MFactor = 1.0 / MFactor;

    if (fDebug) {
        RESTDebug << "Mfactor : " << MFactor << RESTendl;
        RESTDebug << "(BL/2)^2 : " << BLHalfSquared(fBmag, fLcoh) << RESTendl;
        RESTDebug << "cos(phi) : " << cos(phi) << RESTendl;
        RESTDebug << "Exp(-GammaL) : " << exp(-GammaL) << RESTendl;
    }

    double sol = (double)(MFactor * BLHalfSquared(fBmag, fLcoh) * GammaL);

    if (fDebug) RESTDebug << "Axion-photon absorption probability : " << sol << RESTendl;

    return sol;
}

///////////////////////////////////////////////
/// \brief On top of calculating the axion absorption probability it will assign new values
/// for the magnetic field (Bmag/T), coherence length (Lcoh/mm) and axion energy (Ea/keV).
///
Double_t TRestAxionField::AxionAbsorptionProbability(Double_t Bmag, Double_t Lcoh, Double_t Ea, Double_t ma,
                                                     Double_t mg, Double_t absLength) {
    fBmag = Bmag;
    fLcoh = Lcoh;
    fEa = Ea;

    return AxionAbsorptionProbability(ma, mg, absLength);
}

///////////////////////////////////////////////
/// \brief Performs the calculation of the FWHM for the axion-photon conversion probability
/// computed in `TRestAxionField::GammaTransmissionProbability`.
///
/// Ea in keV, ma in eV, mgamma in eV, deltaL in mm, Bmag in T.
///
/// Default value for the parameters are: Bmag = 2.5 T, Lcoh = 10000 mm, Ea = 4 keV
/// eV,
///
/// IMPORTANT: In the case that the buffer gas is not defined, this method will return the mass at which
/// the probability reaches half of the maximum **vacuum** probability.
///
Double_t TRestAxionField::GammaTransmissionFWHM(Double_t step) {
    Double_t maxMass = 10;  // 10eV is the maximum mass (exit condition)

    Double_t resonanceMass = 0;
    if (fBufferGas) resonanceMass = fBufferGas->GetPhotonMass(fEa);

    /// Scanning towards the right (valid also for vacuum)
    Double_t scanMass = resonanceMass;
    Double_t Pmax = GammaTransmissionProbability(resonanceMass);
    while (Pmax / 2 < GammaTransmissionProbability(scanMass)) {
        scanMass += step;
        if (scanMass > maxMass) {
            RESTError << "TRestAxionField::GammaTransmissionProbability. Something went wrong when "
                         "calculating FWHM"
                      << RESTendl;
            return maxMass;
        }
    }

    Double_t fwhm = scanMass - resonanceMass;
    if (fwhm <= 0) {
        RESTError << "TRestAxionField::GammaTransmissionProbability. Problem calculating FWHM!" << RESTendl;
        fwhm = step;
    }
    return 2 * fwhm;
}

///////////////////////////////////////////////
/// \brief This method determines the proper densities to be used in an axion helioscope experiment in
/// order to achieve a continuous axion mass scan.
///
/// The first scanning density is placed where the axion-photon vacuum probability reaches half the value,
/// `P_ag(max)/2`. Once the first density, or step, has been obtained, the method calculates the FWHM
/// resonance probability for each density/mass and moves the next scanning axion mass by a step of
/// amplitude `FWHM/factor`, where the factor is a value that moves from 2 to 1 as the mass increases, and
/// it falls down with a velocity related with the argument `rampDown`, where the factor follows this
/// formula:
///
///	factor = TMath::Exp( - ma*rampDown  ) + 1;
///
/// The method stops when the axion mass is bigger than the maximum axion mass given as an argument,
/// `maMax`.
///
/// Default arguments: gasName="He", ma_max=0.15 eV, rampDown=5
///
/// \return It returns a vector of pair with the values for the scan, the first one is the axion mass and
/// the second one is the density.
///
/// For additional info see PR: https://github.com/rest-for-physics/axionlib/pull/78
///
std::vector<std::pair<Double_t, Double_t>> TRestAxionField::GetMassDensityScanning(std::string gasName,
                                                                                   double maMax,
                                                                                   double rampDown) {
    std::vector<std::pair<Double_t, Double_t>> massDensityPairs;

    // Storing the gas pointer, if there was one
    TRestAxionBufferGas* previousGas = nullptr;
    if (fBufferGas) {
        previousGas = fBufferGas;
        fBufferGas = nullptr;
    }

    // We are in vacuum now
    double firstMass = GammaTransmissionFWHM() / 2;

    TRestAxionBufferGas gas;
    gas.SetGasDensity(gasName, 0);
    AssignBufferGas(&gas);  // We are in gas now

    Double_t ma = firstMass;
    Double_t density = gas.GetDensityForMass(firstMass, fEa);

    /// Setting mass-density pair for the first step
    massDensityPairs.push_back(std::make_pair(ma, density));

    while (ma < maMax) {
        Double_t factor = TMath::Exp(-ma * rampDown) + 1;
        gas.SetGasDensity(gasName, density);

        ma += GammaTransmissionFWHM() / factor;
        density = gas.GetDensityForMass(ma);

        massDensityPairs.push_back(std::make_pair(ma, density));
    }

    // Recovering back the gas that was defined before calling this method
    fBufferGas = previousGas;

    return massDensityPairs;
}
