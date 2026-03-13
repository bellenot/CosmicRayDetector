//============================================================================
// EventAction.cc
//============================================================================
#include "EventAction.hh"
#include "RunAction.hh"
#include "ScintiHit.hh"
#include "PMTHit.hh"

#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

EventAction::EventAction(RunAction* ra)
  : G4UserEventAction(), fRunAction(ra),
    fEdep(0.), fScintiHCID(-1), fPMTHCID(-1)
{}
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*)
{ fEdep = 0.; }

void EventAction::EndOfEventAction(const G4Event* event)
{
  G4HCofThisEvent* hce = event->GetHCofThisEvent();
  if (!hce) return;

  auto* sdm = G4SDManager::GetSDMpointer();
  if (fScintiHCID < 0) fScintiHCID = sdm->GetCollectionID("ScintiHitsCollection");
  if (fPMTHCID    < 0) fPMTHCID    = sdm->GetCollectionID("PMTHitsCollection");

  auto* sHC = static_cast<ScintiHitsCollection*>(hce->GetHC(fScintiHCID));
  auto* pHC = static_cast<PMTHitsCollection*>   (hce->GetHC(fPMTHCID));

  // --- Scintillator energy deposit ---
  G4double totalEdep = 0.;
  if (sHC)
    for (size_t i = 0; i < sHC->entries(); ++i)
      totalEdep += (*sHC)[i]->GetEdep();

  // --- PMT photon / PE counts ---
  G4int nPhotons = 0, nPE = 0;
  if (pHC) {
    nPhotons = (G4int)pHC->entries();
    for (G4int i = 0; i < nPhotons; ++i)
      if ((*pHC)[i]->IsPhotoelectron()) ++nPE;
  }

  // Accumulate into run
  if (totalEdep > 0.) fRunAction->AddEdep(totalEdep);
  for (G4int i = 0; i < nPhotons; ++i) fRunAction->AddPhoton();
  for (G4int i = 0; i < nPE;      ++i) fRunAction->AddPhotoelectron();

  // Per-event console output (only for events with signal)
  if (totalEdep > 0.01*MeV || nPhotons > 0) {
    G4cout << "  Evt " << event->GetEventID()
           << "  Edep=" << totalEdep/MeV << " MeV"
           << "  photons=" << nPhotons
           << "  PE=" << nPE
           << G4endl;
  }
}
