//============================================================================
// ScintiHit.cc
//============================================================================
#include "ScintiHit.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

G4ThreadLocal G4Allocator<ScintiHit>* ScintiHitAllocator = nullptr;

ScintiHit::ScintiHit()
  : G4VHit(), fEdep(0.), fPos(G4ThreeVector()), fTime(0.),
    fParticleName(""), fTrackLength(0.) {}

ScintiHit::~ScintiHit() {}

ScintiHit::ScintiHit(const ScintiHit& right) : G4VHit()
{
  fEdep         = right.fEdep;
  fPos          = right.fPos;
  fTime         = right.fTime;
  fParticleName = right.fParticleName;
  fTrackLength  = right.fTrackLength;
}

const ScintiHit& ScintiHit::operator=(const ScintiHit& right)
{
  fEdep         = right.fEdep;
  fPos          = right.fPos;
  fTime         = right.fTime;
  fParticleName = right.fParticleName;
  fTrackLength  = right.fTrackLength;
  return *this;
}

G4bool ScintiHit::operator==(const ScintiHit& right) const
{
  return (this == &right);
}

void ScintiHit::Print()
{
  G4cout << "  ScintiHit: Edep=" << fEdep/MeV << " MeV"
         << "  particle=" << fParticleName
         << "  pos=(" << fPos.x()/cm << "," << fPos.y()/cm << "," << fPos.z()/cm << ") cm"
         << "  time=" << fTime/ns << " ns"
         << G4endl;
}
