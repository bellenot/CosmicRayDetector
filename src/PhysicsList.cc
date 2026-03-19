//============================================================================
// PhysicsList.cc
// EM (option4) + hadronic (QGSP_BERT_HP) + optical photons
//============================================================================
#include "PhysicsList.hh"

#include "G4EmStandardPhysics_option4.hh"
#include "G4EmExtraPhysics.hh"
#include "G4DecayPhysics.hh"
#include "G4RadioactiveDecayPhysics.hh"
#include "G4StoppingPhysics.hh"
#include "G4HadronElasticPhysicsHP.hh"
#include "G4HadronPhysicsQGSP_BERT_HP.hh"
#include "G4IonPhysics.hh"
#include "G4NeutronTrackingCut.hh"
#include "G4OpticalPhysics.hh"
#include "G4OpticalParameters.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList() : G4VModularPhysicsList()
{
  SetVerboseLevel(1);

  RegisterPhysics(new G4EmStandardPhysics_option4());
  RegisterPhysics(new G4EmExtraPhysics());
  RegisterPhysics(new G4DecayPhysics());
  RegisterPhysics(new G4RadioactiveDecayPhysics());
  RegisterPhysics(new G4HadronElasticPhysicsHP());
  RegisterPhysics(new G4HadronPhysicsQGSP_BERT_HP());
  RegisterPhysics(new G4StoppingPhysics());
  RegisterPhysics(new G4IonPhysics());
  RegisterPhysics(new G4NeutronTrackingCut());

  // Optical physics
  G4OpticalPhysics *opticalPhysics = new G4OpticalPhysics();
  auto *op = G4OpticalParameters::Instance();
  op->SetScintTrackSecondariesFirst(true);
  op->SetScintByParticleType(false);
  op->SetScintFiniteRiseTime(false);
  op->SetCerenkovMaxPhotonsPerStep(300);
  op->SetCerenkovMaxBetaChange(0.1);
  op->SetCerenkovTrackSecondariesFirst(true);
  op->SetBoundaryInvokeSD(true);   // fires PMTSD when photon hits cathode surface
  RegisterPhysics(opticalPhysics);
}

PhysicsList::~PhysicsList() {}

void PhysicsList::SetCuts()
{
  SetCutValue(0.1*mm, "gamma");
  SetCutValue(0.1*mm, "e-");
  SetCutValue(0.1*mm, "e+");
  SetCutValue(0.1*mm, "proton");
  if (verboseLevel > 0) G4cout << "PhysicsList: cuts set." << G4endl;
}
