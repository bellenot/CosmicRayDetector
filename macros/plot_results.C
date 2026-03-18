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
  gStyle->SetOptStat(0);
  gStyle->SetOptFit(0);
  gStyle->SetPalette(kBird);

  // ---- Merge all matching files ----
  TChain *chEvents = new TChain("CosmicEvents");
  TChain *chPart   = new TChain("ParticleEdep");
  chEvents->Add(pattern);
  chPart  ->Add(pattern);

  Long64_t nEvents = chEvents->GetEntries();
  std::cout << "Total events loaded: " << nEvents << std::endl;

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
  TLegend *leg1 = new TLegend(0.52, 0.75, 0.88, 0.88);
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

  c1->cd(5);
  chEvents->Draw("n_pe/n_photons>>h_qe(100, 0, 0.4)", "n_photons > 5", "HIST");
  auto *h_qe = (TH1*)gDirectory->Get("h_qe");
  h_qe->SetTitle("Observed QE per event (N_{ph}>5);N_{PE}/N_{photons};Events");
  h_qe->SetFillColor(kCyan + 1);
  h_qe->Draw("HIST");

  c1->SaveAs("energy_deposition.pdf");

  std::cout << "\nPlots saved:\n"
            << "  energy_deposition.pdf\n";

}
