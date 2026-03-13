//============================================================================
// DetectorConstruction.hh
//============================================================================
#ifndef DetectorConstruction_h
#define DetectorConstruction_h 1

#include "G4VUserDetectorConstruction.hh"
#include "globals.hh"

class G4LogicalVolume;
class G4VPhysicalVolume;

class DetectorConstruction : public G4VUserDetectorConstruction
{
public:
  DetectorConstruction();
  virtual ~DetectorConstruction();

  virtual G4VPhysicalVolume* Construct();
  virtual void ConstructSDandField();

  G4LogicalVolume* GetScintiLogical() const { return fScintiLog; }
  G4LogicalVolume* GetPMTLogical()    const { return fPMTLog; }

private:
  void DefineMaterials();
  void DefineSurfaces();

  G4double fScintiX, fScintiY, fScintiZ;
  G4double fAlThickness;
  G4double fPMTRadius, fPMTLength;
  G4double fGlueThickness;

  G4LogicalVolume*   fWorldLog;
  G4LogicalVolume*   fScintiLog;
  G4LogicalVolume*   fAlLog;
  G4LogicalVolume*   fPMTLog;
  G4LogicalVolume*   fPMTWindowLog;
  G4LogicalVolume*   fGlueLog;

  G4VPhysicalVolume* fScintiPhys;
  G4VPhysicalVolume* fAlPhys;
  G4VPhysicalVolume* fPMTPhys;
  G4VPhysicalVolume* fPMTWindowPhys;
  G4VPhysicalVolume* fGluePhys;
};

#endif
