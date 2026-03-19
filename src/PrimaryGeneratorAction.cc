//============================================================================
// PrimaryGeneratorAction.cc
//
// Cosmic ray generator with:
//  - cos²(θ) zenith-angle distribution  (physical for sea-level muons)
//  - uniform azimuth φ ∈ [0, 2π)
//  - hemisphere source surface so every angle aims at the detector
//  - realistic species mix and per-species power-law energy spectra
//  - UI commands to adjust source radius and verbosity
//============================================================================
#include "PrimaryGeneratorAction.hh"

#include "G4Event.hh"
#include "G4ParticleTable.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "Randomize.hh"
#include "G4UImanager.hh"

#include <cmath>
#include <numeric>

//----------------------------------------------------------------------------
PrimaryGeneratorAction::PrimaryGeneratorAction() : G4VUserPrimaryGeneratorAction(),
    G4UImessenger(), fGun(nullptr), fSourceRadius(35.*cm), fVerbose(false)
{
  fGun = new G4ParticleGun(1);

  //--------------------------------------------------------------------------
  // Species table
  // fraction : relative flux weight (sea-level approximation)
  // eMin/eMax: kinetic energy range in GeV
  // gamma    : spectral index (dN/dE ∝ E^{-gamma})
  //--------------------------------------------------------------------------
  fSpecies = {{
    //  name         frac   eMin    eMax    gamma
    { "mu-",         0.50,  0.10,   100.0,  2.7  },   // dominant cosmic component
    { "mu+",         0.25,  0.10,   100.0,  2.7  },   // μ+/μ- ratio ~1.25
    { "gamma",       0.10,  0.001,  10.0,   2.5  },   // secondary γ from EM showers
    { "e-",          0.08,  0.001,  1.0,    3.0  },   // secondary electrons
    { "e+",          0.02,  0.001,  1.0,    3.0  },   // secondary positrons
    { "proton",      0.05,  1.0,    1000.0, 2.7  },   // primary proton component
  }};

  // Build cumulative distribution function for species sampling
  G4double total = 0.;
  for (auto &s : fSpecies)
    total += s.fraction;
  G4double cumul = 0.;
  for (int i = 0; i < kNSpecies; ++i) {
    cumul    += fSpecies[i].fraction / total;
    fCDF[i]   = cumul;
  }

  //--------------------------------------------------------------------------
  // UI messenger
  //--------------------------------------------------------------------------
  fDir = new G4UIdirectory("/CosmicGen/");
  fDir->SetGuidance("Cosmic ray generator controls");

  fCmdRadius = new G4UIcmdWithADouble("/CosmicGen/sourceRadius", this);
  fCmdRadius->SetGuidance("Hemisphere source radius (in mm)");
  fCmdRadius->SetParameterName("radius", false);
  fCmdRadius->SetDefaultValue(350.);   // mm = 35 cm

  fCmdVerbose = new G4UIcmdWithABool("/CosmicGen/verbose", this);
  fCmdVerbose->SetGuidance("Print each generated primary");
  fCmdVerbose->SetParameterName("flag", false);
  fCmdVerbose->SetDefaultValue(false);
}

//----------------------------------------------------------------------------
PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fGun;
  delete fCmdRadius;
  delete fCmdVerbose;
  delete fDir;
}

//----------------------------------------------------------------------------
void PrimaryGeneratorAction::SetNewValue(G4UIcommand *cmd, G4String val)
{
  if (cmd == fCmdRadius)
    fSourceRadius = fCmdRadius->GetNewDoubleValue(val)*mm;
  else if (cmd == fCmdVerbose)
    fVerbose = fCmdVerbose->GetNewBoolValue(val);
}

//----------------------------------------------------------------------------
// Sample zenith angle θ from I(θ) ∝ cos²(θ) via rejection sampling.
// cos²(θ) is maximal at θ=0 (=1) so the envelope is uniform on [0,1].
//----------------------------------------------------------------------------
G4double PrimaryGeneratorAction::SampleCosSquaredTheta() const
{
  G4double theta, u;
  do {
    // Uniform θ in [0, π/2)  (upper hemisphere only)
    theta = G4UniformRand() * (CLHEP::pi / 2.);
    u     = G4UniformRand();
  } while (u > std::pow(std::cos(theta), 2.));
  return theta;
}

//----------------------------------------------------------------------------
// Sample kinetic energy from a power-law spectrum dN/dE ∝ E^{-gamma}
// using the inverse-CDF method:
//   CDF(E) = (E^{1-gamma} - Emin^{1-gamma}) /
//            (Emax^{1-gamma} - Emin^{1-gamma})    for gamma ≠ 1
//   CDF(E) = ln(E/Emin)/ln(Emax/Emin)             for gamma = 1
//----------------------------------------------------------------------------
G4double PrimaryGeneratorAction::SamplePowerLaw(G4double eMin, G4double eMax, G4double gam) const
{
  G4double r = G4UniformRand();
  if (std::abs(gam - 1.0) < 1.e-6) {
    // gamma = 1 → log-uniform
    return eMin * std::pow(eMax / eMin, r);
  }
  G4double alpha = 1. - gam;
  G4double t = r * (std::pow(eMax, alpha) - std::pow(eMin, alpha)) + std::pow(eMin, alpha);
  return std::pow(t, 1. / alpha);
}

//----------------------------------------------------------------------------
// Select species index from CDF
//----------------------------------------------------------------------------
G4int PrimaryGeneratorAction::SampleSpecies() const
{
  G4double r = G4UniformRand();
  for (int i = 0; i < kNSpecies; ++i)
    if (r <= fCDF[i])
      return i;
  return kNSpecies - 1;
}

//----------------------------------------------------------------------------
// Place the source point on the upper hemisphere with Y as the vertical axis.
//
// Cosmic rays arrive from above: vertical = +Y, gravity = -Y.
// Spherical coordinates with polar axis along +Y:
//   x =  R sinθ cosφ
//   y =  R cosθ          ← always > 0  (upper hemisphere, "sky side")
//   z =  R sinθ sinφ
//
// θ = 0   → particle comes straight down from directly overhead
// θ = π/2 → particle arrives horizontally (grazing incidence)
//----------------------------------------------------------------------------
G4ThreeVector PrimaryGeneratorAction::SampleSourcePoint(G4double theta, G4double phi) const
{
  G4double sinT = std::sin(theta);
  G4double cosT = std::cos(theta);
  return G4ThreeVector(fSourceRadius * sinT * std::cos(phi),   // x (horizontal)
                       fSourceRadius * cosT,                    // y (vertical, >0)
                       fSourceRadius * sinT * std::sin(phi));   // z (horizontal)
}

//----------------------------------------------------------------------------
// GeneratePrimaries — called once per event
//----------------------------------------------------------------------------
void PrimaryGeneratorAction::GeneratePrimaries(G4Event *event)
{
  // 1. Sample angular coordinates
  G4double theta = SampleCosSquaredTheta();          // zenith  [0, π/2)
  G4double phi   = G4UniformRand() * CLHEP::twopi;  // azimuth [0, 2π)

  // 2. Source position on hemisphere
  G4ThreeVector srcPos = SampleSourcePoint(theta, phi);

  // 3. Momentum direction = from source point toward a random point inside
  //    the detector volume, so every generated particle actually crosses it.
  //    Spread matches the detector extent:
  //      X, Z: ±2.5 cm  (scintillator cross-section half-width)
  //      Y   : ±10 cm   (scintillator half-length along the bar axis)
  G4double spreadX = (G4UniformRand() - 0.5) * 5.*cm;
  G4double spreadY = (G4UniformRand() - 0.5) * 20.*cm;
  G4double spreadZ = (G4UniformRand() - 0.5) * 5.*cm;
  G4ThreeVector aimPt(spreadX, spreadY, spreadZ);
  G4ThreeVector dir = (aimPt - srcPos).unit();

  // 4. Species and energy
  G4int idx = SampleSpecies();
  const CosmicSpecies &sp = fSpecies[idx];

  G4double eKinGeV = SamplePowerLaw(sp.eMin, sp.eMax, sp.gamma);
  G4double eKin    = eKinGeV * GeV;

  // 5. Resolve particle definition
  G4ParticleDefinition *pd = G4ParticleTable::GetParticleTable()->FindParticle(sp.name);
  if (!pd) {
    G4cerr << "CosmicGen: unknown particle '" << sp.name << "'" << G4endl;
    return;
  }

  // 6. Set gun and fire
  fGun->SetParticleDefinition(pd);
  fGun->SetParticlePosition(srcPos);
  fGun->SetParticleMomentumDirection(dir);
  fGun->SetParticleEnergy(eKin);
  fGun->GeneratePrimaryVertex(event);

  if (fVerbose) {
    G4cout << "[CosmicGen] event=" << event->GetEventID()
           << "  particle=" << sp.name
           << "  Ekin="     << eKinGeV  << " GeV"
           << "  θ="        << theta/deg << "°"
           << "  φ="        << phi/deg   << "°"
           << "  src(cm)=(" << srcPos.x()/cm << ","
                            << srcPos.y()/cm << ","
                            << srcPos.z()/cm << ")"
           << "  dir=("     << dir.x() << "," << dir.y() << "," << dir.z() << ")"
           << G4endl;
  }
}
