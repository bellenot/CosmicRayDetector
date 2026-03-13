# CosmicRayDetector — Geant4 Simulation

A complete Geant4 simulation of a **plastic scintillator bar** (5 × 5 × 20 cm)
used for cosmic ray detection, featuring:

- **Scintillator** — EJ-200/BC-408 equivalent (polyvinyltoluene, PVT)
- **Aluminum foil wrapping** — fully light-tight enclosure on all 6 faces of the scintillator and around the entire PMT assembly, with zero air gap
- **Optical grease** — index-matching layer (BC-630 equivalent) between scintillator and PMT window
- **PMT** — borosilicate glass window + vacuum body with bialkali photocathode QE
- **Optical photon tracking** — scintillation, Cherenkov, reflection, absorption; photons hidden from visualiser (tracked physically)
- **Cosmic ray generator** — cos²θ angular distribution with **Y as the vertical axis**, realistic species mix and power-law energy spectra, fully in C++
- **PMT photon yield** — per-photon QE dice roll in `PMTSD`; run summary reports raw photon count and photoelectron count
- **Interactive 3D visualisation** — OpenGL with colour-coded particle trajectories

---

## Directory Structure

```
CosmicRayDetector/
├── CMakeLists.txt
├── CosmicRayDetector.cc          # main()
├── include/
│   ├── DetectorConstruction.hh
│   ├── PhysicsList.hh
│   ├── ActionInitialization.hh
│   ├── PrimaryGeneratorAction.hh
│   ├── RunAction.hh
│   ├── EventAction.hh
│   ├── TrackingAction.hh         # suppresses optical photon trajectories in vis
│   ├── ScintiSD.hh / ScintiHit.hh
│   └── PMTSD.hh   / PMTHit.hh
├── src/
│   ├── DetectorConstruction.cc   # geometry + materials + optical surfaces
│   ├── PhysicsList.cc            # EM + hadronic + optical physics
│   ├── ActionInitialization.cc
│   ├── PrimaryGeneratorAction.cc # custom cosmic generator (C++, no macro)
│   ├── RunAction.cc              # run summary with photon/PE statistics
│   ├── EventAction.cc
│   ├── TrackingAction.cc
│   ├── ScintiSD.cc / ScintiHit.cc
│   └── PMTSD.cc   / PMTHit.cc
└── macros/
    ├── init_vis.mac              # auto-loaded in interactive mode
    ├── vis.mac                   # OpenGL 3D view settings
    ├── run.mac                   # quick test: 10 events
    └── cosmic_run.mac            # batch: full cosmic mix (1000 events)
```

---

## Requirements

| Software  | Version |
|-----------|---------|
| Geant4    | ≥ 11.0 (built with Qt/OpenGL) |
| CMake     | ≥ 3.16  |
| GCC/Clang | C++17   |

Geant4 must be built with:
```
-DGEANT4_USE_OPENGL_X11=ON   (or -DGEANT4_USE_QT=ON)
-DGEANT4_USE_RAYTRACER_X11=ON
-DGEANT4_INSTALL_DATA=ON
```

---

## Build Instructions

```bash
# 1. Source Geant4 environment (adjust path)
source /path/to/geant4/install/bin/geant4.sh   # bash/zsh
# or
source /path/to/geant4/install/bin/geant4.csh  # csh/tcsh

# 2. Create build directory
mkdir build && cd build

# 3. Configure
cmake ../CosmicRayDetector

# 4. Compile
make -j4

# Macro files are automatically copied to the build directory.
```

---

## Running

### Interactive mode (with 3D visualisation)
```bash
cd build
./CosmicRayDetector
```
The OpenGL window opens showing the detector geometry.
In the Geant4 session prompt:
```
Idle> /control/execute macros/run.mac
```

### Batch mode
```bash
./CosmicRayDetector macros/cosmic_run.mac
```

---

## Geometry Overview

The detector lies along the **Z axis**. **Y is the vertical axis** (gravity = −Y,
cosmic rays arrive from +Y). The aluminum foil forms a single continuous
light-tight enclosure around the entire assembly.

```
                       Y (vertical, cosmic rays ↓)
                       │
                       │
   ──────────────────────────────────────────────────── Z axis
   ←────────────── Al foil shell (0.1 mm, all faces) ──────────────→
   ┌──────────────────────────────────────────────────────────────┐
   │  ┌────────────────────────┐  ┌──┐  ┌──────────┬──────────┐  │
   │  │   Scintillator (PVT)   │  │Gl│  │  Window  │   PMT    │  │
   │  │   5 × 5 × 20 cm        │  │ue│  │  (glass) │ (vacuum) │  │
   │  └────────────────────────┘  └──┘  └──────────┴──────────┘  │
   └──────────────────────────────────────────────────────────────┘
  –Z end cap                                            +Z end cap

  Al shell construction: G4UnionSolid(box, tube) with two inner cavity
  subtractions — zero gap on all surfaces including the +Z PMT end cap.
```

---

## Primary Generator — Cosmic Ray Sampling

The generator (`PrimaryGeneratorAction.cc`) is fully implemented in C++ —
no macro needed for angular or energy distributions.

### Coordinate frame

**Y is the vertical axis.** Cosmic rays arrive from above (+Y hemisphere).
The scintillator bar lies along Z; its 5×5 cm cross-section is in the XZ plane.

### Angular distribution

Sea-level cosmic rays follow **I(θ) ∝ cos²(θ)**, where θ is the zenith angle
measured from the vertical (+Y). Sampled by rejection:

```
repeat:
    θ ~ Uniform[0, π/2)
    u ~ Uniform[0, 1)
until u ≤ cos²(θ)
```

Azimuth **φ** is sampled uniformly in `[0, 2π)`.

### Source surface

Each particle originates on an upper hemisphere of radius R centred on the
detector, with the pole at +Y:

```
src.x = R · sinθ · cosφ    (horizontal)
src.y = R · cosθ            (vertical, always > 0)
src.z = R · sinθ · sinφ    (horizontal)
```

The momentum direction points from `src` toward a random point within the
detector volume:

```
aimPoint = ( Uniform(±2.5 cm),  Uniform(±10 cm),  Uniform(±2.5 cm) )
dir      = normalize( aimPoint − src )
```

The spread matches the scintillator half-extents (±2.5 cm in X/Z,
±10 cm in Y along the bar), ensuring every particle traverses active material.

### Species mix & energy spectra

| Particle | Flux fraction | E range       | Spectral index γ |
|----------|:---:|---------------|:---:|
| μ⁻       | 50% | 0.1 – 100 GeV | 2.7 |
| μ⁺       | 25% | 0.1 – 100 GeV | 2.7 |
| γ        | 10% | 1 MeV – 10 GeV | 2.5 |
| e⁻       |  8% | 1 MeV – 1 GeV  | 3.0 |
| e⁺       |  2% | 1 MeV – 1 GeV  | 3.0 |
| p        |  5% | 1 – 1000 GeV  | 2.7 |

Kinetic energy sampled from **dN/dE ∝ E⁻ᵞ** via inverse-CDF:

```
E = [ r·(Emax^{1-γ} − Emin^{1-γ}) + Emin^{1-γ} ]^{1/(1-γ)}
```

### UI commands

| Command | Default | Description |
|---------|---------|-------------|
| `/CosmicGen/sourceRadius <mm>` | 350 | Hemisphere radius |
| `/CosmicGen/verbose true\|false` | false | Print each generated primary |

---

## Physics

### Scintillator (EJ-200 / BC-408)

| Property | Value |
|----------|-------|
| Density  | 1.032 g/cm³ |
| Refractive index | 1.58 |
| Light yield | 8 000 ph/MeV |
| Fast time constant | 2.1 ns |
| Slow time constant | 14.5 ns |
| Bulk attenuation length | 380 cm |
| Birks constant | 0.126 mm/MeV |

### Aluminum foil

The foil is a single `G4UnionSolid` (rectangular box section + cylindrical tube
section) with inner cavities subtracted, giving a seamless light-tight shell.
The inner surface is modelled as a **unified/ground reflector** with ~90%
reflectivity and a mix of specular-lobe and specular-spike components.

### Optical surfaces

| Interface | Model | Reflectivity |
|-----------|-------|-------------|
| All scintillator faces → Al foil | unified / ground, σ=0.1 | ~90% |
| Scintillator → grease → glass window | polished dielectric | TIR-limited |
| Glass window → PMT vacuum (photocathode) | glisur / absorbing | 0% (QE in SD) |

### PMT quantum efficiency (bialkali, e.g. R7600)

Applied per photon in `PMTSD` via linear interpolation + Monte Carlo:

| λ (nm) | E (eV) | QE (%) |
|--------|--------|--------|
| 700    | 1.77   | 1      |
| 600    | 2.07   | 8      |
| 500    | 2.48   | 18     |
| 460    | 2.70   | **22** ← peak |
| 420    | 2.95   | 20     |
| 380    | 3.26   | 12     |
| 340    | 3.65   | 4      |

The surface definition sets `EFFICIENCY=1` (all photons absorbed at boundary);
the real QE is then applied in `PMTSD::ProcessHits` as a Bernoulli trial:
`isPE = (G4UniformRand() < QE(E))`.

### Physics list

| Component | Class |
|-----------|-------|
| Electromagnetic | `G4EmStandardPhysics_option4` (Livermore) |
| Hadronic elastic | `G4HadronElasticPhysicsHP` |
| Hadronic inelastic | `G4HadronPhysicsQGSP_BERT_HP` |
| Stopping / capture | `G4StoppingPhysics` |
| Ion physics | `G4IonPhysics` |
| Decays | `G4DecayPhysics` + `G4RadioactiveDecayPhysics` |
| Optical | `G4OpticalPhysics` (scintillation + Cherenkov + boundary) |
| Neutron tracking cut | `G4NeutronTrackingCut` |

---

## Run Summary Output

At the end of each run the following is printed:

```
============================================================
          Cosmic Ray Detector — Run Summary
============================================================
  Events simulated         : 1000
  Total Edep in scinti     : 18342.5 MeV
  Mean Edep / event        : 18.3 MeV
------------------------------------------------------------
  Optical photons at PMT   : 47821
  Photoelectrons produced  : 9364
  Effective QE (observed)  : 19.6 %
  Mean photons / event     : 47.8
  Mean PE / event          : 9.4
============================================================
```

Expected effective QE ~18–20%, consistent with the EJ-200 emission peak
(~425 nm) sitting just below the bialkali QE peak (460 nm).

---

## Visualisation

Optical photon trajectories are **suppressed** in the visualiser
(`TrackingAction` sets `fpTrackingManager->SetStoreTrajectory(0)` for optical
photons). All physics particle tracks are drawn normally.

### Track colour legend

| Colour  | Particle |
|---------|----------|
| Blue    | μ⁻ |
| Magenta | μ⁺ |
| Yellow  | γ |
| Red     | e⁻ |
| Cyan    | e⁺ |
| White   | proton |
| Green   | neutron |

### Geometry colours

| Colour | Volume |
|--------|--------|
| Green (semi-transparent) | Scintillator |
| Silver (semi-transparent) | Al foil shell |
| Yellow-green | Optical grease |
| Light blue | PMT glass window |
| Dark blue | PMT vacuum body |

---

## Extending the Simulation

### ROOT analysis histograms
Add `G4AnalysisManager` calls in `RunAction` and `EventAction` to fill
histograms of energy deposit, photon arrival time, PE count per event, etc.

### Multiple bars (hodoscope / telescope)
Wrap the scintillator and Al shell placement in a loop over positions along Y
and add per-bar sensitive detectors.

### Wavelength-shifting fibre readout
Add a cylindrical WLS fibre volume inside the scintillator and define a
`G4OpticalSurface` with `WLSABSLENGTH` and `WLSCOMPONENT` properties.

### Dark count noise
In `EventAction::EndOfEventAction`, add a Poisson-sampled dark count
contribution: `nDark ~ Poisson(DCR × gateWidth)` and add to the PE count.

### Threshold trigger
In `EventAction`, apply a PE threshold (e.g. `nPE > 3`) and only fill
histograms for events passing it, mimicking a discriminator.
