//============================================================================
// RunAction.cc
//============================================================================
#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

RunAction::RunAction()
  : G4UserRunAction(), fTotalEdep(0.), fTotalPhotons(0), fTotalPE(0)
{
  auto* am = G4AccumulableManager::Instance();
  am->RegisterAccumulable(fTotalEdep);
  am->RegisterAccumulable(fTotalPhotons);
  am->RegisterAccumulable(fTotalPE);
}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*)
{ G4AccumulableManager::Instance()->Reset(); }

void RunAction::EndOfRunAction(const G4Run* run)
{
  G4AccumulableManager::Instance()->Merge();
  G4int n = run->GetNumberOfEvent();
  if (n == 0) return;

  G4double edep     = fTotalEdep.GetValue();
  G4int    nPhotons = fTotalPhotons.GetValue();
  G4int    nPE      = fTotalPE.GetValue();
  G4double meanQE   = (nPhotons > 0) ? (100.*nPE/nPhotons) : 0.;

  G4cout << G4endl
    << "============================================================\n"
    << "          Cosmic Ray Detector — Run Summary                 \n"
    << "============================================================\n"
    << "  Events simulated         : " << n                          << "\n"
    << "  Total Edep in scinti     : " << edep/MeV    << " MeV"     << "\n"
    << "  Mean Edep / event        : " << edep/MeV/n  << " MeV"     << "\n"
    << "------------------------------------------------------------\n"
    << "  Optical photons at PMT   : " << nPhotons                   << "\n"
    << "  Photoelectrons produced  : " << nPE                        << "\n"
    << "  Effective QE (observed)  : " << meanQE       << " %"       << "\n"
    << "  Mean photons / event     : " << (G4double)nPhotons/n       << "\n"
    << "  Mean PE / event          : " << (G4double)nPE/n            << "\n"
    << "============================================================\n"
    << G4endl;
}
