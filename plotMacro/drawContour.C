#include <iostream>
#include <fstream>

#include <TStyle.h>
#include <TSystem.h>
#include <TROOT.h>
#include <TFile.h>
#include <TDirectory.h>
#include <TString.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphSmooth.h>
#include <TLatex.h>
#include <TLegend.h>
#include <TLine.h>

#include "util.h"

using namespace std;

bool blinded = false;

bool usePasStyle = true;

void drawContour(TString scan="stop-bino", bool print=true) {

  gROOT->ForceStyle();

  gStyle->SetOptStat(0);
  gStyle->SetPalette(1);

  //For the temperature plots
  gStyle->SetPadRightMargin(0.2);
  gStyle->SetPadLeftMargin(0.18);
  gStyle->SetPadTopMargin(0.1);
  gStyle->SetPadBottomMargin(0.15);
  gStyle->SetTitleOffset(1.4, "xz");
  gStyle->SetTitleOffset(1.9, "y");

  gStyle->SetNdivisions(505);
  gStyle->SetTitleFont(43, "xyz");
  gStyle->SetTitleSize(32, "xyz");
  gStyle->SetLabelFont(42, "xyz");
  gStyle->SetLabelSize(0.04, "xyz");

  TString output_dir = "hist";

  TString datafile = "table/" + scan + ".table";

  gSystem->mkdir(output_dir, true);

  bool useSmooth = false;
  bool useCustomGetContour = false;

  int diagonal = 1;

  TString option2D = "COL Z";

  int xMin = 222.5;
  int xMax = 960;
  int yMin = 137.5;
  int yMax = 775;

  TString xLabel = "M_{Stop} [GeV]";
  TString yLabel = "M_{Bino} [GeV]";

  const int nX = 16;
  const int nY = 16;

  Double_t mst[nX] = {235, 260, 285, 310, 335, 360, 385, 410, 460, 510, 560, 610, 660, 710, 810, 910};
  Double_t mBino[nY] = {150, 175, 200, 225, 250, 275, 300, 325, 375, 425, 475, 525, 575, 625, 675, 725};

  Double_t xBins[nX+1];
  xBins[0] = 222.5;
  for(int i = 1; i < nX; i++) xBins[i] = (mst[i] + mst[i-1])/2.;
  xBins[nX] = 960;

  Double_t yBins[nY+1];
  yBins[0] = 137.5;
  for(int i = 1; i < nY; i++) yBins[i] = (mBino[i] + mBino[i-1])/2.;
  yBins[nY] = 775;

  TFile* fout = new TFile(output_dir+"/hist_exclusion_"+scan+".root","RECREATE");

  const int nxs = 3;
  TString xsname[nxs] = {"xsec", "xsec_obs","xsec_exp"};
  TH2D* h_xs[nxs];
  for(int i=0; i<nxs; i++){
    h_xs[i] = new TH2D(xsname[i],xsname[i],nX,xBins,nY,yBins);
  }

  const int nlimit = 10;
  TString limitname[nlimit] = {"obs", "obs_1L", "obs_1H", "exp", "exp_1L","exp_1H", "obs_up3", "obs_down3", "exp_up3", "exp_down3"};
  TH2D* h_limit[nlimit];
  for(int i=0; i<nlimit; i++){
    h_limit[i] = new TH2D(limitname[i],limitname[i],nX,xBins,nY,yBins);
  }

  ifstream fin;
  fin.open(datafile.Data());

  while(1){

    //mStop mBino xsec xsecError obsLimit expLimit exp_m1s exp_m2s exp_p1s exp_p2s

    int ms, mb;
    double xsec, xsecError, obsLimit, expLimit, exp_m1s, exp_m2s, exp_p1s, exp_p2s;
    fin >> ms >> mb >> xsec >> xsecError >> obsLimit >> expLimit >> exp_m1s >> exp_m2s >> exp_p1s >> exp_p2s;
    if(!fin.good()) break;

    double oneSigma_L = xsecError / 100.;
    double oneSigma_H = xsecError / 100.;

    double xx = ms;
    double yy = mb;

    h_xs[0]->Fill(xx, yy, xsec);
    h_xs[1]->Fill(xx,yy,obsLimit*xsec);
    h_xs[2]->Fill(xx,yy,expLimit*xsec);

    h_limit[0]->Fill(xx, yy, obsLimit);
    h_limit[1]->Fill(xx, yy, obsLimit * (1 - oneSigma_L));
    h_limit[2]->Fill(xx, yy, obsLimit * (1 + oneSigma_H));
    h_limit[3]->Fill(xx, yy, expLimit);
    h_limit[4]->Fill(xx, yy, exp_m1s);
    h_limit[5]->Fill(xx, yy, exp_p1s);
    h_limit[6]->Fill(xx, yy, obsLimit / 3.);
    h_limit[7]->Fill(xx, yy, obsLimit * 3.);
    h_limit[8]->Fill(xx, yy, expLimit / 3.);
    h_limit[9]->Fill(xx, yy, expLimit * 3.);

  }// while
  fin.close();

  // fix pot holes for all TH2D's
  for(int i=0; i<nxs; i++) fillPotHoles(h_xs[i], diagonal);
  for(int i=0; i<nlimit; i++) fillPotHoles(h_limit[i], diagonal);

  TLatex pasLumiLatex;
  pasLumiLatex.SetNDC();
  pasLumiLatex.SetTextAngle(0);
  pasLumiLatex.SetTextColor(kBlack);
  pasLumiLatex.SetTextFont(42);
  pasLumiLatex.SetTextAlign(31);
  pasLumiLatex.SetTextSize(0.048 / 0.7);

  TLatex pasCMSLatex;
  pasCMSLatex.SetNDC();
  pasCMSLatex.SetTextAngle(0);
  pasCMSLatex.SetTextColor(kBlack);
  pasCMSLatex.SetTextFont(61);
  pasCMSLatex.SetTextAlign(11);
  pasCMSLatex.SetTextSize(0.06 / 0.7);

  TLatex pasPrelimLatex;
  pasPrelimLatex.SetNDC();
  pasPrelimLatex.SetTextAngle(0);
  pasPrelimLatex.SetTextColor(kBlack);
  pasPrelimLatex.SetTextFont(52);
  pasPrelimLatex.SetTextAlign(11);
  pasPrelimLatex.SetTextSize(0.0456 / 0.7);

  // common labels for every plot
  TLatex* lat = new TLatex(0.18,0.92,"#bf{CMS Preliminary}  #sqrt{s} = 8 TeV");
  lat->SetNDC(true);
  lat->SetTextFont(43);
  lat->SetTextSize(21/*25*/);

  TLatex* lat2 = new TLatex(0.49, 0.92, "       L_{int} = 19.7 fb^{-1},   e/#mu + #geq bjj+#gamma(#gamma)");
  lat2->SetNDC(true);
  lat2->SetTextFont(43);
  lat2->SetTextSize(21/*25*/);

  TLatex* lat3 = new TLatex(0.49, 0.92, "       L_{int} = 19.7 fb^{-1},   e/#mu + bjj + #gamma(#gamma)");
  lat3->SetNDC(true);
  lat3->SetTextFont(43);
  lat3->SetTextSize(25);

  // for diagonal == 1
  TGraph * upperDiagonalRegion = new TGraph(4);
  upperDiagonalRegion->SetPoint(0, yMin + 172.5, yMin);
  upperDiagonalRegion->SetPoint(1, xMin, yMin);
  upperDiagonalRegion->SetPoint(2, xMin, yBins[nY]);
  upperDiagonalRegion->SetPoint(3, yBins[nX] + 172.5, yBins[nY]);
  upperDiagonalRegion->SetFillColor(16);

  TLine * nlspLine = new TLine(222.5, 222.5, 775, 775);
  nlspLine->SetLineStyle(2);
  nlspLine->SetLineWidth(2);

  TLine * virtualLine = new TLine(310, 137.5, 947.5, 775);
  virtualLine->SetLineStyle(2);
  virtualLine->SetLineWidth(2);

  TLatex * nlspComment = new TLatex(250, 275, "M_{Stop} < M_{Bino}");
  nlspComment->SetTextAngle(45);
  nlspComment->SetTextSize(0.02);
  
  TLatex * virtualComment = new TLatex(300, 150, "M_{Stop} - M_{Bino} < M_{t}");
  virtualComment->SetTextAngle(45);
  virtualComment->SetTextSize(0.02);

  TString title;
  double minVal, maxVal;

  TCanvas* can_xs = new TCanvas("can_xsec_"+scan,"can_xsec_"+scan, 800, 600);
  getMinMaxValues(h_xs[0], minVal, maxVal, diagonal);
  cout << "xs min=" << minVal << ", max=" << maxVal << endl;
  h_xs[0]->SetMaximum(1.1*maxVal);
  h_xs[0]->SetMinimum(0.9*minVal);
  title = ";" + xLabel + ";" + yLabel + ";Cross Section [pb]";
  h_xs[0]->SetTitle(title);
  can_xs->SetLogz();
  h_xs[0]->Draw(option2D);

  if(usePasStyle) {
    pasLumiLatex.DrawLatex(0.96, 0.9, "19.7 fb^{-1} (8 TeV) e/#mu + bjj + #gamma(#gamma)");
    pasCMSLatex.DrawLatex(0.12, 0.9, "CMS");
    pasPrelimLatex.DrawLatex(0.2178, 0.9, "Preliminary");
  }
  else {
    lat->Draw("same");
    lat2->Draw("same");
  }
  
  if(diagonal==1) {
    upperDiagonalRegion->Draw("same f");
    virtualLine->Draw("same");
    nlspLine->Draw("same");
    virtualComment->Draw("same");
    nlspComment->Draw("same");
  }
  can_xs->RedrawAxis();
  if(print) {
    can_xs->Print("",".pdf");
  }

  TCanvas* can_limit = new TCanvas("can_limit_"+scan,"can_limit_"+scan,900,800);
  getMinMaxValues(h_xs[1],minVal,maxVal,diagonal);
  cout << "limit min=" << minVal << ", max=" << maxVal << endl;
  h_xs[1]->GetZaxis()->SetRangeUser(1.e-3, 2.e-2);
  //h_xs[1]->SetMaximum(1.1*maxVal);
  //h_xs[1]->SetMinimum(0.9*minVal);
  title = ";" + xLabel + ";" + yLabel + ";95% CL cross section upper limit [pb]";
  h_xs[1]->SetTitle(title);
  can_limit->SetLogz();
  h_xs[1]->Draw(option2D);
  lat->Draw("same");
  lat2->Draw("same");
  if(diagonal==1) {
    upperDiagonalRegion->Draw("same f");
    virtualLine->Draw("same");
    nlspLine->Draw("same");
    virtualComment->Draw("same");
    nlspComment->Draw("same");
  }
  can_limit->RedrawAxis();
  if(print) {
    can_limit->Print("",".pdf");
  }

  TH2D * h_back = new TH2D("h_back_"+scan,";"+xLabel+";"+yLabel,100,xMin,xMax,100,yMin,yMax);

  cout << "Now get contours..." << endl;

  TGraph* curv[nlimit];

  //double contours[2]={ 1.0, 1.5};
  double contours[2]={ 0.0, 1.0};
  TCanvas *can_excl01 = new TCanvas("can_contour_"+scan, "can_contour_"+scan,1200,800);
  can_excl01->Divide(nlimit/2,2);
  for(int i=0; i<nlimit; i++) {
    can_excl01->cd(i + 1);
    h_back->Draw();

    h_limit[i]->SetContour(2,contours);
    h_limit[i]->Draw("SAME CONT LIST");
    gPad->Update();
    
    TObjArray *contsM = (TObjArray*) gROOT->GetListOfSpecials()->FindObject("contours");
    TList* contLevel = (TList*)contsM->At(0);
    curv[i] = (TGraph*)contLevel->First()->Clone("exclusion_contour_"+limitname[i]);

    int maxDurp = 3;
    //if(i == 3 || i == 4 || i == 6 || i == 8) maxDurp = 13;
    for(int iDurp = 0; iDurp < maxDurp; iDurp++) curv[i]->RemovePoint(curv[i]->GetN() - 1);
    
  }// for i

  cout << "Smoothing..." << endl;

  TGraphSmooth* gs[nlimit];
  TGraph* curvS[nlimit];
  for(int i=0; i<nlimit; i++) {
    gs[i] = new TGraphSmooth("normal");
    if(useSmooth) curvS[i] = gs[i]->SmoothSuper(curv[i]);
    else curvS[i] = (TGraph*) curv[i]->Clone();
    curvS[i]->SetName("exclusion_smooth_contour_"+limitname[i]);
  }

  // make excluded region
  TGraph* excludedRegion = new TGraph(curvS[0]->GetN()+3);
  int nbins = curvS[0]->GetN();
  for(int i=0; i<nbins; i++){
    double x,y;
    curvS[0]->GetPoint(i,x,y);
    excludedRegion->SetPoint(i,x,y);
  }

  excludedRegion->SetPoint(nbins,xMin,yMax);
  excludedRegion->SetPoint(nbins+1,xMin,yMin);
  excludedRegion->SetPoint(nbins+2,xMax,yMin);

  excludedRegion->SetFillColor(kBlue-10);
  excludedRegion->SetFillStyle(1001);

  // experimental 1 sigma error around expected limit
  curvS[4]->SetLineStyle(3);
  curvS[4]->SetLineWidth(3);
  curvS[4]->SetLineColor(kOrange+9);

  curvS[5]->SetLineStyle(3);
  curvS[5]->SetLineWidth(3);
  curvS[5]->SetLineColor(kOrange+9);

  // expected limit
  curvS[3]->SetLineStyle(9);
  curvS[3]->SetLineWidth(3);
  curvS[3]->SetLineColor(kOrange+9);

  // theory 1 sigma around observed limit
  curvS[1]->SetLineStyle(3);
  curvS[1]->SetLineWidth(2);
  curvS[1]->SetLineColor(4);

  curvS[2]->SetLineStyle(3);
  curvS[2]->SetLineWidth(2);
  curvS[2]->SetLineColor(4);

  // observed limit
  curvS[0]->SetLineWidth(3);
  curvS[0]->SetLineColor(4);

  // observed limit with xsec scaled up by 3
  curvS[6]->SetLineStyle(9);
  curvS[6]->SetLineWidth(2);
  curvS[6]->SetLineColor(kBlack);

  curvS[8]->SetLineStyle(9);
  curvS[8]->SetLineWidth(2);
  curvS[8]->SetLineColor(kBlack);

  // observed limit with xsec scaled down by 3
  curvS[7]->SetLineStyle(2);
  curvS[7]->SetLineWidth(2);
  curvS[7]->SetLineColor(kBlack);

  curvS[9]->SetLineStyle(2);
  curvS[9]->SetLineWidth(2);
  curvS[9]->SetLineColor(kBlack);

  TGraph* exp1sigma_aroundExp = makeBandGraph(curvS[4],curvS[5]);

  // experimental 1 sigma band around expected limit
  exp1sigma_aroundExp->SetFillColor(kOrange-3);
  exp1sigma_aroundExp->SetFillStyle(1001);

  TCanvas* can_excl02 = new TCanvas("can_exclusion_"+scan, "can_exclusion_"+scan,900,800);
  h_back->Draw();
  can_excl02->SetRightMargin(0.08);

  // ecluded region
  //  excludedRegion->Draw("SAME F");

  exp1sigma_aroundExp->Draw("SAME F");
  curvS[3]->Draw("SAME L");
  if(!blinded) curvS[1]->Draw("SAME L");
  if(!blinded) curvS[2]->Draw("SAME L");
  if(!blinded) curvS[0]->Draw("SAME L");
  lat->Draw("same");
  lat3->Draw("same");

  //  PrintPoints(curvS[0]);

  if(diagonal==1) {
    upperDiagonalRegion->Draw("same f");
    virtualLine->Draw("same");
    nlspLine->Draw("same");
    virtualComment->Draw("same");
    nlspComment->Draw("same");
  }
  //      PrintPoints(curv[i][j]);
  //      RemovePoints(curv[i][j]);

  double leg_xmin = 0.58;
  double leg_xmax = 0.9;
  double leg_ymin = 0.64;
  double leg_ymax = 0.87;
  if(diagonal){
    leg_xmin -= 0.35;
    leg_xmax -= 0.35;
  }

  TString legendTitle = "pp #rightarrow #tilde{t}#tilde{t},    #tilde{t} #rightarrow t + #tilde{#chi}^{0}_{1},    #tilde{#chi}^{0}_{1} #rightarrow #gamma + #tilde{G}";

  TLegend* leg = new TLegend(leg_xmin,leg_ymin,leg_xmax,leg_ymax,legendTitle,"brNDC");
  leg->SetFillStyle(0);
  leg->SetLineColor(0);
  leg->SetBorderSize(0);
  leg->SetTextFont(42);
  leg->SetTextSize(0.03);
  leg->AddEntry(curvS[0],"Observed","L");
  leg->AddEntry(curvS[1],"Observed #pm1#sigma theory","L");
  TGraph* legGraph = (TGraph*) exp1sigma_aroundExp->Clone();
  legGraph->SetLineColor(kOrange+9);
  legGraph->SetLineStyle(9);
  legGraph->SetLineWidth(3);
  leg->AddEntry(legGraph,"Expected #pm1#sigma exp.","FL");
  leg->Draw("same");

  double xv = 0.25;
  double yv = 0.25;
  
  TLatex* lat4 = new TLatex(xv,yv,"Excluded");
  lat4->SetNDC(true);
  lat4->SetTextFont(43);
  lat4->SetTextSize(30);
  //lat4->Draw("same");

  can_excl02->RedrawAxis();

  if(print) {
    can_excl02->Print("",".pdf");
  }


  TCanvas * can_exclusionOnLimit = new TCanvas("can_exclusionOnLimit_"+scan,"can_exclusionOnLimit_"+scan,900,800);
  can_exclusionOnLimit->SetLogz();
  h_xs[1]->SetTitle(";M_{stop} [GeV];M_{Bino} [GeV];95% CL cross section upper limit [pb]");
  h_xs[1]->SetMinimum(0);
  //h_xs[1]->GetZaxis()->SetRangeUser(5.e-3, 2.1e-2);
  //h_xs[1]->GetZaxis()->SetNdivisions(210);
  h_xs[1]->Draw(option2D);

  curvS[0]->SetLineColor(kBlack);
  
  curvS[6]->Draw("SAME L");
  curvS[7]->Draw("SAME L");
  curvS[0]->Draw("SAME L");
  
  upperDiagonalRegion->SetFillColor(0);
  upperDiagonalRegion->Draw("same f");
  virtualLine->Draw("same");
  nlspLine->Draw("same");
  virtualComment->SetTextAngle(50);
  virtualComment->Draw("same");
  nlspComment->SetTextAngle(50);
  nlspComment->Draw("same");
  
  TLegend* leg2 = new TLegend(leg_xmin,leg_ymin,leg_xmax - 0.05,leg_ymax,legendTitle,"brNDC");
  leg2->SetFillStyle(0);
  leg2->SetFillColor(0);
  leg2->SetLineColor(0);
  leg2->SetBorderSize(0);
  leg2->SetTextFont(62);
  leg2->SetTextSize(0.03);
  //    leg2->AddEntry("NULL","NLO+NLL Limits","h");
  leg2->AddEntry("NULL", "m(#tilde{q}, #tilde{g}) >> m(#tilde{t})", "h");
  leg2->AddEntry(curvS[0],"#sigma^{NLO-QCD}","L");
  leg2->AddEntry(curvS[6],"3#times#sigma^{NLO-QCD}","L");
  leg2->AddEntry(curvS[7],"1/3#times#sigma^{NLO-QCD}","L");
  leg2->Draw("same");
  
  lat->Draw("same");
  lat2->Draw("same");
  
  can_exclusionOnLimit->RedrawAxis();
  
  if(print) {
    can_exclusionOnLimit->Print("",".pdf");
  }

  TCanvas * can_exclusionOnLimit_exp = new TCanvas("can_exclusionOnLimit_exp_"+scan, "can_exclusionOnLimit_exp_"+scan, 900, 800);
  can_exclusionOnLimit_exp->SetLogz();
  h_xs[2]->SetTitle(";M_{stop} [GeV];M_{Bino} [GeV];95% CL cross section upper limit [pb]");
  h_xs[2]->SetMinimum(0);
  h_xs[2]->GetZaxis()->SetRangeUser(1.e-3, 2.1e-2);
  //h_xs[2]->GetZaxis()->SetNdivisions(210);
  h_xs[2]->Draw(option2D);
  
  curvS[3]->SetLineColor(kBlack);
  curvS[3]->SetLineStyle(1);

  curvS[8]->Draw("SAME L");
  curvS[9]->Draw("SAME L");
  curvS[3]->Draw("SAME L");
  
  upperDiagonalRegion->SetFillColor(0);
  upperDiagonalRegion->Draw("same f");
  virtualLine->Draw("same");
  nlspLine->Draw("same");
  virtualComment->Draw("same");
  nlspComment->Draw("same");
  
  leg2->AddEntry("NULL", "Expected", "h");
  leg2->Draw("same");
  
  lat->Draw("same");
  lat2->Draw("same");
  
  can_exclusionOnLimit_exp->RedrawAxis();
  
  if(print) {
    can_exclusionOnLimit_exp->Print("",".pdf");
  }
  
  TFile* fsms = new TFile(output_dir+ "/hist_sms_gg_1jet_output.root","RECREATE");
  fsms->cd();
  h_xs[1]->Write("h2_95CL_limit");
  exp1sigma_aroundExp->Write("contour_exp_1s_band");
  curvS[3]->Write("contour_exp");
  curvS[4]->Write("contour_exp_1s_down");
  curvS[5]->Write("contour_exp_1s_up");
  curvS[1]->Write("contour_theory_obs_1s_down");
  curvS[2]->Write("contour_theory_obs_1s_up");
  curvS[0]->Write("contour_obs");
  curvS[6]->Write("contour_obs_up3");
  curvS[7]->Write("contour_obs_down3");
  curvS[8]->Write("contour_exp_up3");
  curvS[9]->Write("contour_exp_down3");
  h_xs[0]->Write("xsec");
  h_xs[1]->Write("observed_limit_in_xsecUnit");
  h_xs[2]->Write("expected_limit_in_xsecUnit");
  h_limit[0]->Write("observed_limit_in_R");
  h_limit[1]->Write("observed_limit_1s_down_in_R");
  h_limit[2]->Write("observed_limit_1s_up_in_R");
  h_limit[3]->Write("expected_limit_in_R");
  h_limit[4]->Write("expected_limit_1s_down_in_R");
  h_limit[5]->Write("expected_limit_1s_up_in_R");
  can_exclusionOnLimit->Write();
  can_exclusionOnLimit_exp->Write();
  fsms->Write();
  fsms->Close();
  
  fout->cd();
  fout->Write();
  
  can_limit->Write();
  can_excl01->Write();
  can_excl02->Write();
  can_exclusionOnLimit->Write();
  
  for(int i=0; i<nlimit; i++){
    curv[i]->Write();
    curvS[i]->Write();
  }


}



