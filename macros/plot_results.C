// ============================================================
// plot_results.C
//
// ROOT macro to plot histograms from one or more simulation
// output files produced by CosmicRayDetector.
//
// Usage (merge several runs):
//   root -l 'plot_results.C("CosmicRayDetector_run*.root")'
//
// Or single file:
//   root -l 'plot_results.C("CosmicRayDetector_run0.root")'
// ============================================================
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TTree.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "THStack.h"
#include "TStyle.h"
#include "TChain.h"
#include "TROOT.h"
#include <iostream>

void plot_results(const char *pattern = "CosmicRayDetector_run0.root")
{
  gStyle->SetOptStat(1110);
  gStyle->SetOptFit(1);
  gStyle->SetPalette(kBird);

  // ---- Merge all matching files ----
  TChain *chEvents = new TChain("CosmicEvents");
  TChain *chPart   = new TChain("ParticleEdep");
  chEvents->Add(pattern);
  chPart  ->Add(pattern);

  Long64_t nEvents = chEvents->GetEntries();
  std::cout << "Total events loaded: " << nEvents << std::endl;

#if 1
  // ============================================================
  // Canvas 1: Energy deposit
  // ============================================================
  TCanvas *c1 = new TCanvas("c1", "Energy Deposition", 1200, 800);
  c1->Divide(3, 2);

  c1->cd(1);
  gPad->SetLogy();
  chEvents->Draw("edep>>h_edep(100, 0, 50)",  "", "HIST");
  auto *h_edep = (TH1*)gDirectory->Get("h_edep");
  h_edep->SetTitle("Total E_{dep} per event;E_{dep} (MeV);Events");
  h_edep->SetFillColor(kAzure + 7);
  h_edep->Draw("HIST");

  c1->cd(2);
  gPad->SetLogy();
  chEvents->Draw("edep_primary>>h_pri(100, 0, 50)", "", "HIST");
  chEvents->Draw("edep_secondary>>h_sec(100, 0, 50)", "", "HIST SAME");
  auto *h_pri = (TH1*)gDirectory->Get("h_pri");
  auto *h_sec = (TH1*)gDirectory->Get("h_sec");
  h_pri->SetTitle("Primary vs Secondary E_{dep};E_{dep} (MeV);Events");
  h_pri->SetLineColor(kBlue);
  h_pri->SetFillColor(kBlue - 9);
  h_pri->SetFillStyle(3004);
  h_sec->SetLineColor(kRed);
  h_sec->SetFillColor(kRed - 9);
  h_sec->SetFillStyle(3005);
  h_pri->Draw("HIST");
  h_sec->Draw("HIST SAME");
  TLegend *leg1 = new TLegend(0.55, 0.65, 0.88, 0.88);
  leg1->AddEntry(h_pri, "Primary tracks",   "f");
  leg1->AddEntry(h_sec, "Secondary tracks", "f");
  leg1->Draw();

  c1->cd(3);
  // Ratio of secondary to total Edep per event
  chEvents->Draw("(edep_secondary/(edep+1e-9))>>h_ratio(100, 0, 1)", "edep > 0.01", "HIST");
  auto *h_ratio = (TH1*)gDirectory->Get("h_ratio");
  h_ratio->SetTitle("Secondary fraction of E_{dep};E_{sec}/E_{tot};Events");
  h_ratio->SetFillColor(kGreen + 2);
  h_ratio->Draw("HIST");

  c1->cd(4);
  chEvents->Draw("edep:n_pe>>h_edep_pe(200, 0, 5000, 200, 0, 20)", "", "COLZ");
  auto *h2 = (TH2*)gDirectory->Get("h_edep_pe");
  h2->SetTitle("E_{dep} vs Photoelectrons;E_{dep} (MeV);N_{PE}");
  h2->Draw("COLZ");
  c1->SaveAs("energy_deposition.pdf");

  c1->cd(5);
  chEvents->Draw("n_pe/n_photons>>h_qe(100, 0, 0.4)", "n_photons > 5", "HIST");
  auto *h_qe = (TH1*)gDirectory->Get("h_qe");
  h_qe->SetTitle("Observed QE per event (N_{ph}>5);N_{PE}/N_{photons};Events");
  h_qe->SetFillColor(kCyan + 1);
  h_qe->Draw("HIST");

  std::cout << "\nPlots saved:\n"
            << "  energy_deposition.pdf\n";

#else
  // ============================================================
  // Canvas 1: Energy deposit
  // ============================================================
  TCanvas *c1 = new TCanvas("c1", "Energy Deposition", 1200, 800);
  c1->Divide(2, 2);

  c1->cd(1);
  chEvents->Draw("edep>>h_edep(200,0,500)",  "", "HIST");
  auto *h_edep = (TH1*)gDirectory->Get("h_edep");
  h_edep->SetTitle("Total E_{dep} per event;E_{dep} (MeV);Events");
  h_edep->SetFillColor(kAzure+7);
  h_edep->Draw("HIST");

  c1->cd(2);
  chEvents->Draw("edep_primary>>h_pri(200,0,500)", "", "HIST");
  chEvents->Draw("edep_secondary>>h_sec(200,0,200)", "", "HIST SAME");
  auto *h_pri = (TH1*)gDirectory->Get("h_pri");
  auto *h_sec = (TH1*)gDirectory->Get("h_sec");
  h_pri->SetTitle("Primary vs Secondary E_{dep};E_{dep} (MeV);Events");
  h_pri->SetLineColor(kBlue);  h_pri->SetFillColor(kBlue-9);
  h_sec->SetLineColor(kRed);   h_sec->SetFillColor(kRed-9);
  h_pri->Draw("HIST");
  h_sec->Draw("HIST SAME");
  TLegend *leg1 = new TLegend(0.55,0.65,0.88,0.88);
  leg1->AddEntry(h_pri, "Primary tracks",   "f");
  leg1->AddEntry(h_sec, "Secondary tracks", "f");
  leg1->Draw();

  c1->cd(3);
  chEvents->Draw("edep:n_pe>>h_edep_pe(100,0,500,100,0,500)", "", "COLZ");
  auto *h2 = (TH2*)gDirectory->Get("h_edep_pe");
  h2->SetTitle("E_{dep} vs Photoelectrons;E_{dep} (MeV);N_{PE}");
  h2->Draw("COLZ");

  c1->cd(4);
  // Ratio of secondary to total Edep per event
  chEvents->Draw("(edep_secondary/(edep+1e-9))>>h_ratio(100,0,1)",
      "edep>0.01", "HIST");
  auto *h_ratio = (TH1*)gDirectory->Get("h_ratio");
  h_ratio->SetTitle("Secondary fraction of E_{dep};E_{sec}/E_{tot};Events");
  h_ratio->SetFillColor(kGreen+2);
  h_ratio->Draw("HIST");

  c1->SaveAs("energy_deposition.pdf");

  // ============================================================
  // Canvas 2: PMT signals
  // ============================================================
  TCanvas *c2 = new TCanvas("c2", "PMT Signals", 1200, 500);
  c2->Divide(3, 1);

  c2->cd(1);
  chEvents->Draw("n_photons>>h_ph(200,0,2000)", "", "HIST");
  auto *h_ph = (TH1*)gDirectory->Get("h_ph");
  h_ph->SetTitle("Optical photons at PMT;N_{photons};Events");
  h_ph->SetFillColor(kOrange+7);
  h_ph->Draw("HIST");

  c2->cd(2);
  chEvents->Draw("n_pe>>h_pe(200,0,500)", "", "HIST");
  auto *h_pe = (TH1*)gDirectory->Get("h_pe");
  h_pe->SetTitle("Photoelectrons;N_{PE};Events");
  h_pe->SetFillColor(kViolet+2);
  h_pe->Draw("HIST");

  c2->cd(3);
  chEvents->Draw("n_pe/n_photons>>h_qe(100,0,0.4)",
      "n_photons>5", "HIST");
  auto *h_qe = (TH1*)gDirectory->Get("h_qe");
  h_qe->SetTitle("Observed QE per event (N_{ph}>5);N_{PE}/N_{photons};Events");
  h_qe->SetFillColor(kCyan+1);
  h_qe->Draw("HIST");

  c2->SaveAs("pmt_signals.pdf");

  // ============================================================
  // Canvas 3: Particle types
  // ============================================================
  TCanvas *c3 = new TCanvas("c3", "Particle Types", 1200, 600);
  c3->Divide(2, 1);

  // Get unique particle names and their total Edep
  c3->cd(1);
  // Draw primary Edep by particle type
  chPart->Draw("particle>>h_part_pri",
      "edep_primary>0", "");
  auto *h_part_pri = (TH1*)gDirectory->Get("h_part_pri");
  if (h_part_pri) {
    h_part_pri->SetTitle("Primary particle types in scintillator;Particle;Count");
    h_part_pri->SetFillColor(kBlue-7);
    h_part_pri->LabelsDeflate("X");
    h_part_pri->LabelsOption("v", "X");
    h_part_pri->Draw("HIST");
  }

  c3->cd(2);
  chPart->Draw("particle>>h_part_sec",
      "edep_secondary>0", "");
  auto *h_part_sec = (TH1*)gDirectory->Get("h_part_sec");
  if (h_part_sec) {
    h_part_sec->SetTitle("Secondary particle types in scintillator;Particle;Count");
    h_part_sec->SetFillColor(kRed-7);
    h_part_sec->LabelsDeflate("X");
    h_part_sec->LabelsOption("v", "X");
    h_part_sec->Draw("HIST");
  }

  c3->SaveAs("particle_types.pdf");

  std::cout << "\nPlots saved:\n"
            << "  energy_deposition.pdf\n"
            << "  pmt_signals.pdf\n"
            << "  particle_types.pdf\n";
#endif
}
