//============================================================================
// RunAction.cc
//============================================================================
#include "RunAction.hh"
#include "G4Run.hh"
#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

//----------------------------------------------------------------------------
RunAction::RunAction() : G4UserRunAction(), fTotalEdep(0.), fTotalPhotons(0),
    fTotalPE(0), fFileName("CosmicRayDetector")
{
  auto *am = G4AccumulableManager::Instance();
  am->RegisterAccumulable(fTotalEdep);
  am->RegisterAccumulable(fTotalPhotons);
  am->RegisterAccumulable(fTotalPE);

  //--------------------------------------------------------------------------
  // Book histograms (done once in the constructor so they exist on all
  // threads in MT mode; the analysis manager merges them at end-of-run)
  //--------------------------------------------------------------------------
  auto *anaMgr = G4AnalysisManager::Instance();
  anaMgr->SetVerboseLevel(1);
  anaMgr->SetNtupleMerging(true);   // merge ntuples across threads

  // ---- 1D histograms ----
  // H1 id=0: total Edep per event
  anaMgr->CreateH1("h_edep", "Energy deposit in scintillator per event;E_{dep} (MeV);Events", 200, 0., 20.);

  // H1 id=1: raw photon count at PMT per event
  anaMgr->CreateH1("h_photons", "Optical photons reaching PMT per event;N_{photons};Events", 200, 0., 25000.);

  // H1 id=2: photoelectron count per event
  anaMgr->CreateH1("h_pe", "Photoelectrons produced per event;N_{PE};Events", 200, 0., 5000.);

  // H1 id=3: Edep from primary tracks only
  anaMgr->CreateH1("h_edep_primary", "Edep from primary tracks;E_{dep,primary} (MeV);Events", 200, 0., 15.);

  // H1 id=4: Edep from secondary tracks only
  anaMgr->CreateH1("h_edep_secondary", "Edep from secondary tracks;E_{dep,secondary} (MeV);Events", 200, 0., 15.);

  // H1 id=5: photon arrival time at PMT (relative to event start)
  anaMgr->CreateH1("h_photon_time", "Optical photon arrival time at PMT;t (ns);Photons", 200, 0., 20.);

  // ---- 2D histogram ----
  // H2 id=0: Edep vs PE count
  anaMgr->CreateH2("h_edep_vs_pe", "Energy deposit vs photoelectrons;E_{dep} (MeV);N_{PE}", 100, 0., 20., 100, 0., 5000.);

  // ---- Ntuples (one row per event) ----
  // Ntuple id=0: per-event summary
  anaMgr->CreateNtuple("CosmicEvents", "Per-event summary");
  anaMgr->CreateNtupleDColumn("edep");           // col 0: total Edep (MeV)
  anaMgr->CreateNtupleIColumn("n_photons");      // col 1: raw photons at PMT
  anaMgr->CreateNtupleIColumn("n_pe");           // col 2: photoelectrons
  anaMgr->CreateNtupleDColumn("edep_primary");   // col 3: primary Edep (MeV)
  anaMgr->CreateNtupleDColumn("edep_secondary"); // col 4: secondary Edep (MeV)
  anaMgr->FinishNtuple();                        // id=0

  // Ntuple id=1: per-event particle-type breakdown
  // One row per (event, particle_name) pair that deposited energy
  anaMgr->CreateNtuple("ParticleEdep", "Per-event particle-type energy deposit");
  anaMgr->CreateNtupleSColumn("particle");       // col 0: particle name
  anaMgr->CreateNtupleDColumn("edep_primary");   // col 1: primary Edep for this type (MeV)
  anaMgr->CreateNtupleDColumn("edep_secondary"); // col 2: secondary Edep for this type (MeV)
  anaMgr->FinishNtuple();                        // id=1
}

RunAction::~RunAction() {}

//----------------------------------------------------------------------------
void RunAction::BeginOfRunAction(const G4Run *run)
{
  G4AccumulableManager::Instance()->Reset();
 
  auto *anaMgr = G4AnalysisManager::Instance();
  // Append run number to filename so multiple runs don't overwrite each other
  G4String fname = fFileName + "_run" + std::to_string(run->GetRunID()) + ".root";
  // Reset histograms from previous run
  anaMgr->Reset();
  anaMgr->OpenFile(fname);
  G4cout << "==> ROOT file: " << fname << G4endl;
}

//----------------------------------------------------------------------------
void RunAction::EndOfRunAction(const G4Run *run)
{
  G4AccumulableManager::Instance()->Merge();

  auto *anaMgr = G4AnalysisManager::Instance();
  anaMgr->Write();
  anaMgr->CloseFile(false);

  G4int n = run->GetNumberOfEvent();
  if (n == 0)
    return;

  G4double edep     = fTotalEdep.GetValue();
  G4int    nPhotons = fTotalPhotons.GetValue();
  G4int    nPE      = fTotalPE.GetValue();
  G4double meanQE   = (nPhotons > 0) ? (100.*nPE / nPhotons) : 0.;

  G4cout << G4endl
    << "============================================================\n"
    << "     Cosmic Ray Detector — Run " << run->GetRunID() << " Summary\n"
    << "============================================================\n"
    << "  Events simulated         : " << n                         << "\n"
    << "  Total Edep in scinti     : " << edep/MeV   << " MeV"      << "\n"
    << "  Mean Edep / event        : " << edep/MeV/n << " MeV"      << "\n"
    << "------------------------------------------------------------\n"
    << "  Optical photons at PMT   : " << nPhotons                  << "\n"
    << "  Photoelectrons produced  : " << nPE                       << "\n"
    << "  Effective QE (observed)  : " << meanQE      << " %"       << "\n"
    << "  Mean photons / event     : " << (G4double)nPhotons/n      << "\n"
    << "  Mean PE / event          : " << (G4double)nPE/n           << "\n"
    << "============================================================\n"
    << G4endl;
}
