//============================================================================
// ScintiSD.hh  -  Scintillator sensitive detector
//============================================================================
#ifndef ScintiSD_h
#define ScintiSD_h 1

#include "G4VSensitiveDetector.hh"
#include "ScintiHit.hh"
#include <vector>

class G4Step;
class G4HCofThisEvent;
class G4TouchableHistory;

class ScintiSD : public G4VSensitiveDetector
{
public:
  ScintiSD(const G4String& name, const G4String& hitsCollectionName);
  virtual ~ScintiSD();

  virtual void   Initialize(G4HCofThisEvent* hitCollection);
  virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
  virtual void   EndOfEvent(G4HCofThisEvent* hitCollection);

private:
  ScintiHitsCollection* fHitsCollection;
};

#endif
