#include "TROOT.h"
#include <TStyle.h>
#include <TFile.h>
#include <TH1D.h>
#include <TH2D.h>
#include <TString.h>
#include <TCanvas.h>
#include <TMath.h>

#include <iostream>
#include <fstream>
#include <istream>
#include <sstream>
#include <vector>
#include <cmath>

using namespace std;

const int nBackgrounds = 9;
const TString backgroundNames[nBackgrounds] = {"ttjets", "wjets", "zjets", "singleTop", "diboson", "ttW", "ttZ", "ttgamma", "vgamma"};

const bool scale_tt[nBackgrounds] = {true, false, false, true, false, true, true, true, false};
const bool scale_V[nBackgrounds]  = {false, true, true, false, false, false, false, false, false};
const bool scale_VV[nBackgrounds] = {false, false, false, false, true, false, false, false, true};

const bool pdf_gg[nBackgrounds]   = {true, false, false, false, false, false, false/*no pdf given for ttZ, but it would be here*/, true, false};
const bool pdf_qq[nBackgrounds]   = {false, true, true, false, true, true, false, false, true};
const bool pdf_qg[nBackgrounds]   = {false, false, false, true, false, false, false, false, false};

const int nSystematics = 6;
const TString systematicNames[nSystematics] = {"btagWeight", "puWeight", "JEC", "eleSF", "muonSF", "photonSF"};

const double epsilon = 1e-10;

const TString datacard_dir = "datacards";

class GridPoint {

 public:
  GridPoint(TString fileNameCode_);
  virtual ~GridPoint() {
    channels.clear();
    useQCD.clear();
    nBins.clear();
    backgroundYields.clear();
    
    useStatErrors.clear();
    useQCDStatErrors.clear();
    val.clear();
    val_err.clear();

    bkg.clear();
    bkg_err.clear();
    data_err.clear();
    sig.clear();

    qcd.clear();
    qcd_err.clear();

    isSensitive.clear();
    signalYields.clear();
    obs.clear();

    // chan
    qcdYields.clear();
    // chan/bin
    qcd.clear();
    qcd_err.clear();
  }

  void SetUseExtraFloat(bool v) { useExtraFloat = v; };

  void AddChannel(TString chan, bool needsQCD, int nBins_) { 
    channels.push_back(chan);
    useQCD.push_back(needsQCD);
    nBins.push_back(nBins_);
  };

  void Init() {

    // chan/bkg
    vector<double> bkgY(nBackgrounds, 0.);
    for(unsigned int i = 0; i < channels.size(); i++) backgroundYields.push_back(bkgY);

    // chan/bkg/bin
    for(unsigned int i = 0; i < channels.size(); i++) {
      vector< vector<bool> > useSt(nBackgrounds, vector<bool>(nBins[i], false));
      useStatErrors.push_back(useSt);
      
      vector< vector<double> > vl(nBackgrounds, vector<double>(nBins[i], 0.));
      val.push_back(vl);

      vector< vector<double> > vle(nBackgrounds, vector<double>(nBins[i], 0.));
      val_err.push_back(vle);
    }

    // chan/bin
    for(unsigned int i = 0; i < channels.size(); i++) {
      vector<double> bg(nBins[i], 0.);
      bkg.push_back(bg);
      
      vector<double> bge(nBins[i], 0.);
      bkg_err.push_back(bge);
      
      vector<double> de(nBins[i], 0.);
      data_err.push_back(de);
      
      vector<double> sg(nBins[i], 0.);
      sig.push_back(sg);
    }

    // chan
    signalYields.resize(channels.size());
    isSensitive.resize(channels.size());
    obs.resize(channels.size());

    // chan
    qcdYields.resize(channels.size());

    // chan/bin
    for(unsigned int i = 0; i < channels.size(); i++) {
      vector<bool> useQCDSt(nBins[i], false);
      useQCDStatErrors.push_back(useQCDSt);

      vector<double> v_qcd(nBins[i], 0.);
      qcd.push_back(v_qcd);

      vector<double> v_qcde(nBins[i], 0.);
      qcd_err.push_back(v_qcde);
    }

  }

  void Print();
  bool SetBackgroundYields(TFile * f);
  bool SetSignalYields(TFile * f);
  void SetUseStatError();

  int mStop;
  int mBino;

  double xsec;
  double xsecError;

  double lumi_sysError;

  vector<TString> channels;
  vector<bool> useQCD;
  vector<int> nBins;

  vector< vector<double> > backgroundYields;
  vector<double> signalYields;
  vector<bool> isSensitive;
  vector<double> obs;

  vector<double> qcdYields;

  vector< vector< vector<bool> > > useStatErrors;
  vector< vector< vector<double> > > val;
  vector< vector< vector<double> > > val_err;

  vector< vector<double> > bkg;
  vector< vector<double> > bkg_err;
  vector< vector<double> > data_err;
  vector< vector<double> > sig;

  vector< vector<bool> > useQCDStatErrors;
  vector< vector<double> > qcd;
  vector< vector<double> > qcd_err;

  TString fileNameCode;

  bool useExtraFloat;

  double limit;           // observed limit
  double explimit;        // expected limit
  double explimit_1L;     // expected limit -1 sigma
  double explimit_1H;     // expected limit +1 sigma
  double explimit_2L;     // expected limit -2 sigma
  double explimit_2H;     // expected limit +2 sigma
};

GridPoint::GridPoint(TString fileNameCode_) {

  fileNameCode = fileNameCode_;

  mStop = mBino = 0;

  lumi_sysError = 1.026;

  useExtraFloat = false;

  channels.clear();
  useQCD.clear();
  backgroundYields.clear();
  
  useStatErrors.clear();
  val.clear();
  val_err.clear();
  
  bkg.clear();
  bkg_err.clear();
  data_err.clear();
  sig.clear();
  
  isSensitive.clear();
  signalYields.clear();
  obs.clear();

  qcdYields.clear();
  qcd.clear();
  qcd_err.clear();
  useQCDStatErrors.clear();
    
  limit = explimit = explimit_1L = explimit_1H = explimit_2L = explimit_2H = 0;
}

void GridPoint::Print() {

  stringstream outname;
  outname << datacard_dir.Data() << "/stop-bino_mst_" << mStop << "_m1_" << mBino << "_" << fileNameCode << ".dat";
  fstream outfile(outname.str().c_str(), ios::out);

  outfile << "# stop = " << mStop << endl;
  outfile << "# bino = " << mBino << endl;
  outfile << "# xsec = " << xsec << endl;
  outfile << "# xsec uncertainty = " << xsecError << " %" << endl;

  outfile << endl << "imax * number of channels" << endl;
  outfile << endl << "jmax * number of backgrounds" << endl;
  outfile << "kmax * number of nuisance parameters" << endl;
  outfile << "--------------------" << endl;
  outfile << "shapes * * limitInputs_bjj.root $CHANNEL/$PROCESS  $CHANNEL/$PROCESS_$SYSTEMATIC" << endl;
  outfile << "--------------------" << endl;

  outfile << "bin                 ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) outfile << "\t" << channels[i];
  }
  outfile << endl;

  outfile << "observation         ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) outfile << "\t" << obs[i];
  }
  outfile << endl;

  outfile << "--------------------" << endl;
  outfile << "bin                 ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      int nTimes = (useQCD[i]) ? nBackgrounds + 2 : nBackgrounds + 1;
      for(int j = 0; j < nTimes; j++) outfile << "\t" << channels[i];
    }
  }
  outfile << endl;

  stringstream code;
  code << "_mst_" << mStop << "_m1_" << mBino;
    
  outfile << "process             ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\tsignal" << code.str();
      for(int j = 0; j < nBackgrounds; j++) outfile << "\t" << backgroundNames[j];
      if(useQCD[i]) outfile << "\tqcd";
    }
  }
  outfile << endl;

  outfile << "process             ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      int nTimes = (useQCD[i]) ? nBackgrounds + 2 : nBackgrounds + 1;
      for(int j = 0; j < nTimes; j++) outfile << "\t" << j;
    }
  }
  outfile << endl;

  outfile << "rate                ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t" << signalYields[i];
      for(int j = 0; j < nBackgrounds; j++) outfile << "\t" << backgroundYields[i][j];
      if(useQCD[i]) outfile << "\t" << qcdYields[i];
    }
  }
  outfile << endl;
  outfile << "--------------------" << endl;

  outfile << "lumi lnN          ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      for(int j = 0; j < nBackgrounds + 1; j++) outfile << "\t" << lumi_sysError;
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  for(int iS = 0; iS < nSystematics; iS++) {

    outfile << systematicNames[iS].Data() << " shape          ";
    for(unsigned int i = 0; i < channels.size(); i++) {
      if(isSensitive[i]) {

	for(int j = 0; j < nBackgrounds + 1; j++) {
	  if(systematicNames[iS] == "eleSF") {
	    if(channels[i].Contains("ele")) outfile << "\t1";
	    else outfile << "\t-";
	  }
	  else if(systematicNames[iS] == "muonSF") {
	    if(channels[i].Contains("muon")) outfile << "\t1";
	    else outfile << "\t-";
	  }
	  else outfile << "\t1";
	}
	if(useQCD[i]) outfile << "\t-";

      }
    }
    outfile << endl;

  }

  outfile << "topPt shape          ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(backgroundNames[j] == "ttjets") outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";

    }
  }
  outfile << endl;

  outfile << "userSystA_ele shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      if(channels[i] == "ele_SR1" || channels[i] == "ele_SR2") {
	for(int j = 0; j < nTimes; j++) outfile << "\t1";
      }
      else {
	for(int j = 0; j < nTimes; j++) outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "userSystA_muon shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      if(channels[i] == "muon_SR1" || channels[i] == "muon_SR2") {
	for(int j = 0; j < nTimes; j++) outfile << "\t1";
      }
      else {
	for(int j = 0; j < nTimes; j++) outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "userSystB_ele shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      if(channels[i] == "ele_SR1" || channels[i] == "ele_SR2") {
	for(int j = 0; j < nTimes; j++) outfile << "\t1";
      }
      else {
	for(int j = 0; j < nTimes; j++) outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "userSystB_muon shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      if(channels[i] == "muon_SR1" || channels[i] == "muon_SR2") {
	for(int j = 0; j < nTimes; j++) outfile << "\t1";
      }
      else {
	for(int j = 0; j < nTimes; j++) outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "userSystC_ele shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      if(channels[i] == "ele_SR2") {
	for(int j = 0; j < nTimes; j++) outfile << "\t1";
      }
      else {
	for(int j = 0; j < nTimes; j++) outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "userSystC_muon shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      if(channels[i] == "muon_SR2") {
	for(int j = 0; j < nTimes; j++) outfile << "\t1";
      }
      else {
	for(int j = 0; j < nTimes; j++) outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "scale_tt shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(scale_tt[j]) outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "scale_V shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(scale_V[j]) outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "scale_VV shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(scale_VV[j]) outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "pdf_gg shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(pdf_gg[j]) outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "pdf_qq shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(pdf_qq[j]) outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "pdf_qg shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {
	if(pdf_qg[j]) outfile << "\t1";
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "susy_xsec lnN     ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      outfile << "\t" << 1. + xsecError/100.;
      int nTimes = (useQCD[i]) ? nBackgrounds + 1 : nBackgrounds;
      for(int j = 0; j < nTimes; j++) outfile << "\t-";
    }
  }
  outfile << endl;

  outfile << "ele_qcdDef shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      for(int j = 0; j < nBackgrounds + 1; j++) outfile << "\t-";
      if(useQCD[i]) {
	if(channels[i].Contains("ele")) outfile << "\t1";
	else outfile << "\t-";
      }
    }
  }
  outfile << endl;

  outfile << "muon_qcdDef shape ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      for(int j = 0; j < nBackgrounds + 1; j++) outfile << "\t-";
      if(useQCD[i]) {
	if(channels[i].Contains("muon")) outfile << "\t1";
	else outfile << "\t-";
      }
    }
  }
  outfile << endl;

  for(unsigned int ichan = 0; ichan < channels.size(); ichan++) {

    for(int ibin = 0; ibin < nBins[ichan]; ibin++) {

      for(int ibkg = 0; ibkg < nBackgrounds; ibkg++) {

	if(!useStatErrors[ichan][ibkg][ibin]) continue;

	if(isSensitive[ichan]) {

	  outfile << backgroundNames[ibkg] << "_" << channels[ichan] << "_stat_bin" << ibin + 1 << " shape";
	  for(unsigned int jchan = 0; jchan < channels.size(); jchan++) {
	    if(isSensitive[jchan]) {
	      outfile << "\t-";
	      for(int jbkg = 0; jbkg < nBackgrounds; jbkg++) {
		if(jbkg == ibkg && jchan == ichan) outfile << "\t1";
		else outfile << "\t-";
	      }
	      if(useQCD[jchan]) outfile << "\t-";
	    }
	  }
	  outfile << endl;

	}

      } // for backgrounds
      
    } // for bins

  } // for channels

  for(unsigned int ichan = 0; ichan < channels.size(); ichan++) {
    if(!useQCD[ichan]) continue;

    for(int ibin = 0; ibin < nBins[ichan]; ibin++) {

      if(!useQCDStatErrors[ichan][ibin]) continue;

      if(isSensitive[ichan]) {

	outfile << "qcd_" << channels[ichan] << "_stat_bin" << ibin + 1 << " shape";
	for(unsigned int jchan = 0; jchan < channels.size(); jchan++) {
	  if(isSensitive[jchan]) {
	    for(int jbkg = 0; jbkg < nBackgrounds + 1; jbkg++) outfile << "\t-";
	    if(jchan == ichan) outfile << "\t1";
	    else outfile << "\t-";
	  }
	}
	outfile << endl;

      }

    } // for qcd bins

  } // for channels (qcd)

  // photon purity fit systematic

  double fitError_ttjets_ele_SR1 = fabs(1.08939867913 - 1.22523777194) / 1.08939867913 + 1.;
  double fitError_ttjets_ele_SR2 = fabs(1.08939867913*1.08939867913 - 1.22523777194*1.22523777194) / 1.08939867913 / 1.08939867913 + 1.;

  double fitError_ttjets_muon_SR1 = fabs(1.02019169962 - 1.20688543609) / 1.02019169962 + 1.;
  double fitError_ttjets_muon_SR2 = fabs(1.02019169962*1.02019169962 - 1.20688543609*1.20688543609) / 1.02019169962*1.02019169962 + 1.;


  double fitError_ttgamma_ele_SR1 = fabs(1.16586766438 - 1.03926230015) / 1.16586766438 + 1.;
  double fitError_ttgamma_ele_SR2 = fabs(1.16586766438*1.16586766438 - 1.03926230015*1.03926230015) / 1.16586766438*1.16586766438 + 1.;

  double fitError_ttgamma_muon_SR1 = fabs(1.13594181881 - 0.957076347477) / 1.13594181881 + 1.;
  double fitError_ttgamma_muon_SR2 = fabs(1.13594181881*1.13594181881 - 0.957076347477*0.957076347477) / 1.13594181881*1.13594181881 + 1.;

  outfile << "u_ttjets_fit_ele lnN ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {

	if(backgroundNames[j] == "ttjets") {
	  if(channels[i] == "ele_SR1") outfile << "\t" << fitError_ttjets_ele_SR1;
	  if(channels[i] == "ele_SR2") outfile << "\t" << fitError_ttjets_ele_SR2;
	  else outfile << "\t-";
	}
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";

    }
  }
  outfile << endl;

  outfile << "u_ttjets_fit_muon lnN ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {

	if(backgroundNames[j] == "ttjets") {
	  if(channels[i] == "muon_SR1") outfile << "\t" << fitError_ttjets_muon_SR1;
	  if(channels[i] == "muon_SR2") outfile << "\t" << fitError_ttjets_muon_SR2;
	  else outfile << "\t-";
	}
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";

    }
  }
  outfile << endl;

  outfile << "u_ttgamma_fit_ele lnN ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {

	if(backgroundNames[j] == "ttgamma") {
	  if(channels[i] == "ele_SR1") outfile << "\t" << fitError_ttgamma_ele_SR1;
	  if(channels[i] == "ele_SR2") outfile << "\t" << fitError_ttgamma_ele_SR2;
	  else outfile << "\t-";
	}
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";

    }
  }
  outfile << endl;

  outfile << "u_ttgamma_fit_muon lnN ";
  for(unsigned int i = 0; i < channels.size(); i++) {
    if(isSensitive[i]) {
      
      outfile << "\t-";
      for(int j = 0; j < nBackgrounds; j++) {

	if(backgroundNames[j] == "ttgamma") {
	  if(channels[i] == "muon_SR1") outfile << "\t" << fitError_ttgamma_muon_SR1;
	  if(channels[i] == "muon_SR2") outfile << "\t" << fitError_ttgamma_muon_SR2;
	  else outfile << "\t-";
	}
	else outfile << "\t-";
      }
      if(useQCD[i]) outfile << "\t-";

    }
  }
  outfile << endl;

  if(useExtraFloat) {

    outfile << "float_ttg_ttjets_sr1 lnU";
    for(unsigned int i = 0; i < channels.size(); i++) {
      if(isSensitive[i]) {
	outfile << "\t-";
	for(unsigned int j = 0; j < nBackgrounds; j++) {
	  if(channels[i].Contains("SR1") && (backgroundNames[j] == "ttjets" || backgroundNames[j] == "ttgamma")) outfile << "\t2";
	  else outfile << "\t-";
	}
	if(useQCD[i]) outfile << "\t-";
      }
    }
    outfile << endl;

    outfile << "float_ttg_ttjets_sr2 lnU";
    for(unsigned int i = 0; i < channels.size(); i++) {
      if(isSensitive[i]) {
	outfile << "\t-";
	for(unsigned int j = 0; j < nBackgrounds; j++) {
	  if(channels[i].Contains("SR2") && (backgroundNames[j] == "ttjets" || backgroundNames[j] == "ttgamma")) outfile << "\t2";
	  else outfile << "\t-";
	}
	if(useQCD[i]) outfile << "\t-";
      }
    }
    outfile << endl;

  } // use extra float if(useExtraFloat)

} // Print()

bool GridPoint::SetBackgroundYields(TFile * f) {

  for(unsigned int i = 0; i < channels.size(); i++) {

    TH1D * h = (TH1D*)f->Get(channels[i]+"/data_obs");
    if(!h) return false;
    obs[i] = h->Integral();
    for(int ibin = 0; ibin < nBins[i]; ibin++) data_err[i][ibin] = h->GetBinError(ibin+1);

    for(int j = 0; j < nBackgrounds; j++) {
      h = (TH1D*)f->Get(channels[i]+"/"+backgroundNames[j]);
      if(!h) return false;
      backgroundYields[i][j] = h->Integral();
      for(int ibin = 0; ibin < nBins[i]; ibin++) {
	val[i][j][ibin] = h->GetBinContent(ibin+1);
	val_err[i][j][ibin] = h->GetBinError(ibin+1);

	bkg[i][ibin] += h->GetBinContent(ibin+1);
	bkg_err[i][ibin] = TMath::Sqrt(bkg_err[i][ibin]*bkg_err[i][ibin] + h->GetBinError(ibin+1)*h->GetBinError(ibin+1));
      }

    }

    h = (TH1D*)f->Get(channels[i]+"/qcd");
    if(!h) return false;
    qcdYields[i] = h->Integral();
    for(int ibin = 0; ibin < nBins[i]; ibin++) {
      qcd[i][ibin] = h->GetBinContent(ibin+1);
      qcd_err[i][ibin] = h->GetBinError(ibin+1);

      bkg[i][ibin] += h->GetBinContent(ibin+1);
      bkg_err[i][ibin] = TMath::Sqrt(bkg_err[i][ibin]*bkg_err[i][ibin] + h->GetBinError(ibin+1)*h->GetBinError(ibin+1));
    }


  }

  return true;
}

bool GridPoint::SetSignalYields(TFile * f) {
  
  stringstream code;
  code << "_mst_" << mStop << "_m1_" << mBino;
  TString code_t = code.str();

  for(unsigned int i = 0; i < channels.size(); i++) {
    TH1D * h = (TH1D*)f->Get(channels[i]+"/signal"+code_t);
    if(!h) return false;
    signalYields[i] = h->Integral();
    isSensitive[i] = (h->Integral() > epsilon);
    for(int ibin = 0; ibin < nBins[i]; ibin++) sig[i][ibin] = h->GetBinContent(ibin+1);
  }

  return true;
}

void GridPoint::SetUseStatError() {
  
  for(unsigned int chan = 0; chan < channels.size(); chan++) {

    for(int ibkg = 0; ibkg < nBackgrounds; ibkg++) {

      for(int bin = 0; bin < nBins[chan]; bin++) {
	bool negligable = val[chan][ibkg][bin] < 0.01 ||
	  bkg_err[chan][bin] < data_err[chan][bin] / 5. ||
	  TMath::Sqrt(bkg_err[chan][bin]*bkg_err[chan][bin] - val_err[chan][ibkg][bin]*val_err[chan][ibkg][bin]) / bkg_err[chan][bin] > 0.95 ||
	  sig[chan][bin] / bkg[chan][bin] < 0.01;

	useStatErrors[chan][ibkg][bin] = !negligable;
      }

    }

    for(int bin = 0; bin < nBins[chan]; bin++) {
      bool negligable = qcd[chan][bin] < 0.01 ||
	bkg_err[chan][bin] < data_err[chan][bin] / 5. ||
	TMath::Sqrt(bkg_err[chan][bin]*bkg_err[chan][bin] - qcd_err[chan][bin]*qcd_err[chan][bin]) / bkg_err[chan][bin] > 0.95 ||
	sig[chan][bin] / bkg[chan][bin] < 0.01;

      useQCDStatErrors[chan][bin] = !negligable;
    }

  }

}
