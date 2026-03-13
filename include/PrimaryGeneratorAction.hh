//============================================================================
// PrimaryGeneratorAction.hh
//
// Custom cosmic ray generator — fully in C++, no macro required.
//
// Angular distribution : I(θ) ∝ cos²(θ)  (rejection sampling)
// Azimuth φ            : uniform [0, 2π)
// Source surface       : hemisphere of radius R centred on detector,
//                        so particles always aim toward the detector
//                        regardless of incidence angle.
// Species mix (sea level):
//   μ⁻  50%,  μ⁺  25%,  γ  10%,  e⁻  8%,  e⁺  2%,  p  5%
// Energy spectra       : power-law per species
//
// UI commands:
//   /CosmicGen/sourceRadius  <value cm>
//   /CosmicGen/verbose       true|false
//============================================================================
#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1

#include "G4VUserPrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4UImessenger.hh"
#include "G4UIdirectory.hh"
#include "G4UIcmdWithADouble.hh"
#include "G4UIcmdWithABool.hh"
#include "globals.hh"
#include <array>

class G4Event;

struct CosmicSpecies {
  G4String name;      // Geant4 particle name
  G4double fraction;  // unnormalised weight
  G4double eMin;      // min kinetic energy (GeV, converted on use)
  G4double eMax;      // max kinetic energy (GeV)
  G4double gamma;     // spectral index (dN/dE ∝ E^{-gamma})
};

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction,
                               public G4UImessenger
{
public:
  PrimaryGeneratorAction();
  virtual ~PrimaryGeneratorAction();

  virtual void GeneratePrimaries(G4Event* event);
  virtual void SetNewValue(G4UIcommand* cmd, G4String val);

private:
  G4double      SampleCosSquaredTheta() const;
  G4double      SamplePowerLaw(G4double eMin, G4double eMax, G4double gam) const;
  G4int         SampleSpecies() const;
  G4ThreeVector SampleSourcePoint(G4double theta, G4double phi) const;

  G4ParticleGun* fGun;
  G4double       fSourceRadius;

  static constexpr int kNSpecies = 6;
  std::array<CosmicSpecies, kNSpecies> fSpecies;
  std::array<G4double,      kNSpecies> fCDF;

  G4UIdirectory*      fDir;
  G4UIcmdWithADouble* fCmdRadius;
  G4UIcmdWithABool*   fCmdVerbose;
  G4bool              fVerbose;
};

#endif
