//============================================================================
// ScintiHit.cc
//============================================================================
#include "ScintiHit.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

G4ThreadLocal G4Allocator<ScintiHit>* ScintiHitAllocator = nullptr;

ScintiHit::ScintiHit()
  : G4VHit(), fEdep(0.), fPos(G4ThreeVector()),
    fTime(0.), fParticleName(""), fTrackLength(0.), fParentID(0) {}

ScintiHit::~ScintiHit() {}

ScintiHit::ScintiHit(const ScintiHit& r) : G4VHit()
{
  fEdep         = r.fEdep;
  fPos          = r.fPos;
  fTime         = r.fTime;
  fParticleName = r.fParticleName;
  fTrackLength  = r.fTrackLength;
  fParentID     = r.fParentID;
}

const ScintiHit& ScintiHit::operator=(const ScintiHit& r)
{
  fEdep         = r.fEdep;
  fPos          = r.fPos;
  fTime         = r.fTime;
  fParticleName = r.fParticleName;
  fTrackLength  = r.fTrackLength;
  fParentID     = r.fParentID;
  return *this;
}

G4bool ScintiHit::operator==(const ScintiHit& r) const { return this == &r; }

void ScintiHit::Print()
{
  G4cout << "  ScintiHit: Edep=" << fEdep/MeV << " MeV"
         << "  particle=" << fParticleName
         << "  pos=(" << fPos.x()/cm << "," << fPos.y()/cm << "," << fPos.z()/cm << ") cm"
         << "  parentID=" << fParentID
         << "  time=" << fTime/ns << " ns"
         << G4endl;
}
