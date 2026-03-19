//============================================================================
// PMTSD.cc
//
// Photon collection model:
//   1. Optical photon arrives at the PMT volume boundary (detected by
//      G4OpticalBoundaryProcess when BoundaryInvokeSD=true).
//   2. Its energy is interpolated against the bialkali QE curve.
//   3. A uniform random number decides whether a photoelectron is produced.
//   4. Both raw hits and PE hits are recorded in PMTHit objects.
//   5. The photon is killed regardless (surface efficiency=1 in surface def,
//      quantum efficiency handled here in the SD).
//
// Bialkali QE curve (typical R7600 / H3164 series):
//   λ(nm)  E(eV)   QE(%)
//   700    1.77    1
//   600    2.07    8
//   500    2.48   18
//   460    2.70   22   ← peak
//   420    2.95   20
//   380    3.26   12
//   340    3.65    4
//============================================================================
#include "PMTSD.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4HCofThisEvent.hh"
#include "G4SDManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"
#include "G4ios.hh"
#include <algorithm>

// QE table definition
const std::array<G4double, PMTSD::kQENodes> PMTSD::kQE_E = {
  1.77*eV, 2.07*eV, 2.48*eV, 2.70*eV, 2.95*eV, 3.26*eV, 3.65*eV
};

const std::array<G4double, PMTSD::kQENodes> PMTSD::kQE_QE = {
  0.01,    0.08,    0.18,    0.22,    0.20,    0.12,    0.04
};

//----------------------------------------------------------------------------
PMTSD::PMTSD(const G4String &name, const G4String &hcName)
  : G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
  collectionName.insert(hcName);
}

PMTSD::~PMTSD() {}

//----------------------------------------------------------------------------
void PMTSD::Initialize(G4HCofThisEvent *hce)
{
  fHitsCollection = new PMTHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int id = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(id, fHitsCollection);
}

//----------------------------------------------------------------------------
G4double PMTSD::GetQE(G4double energy) const
{
  // Below/above table range → 0
  if (energy <= kQE_E[0])
    return 0.;
  if (energy >= kQE_E[kQENodes - 1])
    return 0.;

  // Linear interpolation
  for (int i = 0; i < kQENodes - 1; ++i) {
    if (energy >= kQE_E[i] && energy < kQE_E[i+1]) {
      G4double t = (energy - kQE_E[i]) / (kQE_E[i+1] - kQE_E[i]);
      return kQE_QE[i] + t * (kQE_QE[i+1] - kQE_QE[i]);
    }
  }
  return 0.;
}

//----------------------------------------------------------------------------
G4bool PMTSD::ProcessHits(G4Step *step, G4TouchableHistory*)
{
  // Only optical photons
  if (step->GetTrack()->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
    return false;

  G4double energy = step->GetTrack()->GetTotalEnergy();
  G4double qe     = GetQE(energy);
  G4bool   isPE   = (G4UniformRand() < qe);

  auto *hit = new PMTHit();
  hit->SetPhotonEnergy(energy);
  hit->SetTime(step->GetPostStepPoint()->GetGlobalTime());
  hit->SetIsPhotoelectron(isPE);
  fHitsCollection->insert(hit);

  // Kill photon — it has been absorbed by the cathode
  step->GetTrack()->SetTrackStatus(fStopAndKill);
  return true;
}

//----------------------------------------------------------------------------
void PMTSD::EndOfEvent(G4HCofThisEvent*)
{
  if (verboseLevel > 1 && fHitsCollection) {
    G4int nRaw = fHitsCollection->entries();
    G4int nPE  = 0;
    for (G4int i = 0; i < nRaw; ++i)
      if ((*fHitsCollection)[i]->IsPhotoelectron())
        ++nPE;
    G4cout << "  PMTSD: " << nRaw << " photons → " << nPE << " PE" << G4endl;
  }
}
