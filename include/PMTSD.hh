//============================================================================
// PMTSD.hh
// Detects optical photons at the PMT photocathode.
// Applies bialkali QE curve via a per-photon Monte Carlo dice roll.
// Reports both raw photon count and photoelectron (PE) count.
//============================================================================
#ifndef PMTSD_h
#define PMTSD_h 1

#include "G4VSensitiveDetector.hh"
#include "PMTHit.hh"
#include <array>

class G4Step;
class G4HCofThisEvent;

class PMTSD : public G4VSensitiveDetector
{
public:
  PMTSD(const G4String &name, const G4String &hcName);
  virtual ~PMTSD();

  virtual void   Initialize(G4HCofThisEvent *hce);
  virtual G4bool ProcessHits(G4Step *step, G4TouchableHistory*);
  virtual void   EndOfEvent(G4HCofThisEvent *hce);

private:
  // Interpolate QE at a given photon energy
  G4double GetQE(G4double energy) const;

  PMTHitsCollection *fHitsCollection;

  // Bialkali QE table (energy in eV, QE in [0,1])
  // 7 nodes spanning 340–700 nm
  static constexpr int kQENodes = 7;
  static const std::array<G4double, kQENodes> kQE_E;   // eV
  static const std::array<G4double, kQENodes> kQE_QE;  // fraction
};

#endif
