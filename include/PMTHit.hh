//============================================================================
// PMTHit.hh
//============================================================================
#ifndef PMTHit_h
#define PMTHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class PMTHit : public G4VHit
{
public:
  PMTHit();
  virtual ~PMTHit();
  PMTHit(const PMTHit&);
  const PMTHit& operator=(const PMTHit&);
  G4bool operator==(const PMTHit&) const;

  inline void* operator new(size_t);
  inline void  operator delete(void*);

  virtual void Draw() {}
  virtual void Print();

  void SetPhotonEnergy(G4double e) { fPhotonEnergy = e; }
  void SetTime(G4double t)         { fTime = t; }
  void SetIsPhotoelectron(G4bool b){ fIsPhotoelectron = b; }

  G4double GetPhotonEnergy()    const { return fPhotonEnergy; }
  G4double GetTime()            const { return fTime; }
  G4bool   IsPhotoelectron()    const { return fIsPhotoelectron; }

private:
  G4double fPhotonEnergy;
  G4double fTime;
  G4bool   fIsPhotoelectron;   // true if photon passed QE dice roll
};

typedef G4THitsCollection<PMTHit> PMTHitsCollection;
extern G4ThreadLocal G4Allocator<PMTHit>* PMTHitAllocator;

inline void* PMTHit::operator new(size_t)
{
  if (!PMTHitAllocator)
    PMTHitAllocator = new G4Allocator<PMTHit>;
  return (void*)PMTHitAllocator->MallocSingle();
}
inline void PMTHit::operator delete(void* h)
{ PMTHitAllocator->FreeSingle((PMTHit*)h); }

#endif
