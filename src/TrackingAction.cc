//============================================================================
// TrackingAction.cc
//============================================================================
#include "TrackingAction.hh"
#include "G4Track.hh"
#include "G4TrackingManager.hh"
#include "G4OpticalPhoton.hh"

TrackingAction::TrackingAction()  : G4UserTrackingAction() {}
TrackingAction::~TrackingAction() {}

void TrackingAction::PreUserTrackingAction(const G4Track *track)
{
  if (track->GetDefinition() == G4OpticalPhoton::OpticalPhotonDefinition())
    fpTrackingManager->SetStoreTrajectory(0);   // suppress from vis
  else
    fpTrackingManager->SetStoreTrajectory(1);   // draw all other particles
}
