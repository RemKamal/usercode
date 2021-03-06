#include "CalcAccAndEff.h" 
  //this class
#include "../Include/TMathTools.h"
#include "../Include/TMetTools.h" 
#include "../Include/TMuonCuts.h" 
#include "../Include/TElectronCuts.h" 
#include "../Include/TPhotonCuts.h" 
#include "../Include/TFullCuts.h" 
#include "../Include/TPuReweight.h" 
#include "../Configuration/TConfiguration.h" 
#include "../Configuration/TInputSample.h"
#include "../Configuration/TAllInputSamples.h"
  //this package
#include "../PHOSPHOR_CORRECTION/PhosphorCorrectorFunctor.hh"
  //taken from git
#include "TMath.h" 
#include "TH1F.h" 
#include "TVectorF.h" 
#include "TCanvas.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TLegend.h"
  // ROOT class
#include <iostream> 
#include <string>
#include <sstream>  
  //standard C++ class

CalcAccAndEff::CalcAccAndEff(int year, int channel, int vgamma, int phoWP, string configFile, bool isNoPuReweight, bool isDebugMode)
{

  _INPUT = new TAllInputSamples(channel, vgamma, configFile);

  _year=year;
  _channel=channel;
  _vgamma=vgamma;
  _phoWP=phoWP;
  _isDebugMode=isDebugMode;
  _isNoPuReweight=isNoPuReweight;
  _photonCorrector = new zgamma::PhosphorCorrectionFunctor((_config.GetPhosphorConstantFileName()) );
    //field of this class

  _nAccTotEvents=0;
  _nAccTotEventsErr=0;
  _nAccTotPassed=0;
  _nAccTotPassedErr=0;
  _accTot=-1;
  _accTotErr=-1;

  _nEffTotEvents=0;
  _nEffTotEventsErr=0;
  _nEffTotPrePassed=0;
  _nEffTotPrePassedErr=0;
  _nEffTotPrePassed2=0;
  _nEffTotPrePassed2Err=0;
  _nEffTotPassed=0;
  _nEffTotPassedErr=0;
  _effTot=-1;
  _effTotErr=-1;

  _nAcc1DEvents=new float[_config.GetNPhoPtBins()];
  _nAcc1DEventsErr=new float[_config.GetNPhoPtBins()];
  _nAcc1DPassed=new float[_config.GetNPhoPtBins()];
  _nAcc1DPassedErr=new float[_config.GetNPhoPtBins()];
  _acc1D=new float[_config.GetNPhoPtBins()];
  _acc1DErr=new float[_config.GetNPhoPtBins()];

  _nEff1DEvents=new float[_config.GetNPhoPtBins()];
  _nEff1DEventsErr=new float[_config.GetNPhoPtBins()];
  _nEff1DPrePassed=new float[_config.GetNPhoPtBins()];
  _nEff1DPrePassedErr=new float[_config.GetNPhoPtBins()];
  _nEff1DPrePassed2=new float[_config.GetNPhoPtBins()];
  _nEff1DPrePassed2Err=new float[_config.GetNPhoPtBins()];
  _nEff1DPassed=new float[_config.GetNPhoPtBins()];
  _nEff1DPassedErr=new float[_config.GetNPhoPtBins()];
  _eff1D=new float[_config.GetNPhoPtBins()];
  _eff1DErr=new float[_config.GetNPhoPtBins()];

  _accXeff1D=new float[_config.GetNPhoPtBins()];
  _accXeff1DErr=new float[_config.GetNPhoPtBins()];

  for (int i=0; i<_config.GetNPhoPtBins(); i++){
    _nAcc1DEvents[i]=0;
    _nAcc1DEventsErr[i]=0;
    _nAcc1DPassed[i]=0;
    _nAcc1DPassedErr[i]=0;
    _acc1D[i]=-1;
    _acc1DErr[i]=-1;
    _nEff1DEvents[i]=0;
    _nEff1DEventsErr[i]=0;
    _nEff1DPrePassed[i]=0;
    _nEff1DPrePassedErr[i]=0;
    _nEff1DPrePassed2[i]=0;
    _nEff1DPrePassed2Err[i]=0;
    _nEff1DPassed[i]=0;
    _nEff1DPassedErr[i]=0;
    _eff1D[i]=-1;
    _eff1DErr[i]=-1;
  }

  _phoPtLimits = new float[_config.GetNPhoPtBins()+1];
  _config.GetPhoPtBinsLimits(_phoPtLimits);

}

CalcAccAndEff::~CalcAccAndEff()
{
   _eventTree.fChain = 0;
   //field of TEventTree 
   delete _photonCorrector;
}

void CalcAccAndEff::LoopOverInputFiles()
{
  int nSources = _INPUT->nSources_;
  for (int iSource=0; iSource<nSources; iSource++){
    int sample = _INPUT->allInputs_[iSource].sample_;
    if (sample!=_config.SIGMC) continue;
      // acceptance and efficiency are calculated only through signal MC

    TTree* tree;
    int inputFileNMax = _INPUT->allInputs_[iSource].nFiles_;

    for (int inputFileN=0; inputFileN<inputFileNMax; inputFileN++){
      _lumiWeight=_INPUT->allInputs_[iSource].lumiWeights_[inputFileN];
      TFile f((_INPUT->allInputs_[iSource].fileNames_[inputFileN]) );
      if (f.IsOpen()) 
        std::cout<<std::endl<<"processing file "<<_INPUT->allInputs_[iSource].fileNames_[inputFileN]<<std::endl;
      else{
        std::cout<<"ERROR detected in CalcAccAndEff::LoopOverInputFiles: file "<<_INPUT->allInputs_[iSource].fileNames_[inputFileN]<<" was not found"<<std::endl;
        return;
      } 
      f.cd("ggNtuplizer");
      tree = (TTree*)gDirectory->Get("EventTree");

      if (tree){
        _eventTree.Init(tree);
        //if (sample_!=TInputSample::DATA) SetMCSpecificAddresses();
      }   
      else{
        std::cout<<"ERROR detected in CalcAccAndEff::LoopOverInputFiles: tree in the file "<<_INPUT->allInputs_[iSource].fileNames_[inputFileN]<<" does not exist"<<std::endl;
        return;
      }  

      bool isSkimmed = gDirectory->GetListOfKeys()->Contains("hskim");

      if (isSkimmed){
        std::cout<<"ERROR detected in CalcAccAndEff::LoopOverInputFiles: file"<<_INPUT->allInputs_[iSource].fileNames_[inputFileN]<<" is skimmed; skimmed signal MC file can't be used for acceptance and efficiency calculation; please, use the full signal MC file"<<std::endl;
        return;
      }

      _puWeight=new TPuReweight(_config.GetPileupDataFileName(),_INPUT->allInputs_[iSource].fileNames_[inputFileN]);

      LoopOverTreeEvents();
        //method of this class (CalcAccAndEff)
      _eventTree.fChain=0;
        //field of TEventTree

      delete _puWeight;

    }//loop over inputFileN_ ends

    //Acceptance
    ComputeAcceptance();

    //Efficiency
    ComputeEfficiency();

    //Acc X Eff together
    ComputeAccTimesEff();

    PlotAndSaveOutput();

    PrintAccAndEffSummary();

    std::cout<<std::endl;
    std::cout<<"the output will be saved to "<<std::endl<<std::endl;
  } //loop over iSource ends
}

void CalcAccAndEff::ComputeAcceptance()
{
    if (_nAccTotEventsErr>0) _nAccTotEventsErr=sqrt(_nAccTotEventsErr);
    if (_nAccTotPassedErr>0) _nAccTotPassedErr=sqrt(_nAccTotPassedErr);
    for (int i=0; i<_config.GetNPhoPtBins(); i++){
      if (_nAcc1DEventsErr[i]>0) _nAcc1DEventsErr[i]=sqrt(_nAcc1DEventsErr[i]);
      if (_nAcc1DPassedErr[i]>0) _nAcc1DPassedErr[i]=sqrt(_nAcc1DPassedErr[i]);
    }
    if (_nAccTotEvents!=0){ 
      _accTot=_nAccTotPassed/_nAccTotEvents;
      _accTotErr=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nAccTotPassed,_nAccTotEvents,_nAccTotPassedErr,_nAccTotEventsErr);
    }
    for (int i=0; i<_config.GetNPhoPtBins(); i++){
      if (_nAcc1DEvents[i]!=0){
        _acc1D[i]=_nAcc1DPassed[i]/_nAcc1DEvents[i];
        _acc1DErr[i]=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nAcc1DPassed[i],_nAcc1DEvents[i],_nAcc1DPassedErr[i],_nAcc1DEventsErr[i]);
      }
    }
}

void CalcAccAndEff::ComputeEfficiency()
{
    if (_nEffTotEventsErr!=0) _nEffTotEventsErr=sqrt(_nEffTotEventsErr);
    if (_nEffTotPassedErr!=0) _nEffTotPrePassedErr=sqrt(_nEffTotPrePassedErr);
    for (int i=0; i<_config.GetNPhoPtBins(); i++){
      if (_nEff1DEventsErr[i]!=0) _nEff1DEventsErr[i]=sqrt(_nEff1DEventsErr[i]);
      if (_nEff1DPassedErr[i]!=0) _nEff1DPrePassedErr[i]=sqrt(_nEff1DPrePassedErr[i]);
    }

    TFile* fVeryPreliminary = new TFile(_config.GetSelectedName(_config.VERY_PRELIMINARY, _channel,_vgamma,_config.UNBLIND, _config.SIGMC));
    TTree* tr = (TTree*)fVeryPreliminary->Get("selectedEvents");
    TH1F* hTot = new TH1F("hTot","hTot",1,_config.GetPhoPtMin(),_config.GetPhoPtMax());
    TH1F* h1D = new TH1F("h1D","h1D",_config.GetNPhoPtBins(),_phoPtLimits);
    TCut cutWeight="PUweight";//"weight";
    TCut cut = cutWeight*_fullCuts.RangeFullCut(_year,_channel,_vgamma,_config.UNBLIND,_phoWP);
    tr->Draw("phoGenEt>>hTot",cutWeight,"goff");
    tr->Draw("phoGenEt>>h1D",cutWeight,"goff");
    std::cout<<"file for efficiency selection: "<<_config.GetSelectedName(_config.VERY_PRELIMINARY,_channel,_vgamma,_config.UNBLIND,_config.SIGMC)<<std::endl;
    std::cout<<"preselected tree has "<<tr->GetEntries()<<" events"<<std::endl;
    std::cout<<"tree with full cut has "<<tr->GetEntries(cut)<<"  events"<<std::endl;
    std::cout<<"Print histograms:"<<std::endl;
    h1D->Print();
    hTot->Print();
    _nEffTotPrePassed2=hTot->GetBinContent(1);
    _nEffTotPrePassed2Err=hTot->GetBinError(1);
    for (int i=1; i<=_config.GetNPhoPtBins(); i++){
      _nEff1DPrePassed2[i-1]=h1D->GetBinContent(i);
      _nEff1DPrePassed2Err[i-1]=h1D->GetBinError(i);
    }
    tr->Draw("phoGenEt>>hTot",cut,"goff");
    tr->Draw("phoGenEt>>h1D",cut,"goff");
    _nEffTotPassed=hTot->GetBinContent(1);
    _nEffTotPassedErr=hTot->GetBinError(1);
    for (int i=1; i<=_config.GetNPhoPtBins(); i++){
      _nEff1DPassed[i-1]=h1D->GetBinContent(i);
      _nEff1DPassedErr[i-1]=h1D->GetBinError(i);
    }

//     if (_nEffTotEvents!=0){ 
//       _effTot=(double)_nEffTotPassed/_nEffTotEvents;
//       _effTotErr=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nEffTotPassed,_nEffTotEvents,_nEffTotPassedErr,_nEffTotEventsErr);
//     }

//     for (int i=0; i<_config.GetNPhoPtBins(); i++){
//       _eff1D[i]=_nEff1DPassed[i]/_nEff1DEvents[i];
//       _eff1DErr[i]=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nEff1DPassed[i],_nEff1DEvents[i],_nEff1DPassedErr[i],_nEff1DEventsErr[i]);
//     }

    if (_nEffTotPrePassed2==0){
      _effTot=-1;
      _effTotErr=1;      
    }
    else{
      _effTot=(double)_nEffTotPassed/_nEffTotPrePassed2;
      _effTotErr=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nEffTotPassed,_nEffTotPrePassed2,_nEffTotPassedErr,_nEffTotPrePassed2Err);
    }

    for (int i=0; i<_config.GetNPhoPtBins(); i++){
      if (_nEff1DPrePassed2[i]==0){
        _eff1D[i]=-1;
        _eff1DErr[i]=1;
      }
      else{
        _eff1D[i]=_nEff1DPassed[i]/_nEff1DPrePassed2[i];
        _eff1DErr[i]=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nEff1DPassed[i],_nEff1DPrePassed2[i],_nEff1DPassedErr[i],_nEff1DPrePassed2Err[i]);
      }
    }
}

void CalcAccAndEff::ComputeAccTimesEff()
{
    if (_nAccTotEvents!=0){ 
      _accXeffTot=1.0*_nEffTotPassed/_nAccTotEvents;
      _accXeffTotErr=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nEffTotPassed,_nAccTotEvents,_nEffTotPassedErr,_nAccTotEventsErr);
    }
    for (int i=0; i<_config.GetNPhoPtBins(); i++){
      if (_nAcc1DEvents[i]!=0){
        _accXeff1D[i]=1.0*_nEff1DPassed[i]/_nAcc1DEvents[i];
        _accXeff1DErr[i]=_math.ErrOfTwoIndependent("x1/(x1+x2)",_nEff1DPassed[i],_nAcc1DEvents[i],_nEff1DPassedErr[i],_nAcc1DEventsErr[i]);
      }
    }
}

void CalcAccAndEff::PrintAccAndEffSummary()
{
    std::cout<<"Summary:"<<std::endl;
    std::cout<<"Acc: nEvents="<<_nAccTotEvents<<"+-"<<_nAccTotEventsErr;
    std::cout<<", nPassed="<<_nAccTotPassed<<"+-"<<_nAccTotPassedErr<<", acc="<<_accTot<<"+-"<<_accTotErr<<std::endl;
    std::cout<<"Eff: nEvents="<<_nEffTotEvents<<"+-"<<_nEffTotEventsErr;
    std::cout<<", nPrePassed="<<_nEffTotPrePassed<<"+-"<<_nEffTotPrePassedErr;
    std::cout<<", nPrePassed2="<<_nEffTotPrePassed2<<"+-"<<_nEffTotPrePassed2Err;
    std::cout<<", nPassed="<<_nEffTotPassed<<"+-"<<_nEffTotPassedErr;
    std::cout<<", eff="<<_effTot<<"+-"<<_effTotErr;
    std::cout<<", acc x eff="<<_accTot*_effTot<<", accXeff="<<_accXeffTot<<"+-"<<_accXeffTotErr<<std::endl;
    std::cout<<std::endl;

    for (int i=0; i<_config.GetNPhoPtBins(); i++){
      std::cout<<"pho Pt: "<<_phoPtLimits[i]<<" - "<<_phoPtLimits[i+1]<<std::endl;
      std::cout<<"Acc: nEvents="<<_nAcc1DEvents[i]<<"+-"<<_nAcc1DEventsErr[i];
      std::cout<<", nPassed="<<_nAcc1DPassed[i]<<"+-"<<_nAcc1DPassedErr[i]<<", acc="<<_acc1D[i]<<"+-"<<_acc1DErr[i]<<std::endl;
      std::cout<<"Eff: nEvents="<<_nEff1DEvents[i]<<"+-"<<_nEff1DEventsErr[i];
      std::cout<<", nPrePassed="<<_nEff1DPrePassed[i]<<"+-"<<_nEff1DPrePassedErr[i];
      std::cout<<", nPrePassed2="<<_nEff1DPrePassed2[i]<<"+-"<<_nEff1DPrePassed2Err[i];
      std::cout<<", nPassed="<<_nEff1DPassed[i]<<"+-"<<_nEff1DPassedErr[i];
      std::cout<<", eff="<<_eff1D[i]<<"+-"<<_eff1DErr[i];
      std::cout<<", acc x eff="<<_acc1D[i]*_eff1D[i]<<", accXeff="<<_accXeff1D[i]<<"+-"<<_accXeff1DErr[i]<<std::endl;
   }
}

void CalcAccAndEff::LoopOverTreeEvents()
{
   if (_eventTree.fChain == 0) return;
   Long64_t nentries = _eventTree.fChain->GetEntries();
   if (_isDebugMode) {
     if (_eventTree.fChain->GetEntries()<_debugModeNEntries) nentries=_eventTree.fChain->GetEntries();
     else nentries=_debugModeNEntries;
   }

   //goodLeptonPhotonPairs(two-dimentional array of bool-s)
   //memory allocation for some variables: 
   int nLeptonMax;
   if (_channel==TConfiguration::MUON) nLeptonMax=_eventTree.kMaxnMu;
   else if (_channel==TConfiguration::ELECTRON) nLeptonMax=_eventTree.kMaxnEle;
     //kMaxnMu - field of TEventTree
   else{
     std::cout<<"Error detected in WGammaSelection::LoopOverTreeEvents: channel must be either MUON or ELECTRON."<<std::cout;
     return;
   }

   float** lePhoDeltaR = new float*[nLeptonMax];
   for (int ile=0; ile<nLeptonMax; ile++)
     lePhoDeltaR[ile]=new float[_eventTree.kMaxnPho];
   
   bool** effLeptonPhotonPassed = new bool*[nLeptonMax];
   for (int ile=0; ile<nLeptonMax; ile++)
     effLeptonPhotonPassed[ile]=new bool[_eventTree.kMaxnPho];

   bool** accLeptonPhotonPassed = new bool*[_eventTree.kMaxnMC];
   for (int imc=0; imc<_eventTree.kMaxnMC; imc++)
     accLeptonPhotonPassed[imc]=new bool[_eventTree.kMaxnMC];

   CheckMaxNumbersInTree();
  
   //nentries=20;
   std::cout<<"n entries in MC tree: "<<_eventTree.fChain->GetEntries()<<std::endl;
   for (Long64_t entry=0; entry<nentries; entry++) {

   //loop over events in the tree{
     _eventTree.GetEntryNeededBranchesOnly(_channel,TConfiguration::SIGMC,entry);
          //method of TEventTree class

     if (!_eventTree.treeLeaf.isData) _eventTree.GetEntryMCSpecific(entry);
          //method of TEventTree class

     float weightPU;
     if (_isNoPuReweight)
       weightPU = _puWeight->GetPuWeightMc(_eventTree.treeLeaf.puTrue->at(1));
     else weightPU=1;

     if (_channel==TConfiguration::MUON) _nLe=_eventTree.treeLeaf.nMu;
     else if (_channel==TConfiguration::ELECTRON) _nLe=_eventTree.treeLeaf.nEle;
     else{
       std::cout<<"Error detected in  CalcAccAndEff::LoopOverTreeEvents: channel must be either MUON or ELECTRON."<<std::cout;
       return;
     }  

     bool accPassed = AcceptancePassed(accLeptonPhotonPassed);

  } //end of loop over events in the tree

  //memory release:

  for (int ile=0; ile<nLeptonMax; ile++)
    delete effLeptonPhotonPassed[ile];
  delete[] effLeptonPhotonPassed;

  for (int imc=0; imc<_eventTree.kMaxnMC; imc++)
    delete accLeptonPhotonPassed[imc];
  delete[] accLeptonPhotonPassed;

  for (int ile=0; ile<nLeptonMax; ile++)
    delete lePhoDeltaR[ile];
  delete[] lePhoDeltaR;
 
}

bool CalcAccAndEff::AcceptancePassed(bool** accLeptonPhotonPassed)
{
   bool accPassed=0;
   int bin=-1;

   bool genPhotons[_eventTree.kMaxnMC];
   bool genLeptons[_eventTree.kMaxnMC];
   for (int iMC=0; iMC<_eventTree.treeLeaf.nMC; iMC++){
     genPhotons[iMC]=0;
     genLeptons[iMC]=0;
     if (_eventTree.treeLeaf.mcPID->at(iMC)==22)
       genPhotons[iMC]=1;
     else if (_channel==TConfiguration::MUON && fabs(_eventTree.treeLeaf.mcPID->at(iMC)==13))
       genLeptons[iMC]=1;
     else if (_channel==TConfiguration::ELECTRON && fabs(_eventTree.treeLeaf.mcPID->at(iMC)==11))
       genLeptons[iMC]=1; 
   } // loop over iMC; search for gen level photons and leptons

   for (int iGenLep=0; iGenLep<_eventTree.treeLeaf.nMC; iGenLep++)
     for (int iGenPho=0; iGenPho<_eventTree.treeLeaf.nMC; iGenPho++){
       if (!(genLeptons[iGenLep] && genPhotons[iGenPho])) continue;
         float dR = _math.DeltaR(_eventTree.treeLeaf.mcEta->at(iGenLep),_eventTree.treeLeaf.mcPhi->at(iGenLep),_eventTree.treeLeaf.mcEta->at(iGenPho),_eventTree.treeLeaf.mcPhi->at(iGenPho));
         if (dR<=_config.GetLePhoDeltaRMin()) continue;

         /////////////////////////////////////////////////
         // count number of events 
         if (_eventTree.treeLeaf.mcEt->at(iGenPho)>_config.GetPhoPtMin()){
            _nAccTotEvents+=_lumiWeight;
            _nAccTotEventsErr+=_lumiWeight*_lumiWeight;
         }
         bin = _config.FindPhoPtBinByPhoPt(_eventTree.treeLeaf.mcEt->at(iGenPho));
         if (bin<(int)_config.GetNPhoPtBins() && bin!=-1){
           _nAcc1DEvents[bin]+=_lumiWeight;
           _nAcc1DEventsErr[bin]+=_lumiWeight*_lumiWeight;
         }

         /////////////////////////////////////////////////
         // count number of events within acceptance
         bool if1, if2;

         bool isPhoAcc = _photonEmpty.PassedKinematics(_eventTree.treeLeaf.mcE->at(iGenPho),_eventTree.treeLeaf.mcEta->at(iGenPho),if1,if2) ;
         bool isLepAcc = (_channel==TConfiguration::MUON && 
                   _muonEmpty.PassedKinematics(_vgamma, 1, _eventTree.treeLeaf.mcPt->at(iGenLep),_eventTree.treeLeaf.mcEta->at(iGenLep),if1,if2)) ||
                   (_channel==TConfiguration::ELECTRON);
         if (isPhoAcc && isLepAcc) {
           accPassed=1;
           accLeptonPhotonPassed[iGenLep][iGenPho]=1;
           if (_eventTree.treeLeaf.mcEt->at(iGenPho)>_config.GetPhoPtMin()){
             _nAccTotPassed+=_lumiWeight;
             _nAccTotPassedErr+=_lumiWeight*_lumiWeight;
           }
           bin = _config.FindPhoPtBinByPhoPt(_eventTree.treeLeaf.mcEt->at(iGenPho));
           if (bin<(int)_config.GetNPhoPtBins() && bin!=-1){
             _nAcc1DPassed[bin]+=_lumiWeight;
             _nAcc1DPassedErr[bin]+=_lumiWeight*_lumiWeight;
            }
          }  //end of if (isPhoAcc && isLepAcc)
        }//end of loops over iGenPho and iGenLep
  return accPassed;
}


void CalcAccAndEff::PlotAndSaveOutput()
{
  int nBins = _config.GetNPhoPtBins();

  TH1D* hAcc1D = new TH1D(_config.GetAccName(_config.ONEDI),_config.GetAccName(_config.ONEDI),nBins,_phoPtLimits);
  TH1D* hEff1D = new TH1D(_config.GetEffName(_config.ONEDI),_config.GetEffName(_config.ONEDI),nBins,_phoPtLimits);
  TH1D* hAccTot = new TH1D(_config.GetAccName(_config.TOTAL),_config.GetAccName(_config.TOTAL),1,_config.GetPhoPtMin(),_config.GetPhoPtMax());
  TH1D* hEffTot = new TH1D(_config.GetEffName(_config.TOTAL),_config.GetEffName(_config.TOTAL),1,_config.GetPhoPtMin(),_config.GetPhoPtMax());

  TH1D* hAccXEff1D = new TH1D("hAccXEff","hAccXEff",nBins,_phoPtLimits);
  TH1D* hAccxEff1D = new TH1D("hAccxEff","hAccxEff",nBins,_phoPtLimits);

  for (int i=0; i<_config.GetNPhoPtBins(); i++){
    hAcc1D->SetBinContent(i+1,_acc1D[i]);
    hAcc1D->SetBinError(i+1,_acc1DErr[i]);
    hEff1D->SetBinContent(i+1,_eff1D[i]);
    hEff1D->SetBinError(i+1,_eff1DErr[i]);

    hAccXEff1D->SetBinContent(i+1,_accXeff1D[i]);
    hAccXEff1D->SetBinError(i+1,_accXeff1DErr[i]);
  }
  hAccxEff1D->Multiply(hAcc1D,hEff1D);
 
  hAccTot->SetBinContent(1,_accTot);
  hAccTot->SetBinError(1,_accTotErr);
  hEffTot->SetBinContent(1,_effTot);
  hEffTot->SetBinError(1,_effTotErr);

  hAcc1D->SetLineWidth(2);
  hEff1D->SetLineWidth(2);
  hAccTot->SetLineWidth(2);
  hEffTot->SetLineWidth(2);

  hAcc1D->SetLineColor(2);
  hEff1D->SetLineColor(4);
  hAccTot->SetLineColor(2);
  hEffTot->SetLineColor(4);


  //Draw
  TCanvas* cAcc = new TCanvas("cAcc","cAcc");
  cAcc->SetLogx();
  hAcc1D->SetStats(0);
  hAcc1D->SetTitle("Acceptance");
  hAcc1D->GetXaxis()->SetMoreLogLabels();
  hAcc1D->GetXaxis()->SetNoExponent();
  hAcc1D->Draw();
  hAccTot->Draw("same");

  TCanvas* cEff = new TCanvas("cEff","cEff");
  cEff->SetLogx();
  hEff1D->SetStats(0);
  hEff1D->SetTitle("Efficiency");
  hEff1D->GetXaxis()->SetMoreLogLabels();
  hEff1D->GetXaxis()->SetNoExponent();
  hEff1D->Draw();
  hEffTot->Draw("same");

  TCanvas* cAccXEff = new TCanvas("cAccXEff","cAccXEff");
  cAccXEff->SetLogx();
  hAccXEff1D->SetStats(0);
  hAccXEff1D->SetTitle("AccXEff");
  hAccXEff1D->GetXaxis()->SetMoreLogLabels();
  hAccXEff1D->GetXaxis()->SetNoExponent();
  hAccXEff1D->Draw();
  hAccxEff1D->SetLineColor(3);
  hAccxEff1D->Draw("same");

  //save
  TString fNameAcc=_config.GetAccFileName(_channel,_vgamma) ;
  TFile fAcc(fNameAcc,"recreate");
  hAcc1D->Write((_config.GetAccName(_config.ONEDI)) );
  hAccTot->Write((_config.GetAccName(_config.TOTAL)) );
  cAcc->Write("cAcc");

  TString fNameEff=_config.GetEffFileName(_channel,_vgamma) ;
  TFile fEff(fNameEff,"recreate");  
  hEff1D->Write((_config.GetEffName(_config.ONEDI)) );
  hEffTot->Write((_config.GetEffName(_config.TOTAL)) );
  cEff->Write("cEff");

}

bool CalcAccAndEff::CheckMaxNumbersInTree()
{
  if (_eventTree.fChain->GetMaximum("nMC")>_eventTree.kMaxnMC){
    std::cout<<"ERROR in CalcAccAndEff::LoopOverTreeEvents: _eventTree.treeLeaf.nMC="<<_eventTree.fChain->GetMaximum("nMC")<<" > kMaxnMC="<<_eventTree.kMaxnMC<<"; go to ../Include/TEventTree.h to increase kMaxnMC"<<std::endl;
    return 0;
  }
  if (_eventTree.fChain->GetMaximum("nPho")>_eventTree.kMaxnPho){
     std::cout<<"ERROR in CalcAccAndEff::LoopOverTreeEvents: _eventTree.treeLeaf.nPho="<<_eventTree.fChain->GetMaximum("nPho")<<" > kMaxnPho="<<_eventTree.kMaxnPho<<"; go to ../Include/TEventTree.h to increase kMaxnPho"<<std::endl;
     return 0;
   }
   if (_channel==TConfiguration::MUON && _eventTree.fChain->GetMaximum("nMu")>_eventTree.kMaxnMu){
     std::cout<<"ERROR in CalcAccAndEff::LoopOverTreeEvents: _eventTree.treeLeaf.nMu="<<_eventTree.fChain->GetMaximum("nMu")<<" > kMaxnMu="<<_eventTree.kMaxnMu<<"; go to ../Include/TEventTree.h to increase kMaxnMu"<<std::endl;
     return 0;
   }
   if (_channel==TConfiguration::ELECTRON && _eventTree.fChain->GetMaximum("nEle")>_eventTree.kMaxnEle){
     std::cout<<"ERROR in CalcAccAndEff::LoopOverTreeEvents: _eventTree.treeLeaf.nEle="<<_eventTree.fChain->GetMaximum("nEle")<<" > kMaxnEle="<<_eventTree.kMaxnEle<<"; go to ../Include/TEventTree.h to increase kMaxnEle"<<std::endl;
     return 0;
   }
  return 1;
}
