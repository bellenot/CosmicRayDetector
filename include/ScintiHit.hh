//============================================================================
// ScintiHit.hh
//============================================================================
#ifndef ScintiHit_h
#define ScintiHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "globals.hh"

class ScintiHit : public G4VHit
{
public:
  ScintiHit();
  virtual ~ScintiHit();
  ScintiHit(const ScintiHit&);
  const ScintiHit& operator=(const ScintiHit&);
  G4bool operator==(const ScintiHit&) const;

  inline void* operator new(size_t);
  inline void  operator delete(void*);

  virtual void Draw() {}
  virtual void Print();

  // Setters
  void SetEdep(G4double e)         { fEdep = e; }
  void SetPos(G4ThreeVector p)     { fPos = p; }
  void SetTime(G4double t)         { fTime = t; }
  void SetParticleName(G4String n) { fParticleName = n; }
  void SetTrackLength(G4double l)  { fTrackLength = l; }
  void SetParentID(G4int id)       { fParentID = id; }   // 0=primary, >0=secondary

  // Getters
  G4double      GetEdep()         const { return fEdep; }
  G4ThreeVector GetPos()          const { return fPos; }
  G4double      GetTime()         const { return fTime; }
  G4String      GetParticleName() const { return fParticleName; }
  G4double      GetTrackLength()  const { return fTrackLength; }
  G4int         GetParentID()     const { return fParentID; }

private:
  G4double      fEdep;
  G4ThreeVector fPos;
  G4double      fTime;
  G4String      fParticleName;
  G4double      fTrackLength;
  G4int         fParentID;
};

typedef G4THitsCollection<ScintiHit> ScintiHitsCollection;
extern G4ThreadLocal G4Allocator<ScintiHit>* ScintiHitAllocator;

inline void* ScintiHit::operator new(size_t)
{
  if (!ScintiHitAllocator)
    ScintiHitAllocator = new G4Allocator<ScintiHit>;
  return (void*)ScintiHitAllocator->MallocSingle();
}

inline void ScintiHit::operator delete(void* hit)
{
  ScintiHitAllocator->FreeSingle((ScintiHit*)hit);
}

#endif
