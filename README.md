# CosmicRayDetector — Geant4 Simulation

A complete Geant4 simulation of a **plastic scintillator bar** (5 × 5 × 20 cm)
used for cosmic ray detection, featuring:

- **Scintillator** — EJ-200/BC-408 equivalent (polyvinyltoluene, PVT)
- **Aluminum foil wrapping** — reflective optical boundary on 5 faces
- **Optical grease** — index-matching layer (BC-630 equivalent)
- **PMT** — borosilicate glass window + vacuum body with bialkali photocathode QE
- **Optical photon tracking** — scintillation, Cherenkov, reflection, absorption
- **Cosmic ray particles** — muons, gammas, electrons, protons with realistic spectra
- **Interactive 3D visualisation** — OpenGL with colour-coded trajectories

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
│   ├── ScintiSD.hh / ScintiHit.hh
│   └── PMTSD.hh   / PMTHit.hh
├── src/
│   ├── DetectorConstruction.cc   # geometry + materials + optical surfaces
│   ├── PhysicsList.cc            # EM + hadronic + optical physics
│   ├── ActionInitialization.cc
│   ├── PrimaryGeneratorAction.cc # GPS-based cosmic generator
│   ├── RunAction.cc
│   ├── EventAction.cc
│   ├── ScintiSD.cc / ScintiHit.cc
│   └── PMTSD.cc   / PMTHit.cc
└── macros/
    ├── init_vis.mac              # auto-loaded in interactive mode
    ├── vis.mac                   # OpenGL 3D view settings
    ├── run.mac                   # quick test: 10 muons
    └── cosmic_run.mac            # batch: full cosmic mix (1000 events)
```

---

## Requirements

| Software | Version |
|----------|---------|
| Geant4   | ≥ 11.0 (built with Qt/OpenGL + GDML optional) |
| CMake    | ≥ 3.16  |
| GCC/Clang | C++17  |

Geant4 must be built with:
```
-DGEANT4_USE_OPENGL_X11=ON   (or Qt)
-DGEANT4_USE_RAYTRACER_X11=ON
-DGEANT4_INSTALL_DATA=ON
```

---

## Build Instructions

```bash
# 1. Source Geant4 environment (adjust path)
source /path/to/geant4/install/bin/geant4.sh   # Linux/macOS
# or
source /path/to/geant4/install/bin/geant4.csh  # csh/tcsh

# 2. Create build directory
mkdir build && cd build

# 3. Configure
cmake ../CosmicRayDetector

# 4. Compile (use -j N for N parallel jobs)
make -j4

# The macro folder is automatically copied to the build directory.
```

---

## Running

### Interactive mode (with 3D visualisation)
```bash
cd build
./CosmicRayDetector
```
The OpenGL window opens with the detector geometry rendered.
Then in the Geant4 session prompt, run:
```
Idle> /control/execute macros/run.mac
```

### Batch mode (no window)
```bash
./CosmicRayDetector macros/cosmic_run.mac
```

---

## Geometry Overview

```
Z-axis (beam direction = downward in lab, +Z in simulation)
 ─────────────────────────────────────────────────────────►
           │← Al foil (0.1 mm, 5 faces) →│
 ┌─────────┼─────────────────────────────┼──┬──────────────┐
 │         │     Scintillator (PVT)       │G │ PMT (vacuum) │
 │         │    5 × 5 × 20 cm            │l │  ⌀ 4.2 cm    │
 │         │    EJ-200 equivalent         │u │  5 cm long   │
 │         │    8000 ph/MeV, τ=2.1 ns    │e │              │
 └─────────┼─────────────────────────────┼──┴──────────────┘
                                           ↑
                                      Glass window
                                      (borosilicate, n=1.52)
```

Cosmic particles enter from above (–Z direction in lab frame).

---

## Primary Generator — Cosmic Ray Sampling

The generator (`PrimaryGeneratorAction.cc`) is fully implemented in C++ —
no macro configuration needed for the angular/energy distributions.

### Angular distribution

Sea-level cosmic rays follow **I(θ) ∝ cos²(θ)**, where θ is the zenith angle.
This is sampled via rejection:

```
repeat:
    θ ~ Uniform[0, π/2)
    u ~ Uniform[0, 1)
until u ≤ cos²(θ)
```

The azimuth **φ** is sampled uniformly in `[0, 2π)`.

### Source surface

Each particle is placed on a **hemisphere of radius R** centred on the detector.
The momentum direction points from that surface point toward a randomly chosen
spot inside the detector volume (±2 cm in XY, ±5 cm in Z), so every particle
is guaranteed to cross the detector regardless of angle.

```
src = ( R·sinθ·cosφ,  R·sinθ·sinφ,  R·cosθ )
dir = normalize( detectorAimPoint − src )
```

### Species mix & energy spectra

| Particle | Flux fraction | E range | Spectral index γ |
|----------|:---:|---|:---:|
| μ⁻       | 50% | 0.1 – 100 GeV | 2.7 |
| μ⁺       | 25% | 0.1 – 100 GeV | 2.7 |
| γ        | 10% | 1 MeV – 10 GeV | 2.5 |
| e⁻       |  8% | 1 MeV – 1 GeV  | 3.0 |
| e⁺       |  2% | 1 MeV – 1 GeV  | 3.0 |
| p        |  5% | 1 – 1000 GeV  | 2.7 |

Kinetic energy is sampled from **dN/dE ∝ E⁻ᵞ** via the inverse-CDF:

```
E = [ r·(Emax^{1-γ} − Emin^{1-γ}) + Emin^{1-γ} ]^{1/(1-γ)}
```

### UI commands (available in interactive session)

| Command | Default | Description |
|---------|---------|-------------|
| `/CosmicGen/sourceRadius <mm>` | 350 | Hemisphere radius |
| `/CosmicGen/verbose true\|false` | false | Print each primary |

---

## Physics Highlights

### Scintillator (EJ-200)
| Property | Value |
|----------|-------|
| Density  | 1.032 g/cm³ |
| Refractive index | 1.58 |
| Light yield | 8 000 ph/MeV |
| Fast decay constant | 2.1 ns |
| Bulk attenuation length | 380 cm |
| Birks constant | 0.126 mm/MeV |

### Optical Surfaces
| Interface | Model | Reflectivity |
|-----------|-------|-------------|
| Scintillator → Al foil | unified/ground | ~90% |
| Scintillator → Grease → Glass | polished dielectric | TIR-limited |
| Glass → PMT vacuum (photocathode) | glisur, absorbing | 0% (QE model) |

### PMT Quantum Efficiency (bialkali)
| Energy (eV) | λ (nm) | QE (%) |
|-------------|--------|--------|
| 1.77 | 700 | 1 |
| 2.48 | 500 | 18 |
| 2.70 | 460 | 22 (peak) |
| 2.95 | 420 | 20 |
| 3.65 | 340 | 4 |

### Physics List
- **EM**: `G4EmStandardPhysics_option4` (Livermore, best for optical)
- **Hadronic**: `QGSP_BERT_HP` (HP neutron tracking)
- **Optical**: `G4OpticalPhysics` (scintillation + Cherenkov + boundary)
- **Decays**: standard + radioactive
- **Muons**: stopping + capture physics included

---

## Visualisation Legend

| Colour | Particle |
|--------|----------|
| Yellow | γ (gamma) |
| Red    | e⁻ |
| Cyan   | e⁺ |
| Blue   | μ⁻ |
| Magenta | μ⁺ |
| White  | proton |
| Green  | neutron |
| Orange | optical photon |

---

## Extending the Simulation

### Add WLSF (wavelength-shifting fibre)
Add a `G4OpticalSurface` with `WLSABSLENGTH` and `WLSCOMPONENT` properties
on the new fibre volume.

### Enable analysis (ROOT histograms)
Add `G4AnalysisManager` calls in `RunAction` and `EventAction` to fill
histograms of energy deposit, photon arrival time, etc.

### Multiple scintillator bars (hodoscope)
Wrap the placement in a loop over Z positions and add per-bar SDs.

### Add noise / dark counts
In `PMTSD::ProcessHits`, apply a random acceptance based on QE and add
Poissonian dark count events in `EventAction::EndOfEventAction`.
