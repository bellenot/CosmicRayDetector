#include "PMTHit.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"

G4ThreadLocal G4Allocator<PMTHit>* PMTHitAllocator = nullptr;

PMTHit::PMTHit()
  : G4VHit(), fPhotonEnergy(0.), fTime(0.), fIsPhotoelectron(false) {}
PMTHit::~PMTHit() {}
PMTHit::PMTHit(const PMTHit& r) : G4VHit()
{ fPhotonEnergy=r.fPhotonEnergy; fTime=r.fTime; fIsPhotoelectron=r.fIsPhotoelectron; }
const PMTHit& PMTHit::operator=(const PMTHit& r)
{ fPhotonEnergy=r.fPhotonEnergy; fTime=r.fTime; fIsPhotoelectron=r.fIsPhotoelectron; return *this; }
G4bool PMTHit::operator==(const PMTHit& r) const { return this==&r; }
void PMTHit::Print()
{
  G4cout << "  PMTHit: E=" << fPhotonEnergy/eV << " eV"
         << "  t=" << fTime/ns << " ns"
         << "  PE=" << (fIsPhotoelectron?"yes":"no") << G4endl;
}
