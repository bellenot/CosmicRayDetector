//============================================================================
// DetectorConstruction.cc
//
// Geometry layout (Z axis, all dimensions half-lengths internally):
//
//  ←── Al shell (0.1 mm, fully light-tight) ─────────────────────────────→
//  ┌──────────────────────────────────────────────────────────────────────┐
//  │  ┌──────────────────────────┐  ┌──┐  ┌──────────────────────────┐  │
//  │  │   Scintillator (PVT)     │  │Gl│  │  PMT window  │ PMT body  │  │
//  │  │   5×5×20 cm              │  │ue│  │  (glass)     │ (vacuum)  │  │
//  │  └──────────────────────────┘  └──┘  └──────────────────────────┘  │
//  └──────────────────────────────────────────────────────────────────────┘
//   -Z end cap                                                   +Z end cap
//
// The Al shell is constructed as a single G4UnionSolid (box + cylinder),
// then the inner cavities are subtracted to leave only the thin foil walls.
// All inner volumes are placed directly in the World - the Al shell is a
// separate sibling volume that wraps around them with zero gap.
//============================================================================
#include "DetectorConstruction.hh"
#include "ScintiSD.hh"
#include "PMTSD.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4SubtractionSolid.hh"
#include "G4UnionSolid.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4OpticalSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4SDManager.hh"
#include "G4VisAttributes.hh"
#include "G4Colour.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"

//----------------------------------------------------------------------------
DetectorConstruction::DetectorConstruction()
  : G4VUserDetectorConstruction(),
    fWorldLog(nullptr), fScintiLog(nullptr), fAlLog(nullptr),
    fPMTLog(nullptr), fPMTWindowLog(nullptr), fGlueLog(nullptr),
    fScintiPhys(nullptr), fAlPhys(nullptr),
    fPMTPhys(nullptr), fPMTWindowPhys(nullptr), fGluePhys(nullptr)
{
  fScintiX       = 1.75*cm;    // half-lengths of scintillator
  fScintiY       = 1.2*cm;
  fScintiZ       = 4.0*cm;
  fAlThickness   = 0.1*mm;    // Al foil thickness
  fPMTRadius     = 1.2*cm;    // PMT outer radius (fits inside 5×5 face)
  fPMTLength     = 4.75*cm;    // PMT total half-length
  fGlueThickness = 0.01*mm;    // optical coupling layer
}

DetectorConstruction::~DetectorConstruction() {}

//----------------------------------------------------------------------------
G4VPhysicalVolume *DetectorConstruction::Construct()
{
  DefineMaterials();
  G4NistManager *nist = G4NistManager::Instance();

  //==========================================================================
  // Derived dimensions (all used in both shell and inner volumes)
  //==========================================================================
  const G4double t   = fAlThickness;
  const G4double glueHZ   = fGlueThickness / 2.;
  const G4double windowHZ = 2.5*mm;
  const G4double pmtBodyHZ = fPMTLength - windowHZ;  // half-length of vacuum body

  // Z positions of inner volumes (placed in world, centred on their own mid)
  // Scintillator centred at z=0
  const G4double glueZ   = fScintiZ + glueHZ;
  const G4double windowZ = fScintiZ + fGlueThickness + windowHZ;
  const G4double pmtZ    = windowZ  + windowHZ + pmtBodyHZ;

  // Total Z extent of all inner content (from –fScintiZ to +pmtFarEdge)
  const G4double innerFarZ  = pmtZ + pmtBodyHZ;   // +Z end of PMT body
  const G4double innerNearZ = -fScintiZ;           // –Z end of scintillator

  // Centre of the whole assembly (mid-point of inner content)
  const G4double assemblyCentreZ = (innerFarZ + innerNearZ) / 2.;

  // Half-length of the outer Al shell box section (scintillator part)
  // The shell box extends from –fScintiZ–t to +fScintiZ+t in Z, and is
  // re-centred so the subtraction works cleanly.
  // We build the shell in two pieces and union them:
  //   1. Box section: wraps the scintillator (square cross-section)
  //   2. Tube section: wraps the PMT (circular cross-section)

  //==========================================================================
  // World
  //==========================================================================
  G4Box  *    worldSolid = new G4Box("World", 15.*cm, 15.*cm, 35.*cm);
  G4Material *airMat     = nist->FindOrBuildMaterial("G4_AIR");
  {
    std::vector<G4double> e   = {1.5*eV, 4.0*eV};
    std::vector<G4double> ri  = {1.0003, 1.0003};
    std::vector<G4double> ab  = {1e9*m,  1e9*m };
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("RINDEX",    e, ri);
    mpt->AddProperty("ABSLENGTH", e, ab);
    airMat->SetMaterialPropertiesTable(mpt);
  }
  fWorldLog = new G4LogicalVolume(worldSolid, airMat, "World");
  fWorldLog->SetVisAttributes(G4VisAttributes::GetInvisible());
  G4VPhysicalVolume *worldPhys = new G4PVPlacement(nullptr, G4ThreeVector(), fWorldLog, "World", nullptr, false, 0, true);

  //==========================================================================
  // Inner volumes (all placed in World, no Al parent)
  //==========================================================================

  // -- Scintillator --
  {
    auto *solid = new G4Box("Scintillator", fScintiX, fScintiY, fScintiZ);
    fScintiLog  = new G4LogicalVolume(solid, G4Material::GetMaterial("Scintillator_PVT"), "Scintillator");
    fScintiPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fScintiLog, "Scintillator", fWorldLog, false, 0, true);
    auto *vis = new G4VisAttributes(G4Colour(0.0, 0.8, 0.0, 0.45));
    vis->SetForceSolid(true);
    fScintiLog->SetVisAttributes(vis);
  }

  // -- Optical grease --
  {
    auto *solid = new G4Tubs("Glue", 0, fPMTRadius, glueHZ, 0, 360.*deg);
    fGlueLog    = new G4LogicalVolume(solid, G4Material::GetMaterial("OpticalGrease"), "Glue");
    fGluePhys   = new G4PVPlacement(nullptr, G4ThreeVector(0,0,glueZ), fGlueLog, "Glue", fWorldLog, false, 0, true);
    auto *vis = new G4VisAttributes(G4Colour(0.95, 0.95, 0.4, 0.55));
    vis->SetForceSolid(true);
    fGlueLog->SetVisAttributes(vis);
  }

  // -- PMT glass window --
  {
    G4Material *glassMat = G4Material::GetMaterial("BorosilicateGlass");
    auto *solid   = new G4Tubs("PMTWindow", 0, fPMTRadius, windowHZ, 0, 360.*deg);
    fPMTWindowLog = new G4LogicalVolume(solid, glassMat, "PMTWindow");
    fPMTWindowPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,windowZ), fPMTWindowLog, "PMTWindow", fWorldLog, false, 0, true);
    auto *vis = new G4VisAttributes(G4Colour(0.5, 0.78, 1.0, 0.55));
    vis->SetForceSolid(true);
    fPMTWindowLog->SetVisAttributes(vis);
  }

  // -- PMT body (vacuum) --
  {
    G4Material *vac = nist->FindOrBuildMaterial("G4_Galactic");
    {
      std::vector<G4double> e  = {1.5*eV, 4.0*eV};
      std::vector<G4double> ri = {1.0, 1.0};
      auto *mpt = new G4MaterialPropertiesTable();
      mpt->AddProperty("RINDEX", e, ri);
      vac->SetMaterialPropertiesTable(mpt);
    }
    auto *solid = new G4Tubs("PMT", 0, fPMTRadius, pmtBodyHZ, 0, 360.*deg);
    fPMTLog     = new G4LogicalVolume(solid, vac, "PMT");
    fPMTPhys    = new G4PVPlacement(nullptr, G4ThreeVector(0,0,pmtZ), fPMTLog, "PMT", fWorldLog, false, 0, true);
    auto *vis = new G4VisAttributes(G4Colour(0.2, 0.2, 0.85, 0.6));
    vis->SetForceSolid(true);
    fPMTLog->SetVisAttributes(vis);
  }

  //==========================================================================
  // Aluminum foil shell - one continuous light-tight enclosure
  //
  // Construction strategy (boolean solids):
  //
  //  Step 1: Build outer envelope
  //    A) Outer box  : (fScintiX+t) × (fScintiY+t) × (fScintiZ+t) half-lengths
  //       covers scintillator + –Z end cap + 4 lateral sides + +Z face up to PMT
  //    B) Outer tube : radius (fPMTRadius+t), half-length covering glue+window+PMT
  //       covers cylindrical lateral wall and +Z end cap of PMT
  //    Shell outer = Box ∪ Tube  (union positioned so they share the +Z face)
  //
  //  Step 2: Subtract inner cavities (the volumes that live inside)
  //    Sub 1: scintillator box cavity (exact fit, no gap)
  //    Sub 2: glue + window + PMT cylinder cavity (one long cylinder)
  //
  //  Result: a thin Al shell with zero gap around all inner components.
  //
  // All boolean operations are in the coordinate system where z=0 is the
  // centre of the scintillator (same as the world origin).
  //==========================================================================

  // Outer box: wraps scintillator on all 6 sides
  // Half-lengths: (fScintiX+t, fScintiY+t, fScintiZ+t)
  // Centred at z=0 (same as scintillator)
  const G4double boxHX = fScintiX + t;
  const G4double boxHY = fScintiY + t;
  const G4double boxHZ = fScintiZ + t;   // includes –Z and +Z end caps of scinti

  auto *outerBox  = new G4Box("AlOuterBox", boxHX, boxHY, boxHZ);

  // Outer tube: wraps glue + window + PMT body, plus the +Z end cap
  // The tube runs from the scintillator +Z face (z = +fScintiZ) to the
  // PMT far end (z = +innerFarZ), plus t for the end cap.
  // Tube centre in Z: midpoint of [+fScintiZ, innerFarZ + t]
  const G4double tubeNearEdge = fScintiZ;               // flush with scinti +Z face
  const G4double tubeFarEdge  = innerFarZ + t;          // +Z end cap of PMT
  const G4double tubeHZ       = (tubeFarEdge - tubeNearEdge) / 2.;
  const G4double tubeCentreZ  = (tubeFarEdge + tubeNearEdge) / 2.;

  auto *outerTube = new G4Tubs("AlOuterTube", 0, fPMTRadius + t, tubeHZ, 0, 360.*deg);

  // Union: box + tube (tube offset in Z)
  auto *outerUnion = new G4UnionSolid("AlOuterUnion", outerBox, outerTube, nullptr, G4ThreeVector(0, 0, tubeCentreZ));

  // Inner cavity 1: scintillator box (exact size, no gap)
  auto *innerBox = new G4Box("AlInnerBox", fScintiX, fScintiY, fScintiZ);

  // Inner cavity 2: cylinder covering glue + window + PMT body (exact fit)
  // Runs from z = +fScintiZ to z = +innerFarZ
  const G4double cylHZ      = (innerFarZ - fScintiZ) / 2.;
  const G4double cylCentreZ = (innerFarZ + fScintiZ) / 2.;
  auto *innerCyl = new G4Tubs("AlInnerCyl", 0, fPMTRadius, cylHZ, 0, 360.*deg);

  // Subtract scintillator cavity
  auto *sub1 = new G4SubtractionSolid("AlSub1", outerUnion, innerBox, nullptr, G4ThreeVector(0, 0, 0));

  // Subtract PMT/glue/window cylinder cavity
  auto *alShellSolid = new G4SubtractionSolid("AlFoil", sub1, innerCyl, nullptr, G4ThreeVector(0, 0, cylCentreZ));

  fAlLog  = new G4LogicalVolume(alShellSolid, G4Material::GetMaterial("Aluminum_Refl"), "AlFoil");
  fAlPhys = new G4PVPlacement(nullptr, G4ThreeVector(0,0,0), fAlLog, "AlFoil", fWorldLog, false, 0, true);

  // Silver-grey, semi-transparent so inner volumes are visible
  auto *alVis = new G4VisAttributes(G4Colour(0.8, 0.8, 0.85, 0.25));
  alVis->SetForceSolid(true);
  fAlLog->SetVisAttributes(alVis);

  DefineSurfaces();
  return worldPhys;
}

//----------------------------------------------------------------------------
void DetectorConstruction::DefineMaterials()
{
  G4NistManager *nist = G4NistManager::Instance();
  G4Element *H  = nist->FindOrBuildElement("H");
  G4Element *C  = nist->FindOrBuildElement("C");
  G4Element *O  = nist->FindOrBuildElement("O");
  G4Element *Si = nist->FindOrBuildElement("Si");
  G4Element *B  = nist->FindOrBuildElement("B");
  G4Element *Na = nist->FindOrBuildElement("Na");

  // -- EJ-200 scintillator (PVT) --
  auto *pvt = new G4Material("Scintillator_PVT", 1.032*g/cm3, 2);
  pvt->AddElement(H, 8.5*perCent);
  pvt->AddElement(C, 91.5*perCent);
  pvt->GetIonisation()->SetBirksConstant(0.126*mm/MeV);
  {
    std::vector<G4double> en = {
      1.77*eV, 1.91*eV, 2.07*eV, 2.25*eV, 2.48*eV,
      2.58*eV, 2.70*eV, 2.82*eV, 2.95*eV, 3.10*eV,
      3.26*eV, 3.44*eV, 3.65*eV, 4.00*eV };
    std::vector<G4double> ri(en.size(), 1.58);
    std::vector<G4double> ab(en.size(), 380.*cm);
    std::vector<G4double> sc = {
      0.01,0.04,0.10,0.25,0.60,0.85,1.00,
      0.85,0.60,0.30,0.15,0.05,0.01,0.00 };
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("RINDEX",    en, ri);
    mpt->AddProperty("ABSLENGTH", en, ab);
    mpt->AddProperty("SCINTILLATIONCOMPONENT1", en, sc);
    mpt->AddProperty("SCINTILLATIONCOMPONENT2", en, sc);
    mpt->AddConstProperty("SCINTILLATIONYIELD",         8000./MeV);
    mpt->AddConstProperty("RESOLUTIONSCALE",            1.0);
    mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 2.1*ns);
    mpt->AddConstProperty("SCINTILLATIONTIMECONSTANT2", 14.5*ns);
    mpt->AddConstProperty("SCINTILLATIONYIELD1",        1.0);
    mpt->AddConstProperty("SCINTILLATIONYIELD2",        0.0);
    pvt->SetMaterialPropertiesTable(mpt);
  }

  // -- Aluminum reflector --
  auto *al = new G4Material("Aluminum_Refl",
      nist->FindOrBuildMaterial("G4_Al")->GetDensity(), 1);
  al->AddMaterial(nist->FindOrBuildMaterial("G4_Al"), 1.0);
  {
    std::vector<G4double> e  = {1.5*eV, 4.0*eV};
    std::vector<G4double> ri = {0.95, 0.92};
    std::vector<G4double> ab = {10.*nm, 10.*nm};
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("RINDEX",    e, ri);
    mpt->AddProperty("ABSLENGTH", e, ab);
    al->SetMaterialPropertiesTable(mpt);
  }

  // -- Optical grease (BC-630, n~1.465) --
  auto *grease = new G4Material("OpticalGrease", 1.06*g/cm3, 3);
  grease->AddElement(C,  60.*perCent);
  grease->AddElement(H,  30.*perCent);
  grease->AddElement(Si, 10.*perCent);
  {
    std::vector<G4double> e  = {1.77*eV, 2.48*eV, 3.10*eV, 4.00*eV};
    std::vector<G4double> ri = {1.465, 1.465, 1.470, 1.475};
    std::vector<G4double> ab = {50.*cm, 50.*cm, 50.*cm, 50.*cm};
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("RINDEX",    e, ri);
    mpt->AddProperty("ABSLENGTH", e, ab);
    grease->SetMaterialPropertiesTable(mpt);
  }

  // -- Borosilicate glass (PMT window, n~1.52) --
  auto *glass = new G4Material("BorosilicateGlass", 2.23*g/cm3, 5);
  glass->AddElement(Si, 38.*perCent);
  glass->AddElement(O,  53.*perCent);
  glass->AddElement(B,   4.*perCent);
  glass->AddElement(Na,  3.*perCent);
  glass->AddElement(H,   2.*perCent);
  {
    std::vector<G4double> e  = {1.77*eV, 2.48*eV, 3.10*eV, 4.00*eV};
    std::vector<G4double> ri = {1.519, 1.522, 1.528, 1.540};
    std::vector<G4double> ab = {420.*cm, 420.*cm, 200.*cm, 50.*cm};
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("RINDEX",    e, ri);
    mpt->AddProperty("ABSLENGTH", e, ab);
    glass->SetMaterialPropertiesTable(mpt);
  }
}

//----------------------------------------------------------------------------
void DetectorConstruction::DefineSurfaces()
{
  // -- Al shell inner surface: reflective (applied to entire Al logical volume) --
  auto *alSurf = new G4OpticalSurface("AlFoilSurface");
  alSurf->SetType(dielectric_metal);
  alSurf->SetModel(unified);
  alSurf->SetFinish(ground);
  alSurf->SetSigmaAlpha(0.1);
  {
    std::vector<G4double> e   = {1.5*eV, 4.0*eV};
    std::vector<G4double> ref = {0.90,   0.88  };
    std::vector<G4double> sl  = {0.50,   0.50  };
    std::vector<G4double> ss  = {0.40,   0.40  };
    std::vector<G4double> bs  = {0.05,   0.05  };
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("REFLECTIVITY",          e, ref);
    mpt->AddProperty("SPECULARLOBECONSTANT",  e, sl);
    mpt->AddProperty("SPECULARSPIKECONSTANT", e, ss);
    mpt->AddProperty("BACKSCATTERCONSTANT",   e, bs);
    alSurf->SetMaterialPropertiesTable(mpt);
  }
  new G4LogicalSkinSurface("AlFoilSkin", fAlLog, alSurf);

  // -- PMT photocathode: absorbs all photons; real QE applied in PMTSD --
  auto *pmtSurf = new G4OpticalSurface("PMTCathodeSurface");
  pmtSurf->SetType(dielectric_metal);
  pmtSurf->SetModel(glisur);
  pmtSurf->SetFinish(polished);
  {
    std::vector<G4double> e   = {1.77*eV,2.07*eV,2.48*eV,2.70*eV,2.95*eV,3.26*eV,3.65*eV};
    std::vector<G4double> eff = {1.0,    1.0,    1.0,    1.0,    1.0,    1.0,    1.0   };
    std::vector<G4double> ref = {0.0,    0.0,    0.0,    0.0,    0.0,    0.0,    0.0   };
    auto *mpt = new G4MaterialPropertiesTable();
    mpt->AddProperty("EFFICIENCY",   e, eff);
    mpt->AddProperty("REFLECTIVITY", e, ref);
    pmtSurf->SetMaterialPropertiesTable(mpt);
  }
  new G4LogicalBorderSurface("PMTCathodeBorder", fPMTWindowPhys, fPMTPhys, pmtSurf);
  new G4LogicalSkinSurface("PMTCathodeSkin", fPMTLog, pmtSurf);
}

//----------------------------------------------------------------------------
void DetectorConstruction::ConstructSDandField()
{
  auto *sdm = G4SDManager::GetSDMpointer();

  auto *scintiSD = new ScintiSD("ScintiSD", "ScintiHitsCollection");
  sdm->AddNewDetector(scintiSD);
  SetSensitiveDetector(fScintiLog, scintiSD);

  auto *pmtSD = new PMTSD("PMTSD", "PMTHitsCollection");
  sdm->AddNewDetector(pmtSD);
  SetSensitiveDetector(fPMTLog, pmtSD);
}
