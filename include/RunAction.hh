//============================================================================
// RunAction.hh
//
// Owns the G4AnalysisManager instance and books all ROOT histograms.
// Histograms are filled by EventAction at the end of each event.
// The ROOT file is written and closed at EndOfRunAction.
//
// Histograms booked:
//   ID 0  h_edep          — total energy deposit per event (MeV)
//   ID 1  h_photons        — raw optical photons reaching PMT per event
//   ID 2  h_pe             — photoelectrons produced per event
//   ID 3  h_edep_primary   — Edep from primary tracks only (parentID==0)
//   ID 4  h_edep_secondary — Edep from secondary tracks only (parentID>0)
//   ID 5  h_edep_vs_pe     — 2D: Edep (x) vs PE count (y)
//
// 1D string histograms (H1 with string labels):
//   ID 6  h_particle_primary   — particle types depositing energy (primary)
//   ID 7  h_particle_secondary — particle types depositing energy (secondary)
//============================================================================
#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"

class G4Run;

class RunAction : public G4UserRunAction
{
public:
  RunAction();
  virtual ~RunAction();
  virtual void BeginOfRunAction(const G4Run*);
  virtual void EndOfRunAction(const G4Run*);

  // Called by EventAction
  void AddEdep(G4double e)       { fTotalEdep    += e; }
  void AddPhoton()               { fTotalPhotons += 1; }
  void AddPhotoelectron()        { fTotalPE      += 1; }

  // Output filename (can be overridden before BeginOfRunAction)
  void SetOutputFileName(const G4String& name) { fFileName = name; }

private:
  G4Accumulable<G4double> fTotalEdep;
  G4Accumulable<G4int>    fTotalPhotons;
  G4Accumulable<G4int>    fTotalPE;

  G4String fFileName;
};

#endif
