//============================================================================
// RunAction.hh
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

  void AddEdep(G4double e)      { fTotalEdep    += e; }
  void AddPhoton()              { fTotalPhotons += 1; }
  void AddPhotoelectron()       { fTotalPE      += 1; }

private:
  G4Accumulable<G4double> fTotalEdep;
  G4Accumulable<G4int>    fTotalPhotons;
  G4Accumulable<G4int>    fTotalPE;
};

#endif
