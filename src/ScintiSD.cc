//============================================================================
// ScintiSD.cc
//============================================================================
#include "ScintiSD.hh"
#include "G4HCofThisEvent.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ThreeVector.hh"
#include "G4SDManager.hh"
#include "G4OpticalPhoton.hh"
#include "G4ios.hh"


ScintiSD::ScintiSD(const G4String &name, const G4String &hitsCollectionName)
  : G4VSensitiveDetector(name), fHitsCollection(nullptr)
{
  collectionName.insert(hitsCollectionName);
}

ScintiSD::~ScintiSD() {}

void ScintiSD::Initialize(G4HCofThisEvent *hce)
{
  fHitsCollection = new ScintiHitsCollection(SensitiveDetectorName, collectionName[0]);
  G4int hcID = G4SDManager::GetSDMpointer()->GetCollectionID(collectionName[0]);
  hce->AddHitsCollection(hcID, fHitsCollection);
}

G4bool ScintiSD::ProcessHits(G4Step *step, G4TouchableHistory*)
{
  // Optical photons carry no ionisation energy - skip them here;
  // they are handled by PMTSD when they reach the cathode.
  if (step->GetTrack()->GetDefinition() ==
      G4OpticalPhoton::OpticalPhotonDefinition()) return false;

  G4double edepStep = step->GetTotalEnergyDeposit();
  if (edepStep == 0.)
    return false;

  ScintiHit *hit = new ScintiHit();
  hit->SetEdep(edepStep);
  hit->SetPos(step->GetPostStepPoint()->GetPosition());
  hit->SetTime(step->GetPostStepPoint()->GetGlobalTime());
  hit->SetParticleName(step->GetTrack()->GetDefinition()->GetParticleName());
  hit->SetTrackLength(step->GetStepLength());
  hit->SetParentID(step->GetTrack()->GetParentID());  // 0=primary, >0=secondary

  fHitsCollection->insert(hit);
  return true;
}

void ScintiSD::EndOfEvent(G4HCofThisEvent*)
{
  if (verboseLevel > 1)
    G4cout << "ScintiSD: " << fHitsCollection->entries()
           << " hits in scintillator." << G4endl;
}
