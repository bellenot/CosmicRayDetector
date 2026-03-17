//============================================================================
// EventAction.cc
//
// Fills ROOT histograms and the ntuple at end of each event:
//
//   From ScintiHitsCollection:
//     - Total Edep (all tracks)
//     - Edep split by primary (parentID==0) vs secondary (parentID>0)
//     - Particle type counts (name → frequency map)
//
//   From PMTHitsCollection:
//     - Raw photon count
//     - Photoelectron count
//     - Per-photon arrival time
//
// Particle-type histograms use G4AnalysisManager H1 with string bin labels.
// The label axis is built dynamically: each unique particle name gets its
// own bin, growing as new species appear.  We use two separate TH1 objects
// (stored as profile-style H1 with integer fill weights) for primaries and
// secondaries respectively, writing the particle name as the x-axis label.
//
// Because Geant4's built-in H1 doesn't support string labels natively, we
// fill a per-run std::map<G4String,G4int> in RunAction (thread-local) and
// write a dedicated TH1I per particle category at EndOfRunAction using the
// ROOT raw API via G4RootAnalysisManager.  The cleaner portable approach
// used here is a separate ntuple column (particle name string) so the user
// can project it in ROOT with standard tools.
//============================================================================
#include "EventAction.hh"
#include "RunAction.hh"
#include "ScintiHit.hh"
#include "PMTHit.hh"

#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4HCofThisEvent.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

EventAction::EventAction(RunAction* ra)
  : G4UserEventAction(), fRunAction(ra),
    fScintiHCID(-1), fPMTHCID(-1)
{}
EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {}

//----------------------------------------------------------------------------
void EventAction::EndOfEventAction(const G4Event* event)
{
  G4HCofThisEvent* hce = event->GetHCofThisEvent();
  if (!hce) return;

  auto* sdm = G4SDManager::GetSDMpointer();
  if (fScintiHCID < 0) fScintiHCID = sdm->GetCollectionID("ScintiHitsCollection");
  if (fPMTHCID    < 0) fPMTHCID    = sdm->GetCollectionID("PMTHitsCollection");

  auto* sHC = static_cast<ScintiHitsCollection*>(hce->GetHC(fScintiHCID));
  auto* pHC = static_cast<PMTHitsCollection*>   (hce->GetHC(fPMTHCID));

  //--------------------------------------------------------------------------
  // Scintillator hits — accumulate per-event quantities
  //--------------------------------------------------------------------------
  G4double edepTotal     = 0.;
  G4double edepPrimary   = 0.;
  G4double edepSecondary = 0.;

  // particle name → [edep_primary, edep_secondary]
  std::map<G4String, std::pair<G4double,G4double>> particleEdep;

  if (sHC) {
    for (size_t i = 0; i < sHC->entries(); ++i) {
      ScintiHit* h   = (*sHC)[i];
      G4double   e   = h->GetEdep();
      G4bool     pri = (h->GetParentID() == 0);
      G4String   pn  = h->GetParticleName();

      edepTotal += e;
      if (pri) edepPrimary   += e;
      else     edepSecondary += e;

      auto& p = particleEdep[pn];
      if (pri) p.first  += e;
      else     p.second += e;
    }
  }

  //--------------------------------------------------------------------------
  // PMT hits
  //--------------------------------------------------------------------------
  G4int nPhotons = 0, nPE = 0;
  if (pHC) {
    nPhotons = (G4int)pHC->entries();
    for (G4int i = 0; i < nPhotons; ++i) {
      PMTHit* ph = (*pHC)[i];
      if (ph->IsPhotoelectron()) ++nPE;
    }
  }

  //--------------------------------------------------------------------------
  // Fill RunAction accumulables (for terminal summary)
  //--------------------------------------------------------------------------
  if (edepTotal > 0.) fRunAction->AddEdep(edepTotal);
  for (G4int i = 0; i < nPhotons; ++i) fRunAction->AddPhoton();
  for (G4int i = 0; i < nPE;      ++i) fRunAction->AddPhotoelectron();

  //--------------------------------------------------------------------------
  // Fill ROOT histograms via G4AnalysisManager
  //--------------------------------------------------------------------------
  auto* ana = G4AnalysisManager::Instance();

  // H1 id=0: total Edep per event
  if (edepTotal > 0.)
    ana->FillH1(0, edepTotal / MeV);

  // H1 id=1: photons at PMT
  if (nPhotons > 0)
    ana->FillH1(1, (G4double)nPhotons);

  // H1 id=2: photoelectrons
  if (nPE > 0)
    ana->FillH1(2, (G4double)nPE);

  // H1 id=3: primary Edep
  if (edepPrimary > 0.)
    ana->FillH1(3, edepPrimary / MeV);

  // H1 id=4: secondary Edep
  if (edepSecondary > 0.)
    ana->FillH1(4, edepSecondary / MeV);

  // H1 id=5: photon arrival times
  if (pHC) {
    for (G4int i = 0; i < nPhotons; ++i)
      ana->FillH1(5, (*pHC)[i]->GetTime() / ns);
  }

  // H2 id=0: Edep vs PE
  if (edepTotal > 0. || nPE > 0)
    ana->FillH2(0, edepTotal / MeV, (G4double)nPE);

  //--------------------------------------------------------------------------
  // Fill ntuple (one row per event)
  //--------------------------------------------------------------------------
  // Ntuple id=0 — use explicit (ntupleId, colId, value) form
  ana->FillNtupleDColumn(0, 0, edepTotal     / MeV);
  ana->FillNtupleIColumn(0, 1, nPhotons);
  ana->FillNtupleIColumn(0, 2, nPE);
  ana->FillNtupleDColumn(0, 3, edepPrimary   / MeV);
  ana->FillNtupleDColumn(0, 4, edepSecondary / MeV);
  ana->AddNtupleRow(0);

  //--------------------------------------------------------------------------
  // Per-event particle-type ntuples (one row per particle species seen)
  // Ntuple id=1 (booked in RunAction)
  //--------------------------------------------------------------------------
  for (auto& kv : particleEdep) {
    ana->FillNtupleSColumn(1, 0, kv.first);               // particle name
    ana->FillNtupleDColumn(1, 1, kv.second.first  / MeV); // edep primary
    ana->FillNtupleDColumn(1, 2, kv.second.second / MeV); // edep secondary
    ana->AddNtupleRow(1);
  }

  //--------------------------------------------------------------------------
  // Console output (only for events with signal)
  //--------------------------------------------------------------------------
  if (edepTotal > 0.01*MeV || nPhotons > 0) {
    G4cout << "  Evt " << event->GetEventID()
           << "  Edep=" << edepTotal/MeV << " MeV"
           << " (pri=" << edepPrimary/MeV << " sec=" << edepSecondary/MeV << ")"
           << "  photons=" << nPhotons << "  PE=" << nPE
           << G4endl;
  }
}
