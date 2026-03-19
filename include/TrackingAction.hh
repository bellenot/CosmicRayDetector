//============================================================================
// TrackingAction.hh
//
// Suppresses trajectory storage for optical photons so they never appear
// in the visualiser. All other particles are drawn normally.
//
// Uses fpTrackingManager->SetStoreTrajectory(G4int) — the correct modern
// Geant4 API (works in Geant4 10.x and 11.x).
//   0 = do not store  (photons: invisible in vis, still fully tracked)
//   1 = store         (all other particles)
//============================================================================
#ifndef TrackingAction_h
#define TrackingAction_h 1

#include "G4UserTrackingAction.hh"
#include "globals.hh"

class TrackingAction : public G4UserTrackingAction
{
public:
  TrackingAction();
  virtual ~TrackingAction();

  virtual void PreUserTrackingAction(const G4Track *track);
  virtual void PostUserTrackingAction(const G4Track*) {}
};

#endif
