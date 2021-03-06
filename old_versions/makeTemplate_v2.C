#include "makeTemplate_v2.h"

void makeTemplate_v2() {

  gROOT->Reset();
  gROOT->SetBatch(true);
  gROOT->SetStyle("Plain");
  gStyle->SetOptStat(0000);
  gStyle->SetOptTitle(0);

  TString hist_dir = "inputHists";

  TFile * f_xsec = new TFile("xsecdat/stop-bino_xsecs.root", "READ");
  TH2D * h_xsec = (TH2D*)f_xsec->Get("real_xsec");
  TH2D * h_xsec_errors = (TH2D*)f_xsec->Get("real_errors");

  TFile * fInputs = new TFile(hist_dir + "/limitInputs_bjj.root", "READ");

  GridPoint grid;
  bool foundBackgrounds = grid.SetBackgroundYields(fInputs);
  if(!foundBackgrounds) return;

  int mst[29] = {110, 160, 185, 210, 235, 260, 285, 310, 335, 360, 385, 410, 460, 510, 560, 610, 660, 710, 810, 910, 1010, 1110, 1210, 1310, 1410, 1510, 1710, 2010, 5010};
  int mBino[31] = {25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 375, 425, 475, 525, 575, 625, 675, 725, 825, 925, 1025, 1125, 1225, 1325, 1425, 1525, 1725, 2025};
  
  for(int i = 0; i < 899; i++) {

    grid.mStop = mst[int(i)/31];
    grid.mBino = mBino[int(i)%31];
    grid.xsec = h_xsec->GetBinContent(h_xsec->FindBin(grid.mStop, grid.mBino));
    grid.xsecError = h_xsec_errors->GetBinContent(h_xsec_errors->FindBin(grid.mStop, grid.mBino));

    bool foundPoint = grid.SetSignalYields(fInputs);
    if(!foundPoint) continue;

    grid.Print();

  }

  f_xsec->Close();
  fInputs->Close();

}

