//============================================================================
// EventAction.hh
//============================================================================
#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <map>

class RunAction;

class EventAction : public G4UserEventAction
{
public:
  EventAction(RunAction *runAction);
  virtual ~EventAction();
  virtual void BeginOfEventAction(const G4Event*);
  virtual void EndOfEventAction(const G4Event*);

private:
  RunAction *fRunAction;
  G4int      fScintiHCID;
  G4int      fPMTHCID;
};

#endif
