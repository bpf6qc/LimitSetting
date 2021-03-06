#include "makeTemplate.h"

void makeVersion(TString nameCode) {

  TString hist_dir = "inputHists";

  TFile * f_xsec = new TFile("xsecdat/stop-bino_xsecs.root", "READ");
  TH2D * h_xsec = (TH2D*)f_xsec->Get("real_xsec");
  TH2D * h_xsec_errors = (TH2D*)f_xsec->Get("real_errors");

  TFile * fInputs = new TFile(hist_dir + "/limitInputs_bjj.root", "READ");

  GridPoint grid(nameCode);

  grid.SetUseExtraFloat(true); // extra 100% lnU nuisance on ttjets and ttgamma, correlated

  if(nameCode == "SR1") {
    grid.AddChannel("ele_SR1", false, 10);
    grid.AddChannel("muon_SR1", false, 10);
  }

  else if(nameCode == "SR2") {
    grid.AddChannel("ele_SR2", false, 10);
    grid.AddChannel("muon_SR2", false, 10);
  }

  else if(nameCode == "ele") {
    grid.AddChannel("ele_SR1", false, 10);
    grid.AddChannel("ele_SR2", false, 10);
  }

  else if(nameCode == "muon") {
    grid.AddChannel("muon_SR1", false, 10);
    grid.AddChannel("muon_SR2", false, 10);
  }

  else if(nameCode == "allChannels") {
    grid.AddChannel("ele_SR1", false, 10);
    grid.AddChannel("muon_SR1", false, 10);
    grid.AddChannel("ele_SR2", false, 10);
    grid.AddChannel("muon_SR2", false, 10);
  }

  else {
    cout << endl << endl << "use a real value for nameCode!" << endl << endl;
    return;
  }


  grid.Init();

  bool foundBackgrounds = grid.SetBackgroundYields(fInputs);
  if(!foundBackgrounds) return;

  int mst[29] = {110, 160, 185, 210, 235, 260, 285, 310, 335, 360, 385, 410, 460, 510, 560, 610, 660, 710, 810, 910, 1010, 1110, 1210, 1310, 1410, 1510, 1710, 2010, 5010};
  int mBino[31] = {25, 50, 75, 100, 125, 150, 175, 200, 225, 250, 275, 300, 325, 375, 425, 475, 525, 575, 625, 675, 725, 825, 925, 1025, 1125, 1225, 1325, 1425, 1525, 1725, 2025};
  
  for(int i = 0; i < 899; i++) {

    grid.mStop = mst[int(i)/31];
    grid.mBino = mBino[int(i)%31];

    if(grid.mStop - grid.mBino < 172.5) continue;

    grid.xsec = h_xsec->GetBinContent(h_xsec->FindBin(grid.mStop, grid.mBino));
    grid.xsecError = h_xsec_errors->GetBinContent(h_xsec_errors->FindBin(grid.mStop, grid.mBino));

    bool foundPoint = grid.SetSignalYields(fInputs);
    if(!foundPoint) continue;

    grid.SetUseStatError();

    grid.Print();

  }

  f_xsec->Close();
  fInputs->Close();

}

void makeTemplate() {

  makeVersion("allChannels");
  makeVersion("SR1");
  makeVersion("SR2");
  makeVersion("ele");
  makeVersion("muon");

}
