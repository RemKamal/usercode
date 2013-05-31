#ifndef WGammaSelection_h
#define WGammaSelection_h

#include "../Include/TEventTree.h" 
#include "TSelectedEventsTree.h"
#include "../Include/TMuonCuts.h" 
#include "../Include/TElectronCuts.h" 
#include "../Include/TPhotonCuts.h" 
#include "../Configuration/TAllInputSamples.h"
  //this package
#include "../Include/PhosphorCorrectorFunctor.hh"
  //taken from http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/CPena/src/PHOSPHOR_Corr_v2/
#include "TTree.h" 
#include "TMath.h" 
  //ROOT package

class WGammaSelection : public TEventTree, public TSelectedEventsTree 
  {
     //TEventTree - class for input tree
     //TSelectedEventsTree - class for output tree and output root file
     public:
       WGammaSelection (int channel, int sampleMode = ALL, string configfile="../Configuration/config.txt", bool isReseasedCutsMode=0, bool isDebugMode=0);
       WGammaSelection(int channel, string analyzedSampleNames, string configFile="../Configuration/config.txt", bool isReseasedCutsMode=0, bool isDebugMode=0);
       virtual ~WGammaSelection();
       void    LoopOverInputFiles();
       void    LoopOverTreeEvents();
       bool    CheckMaxNumbersInTree();
       void    PrintErrorMessageMaxNumberOf(int particle);

       enum {DATA, SIGMC, BKGMC, MC, NOBKG, ALL, NOTSPECIFIED};

     private:

       TAllInputSamples* INPUT_;

       enum {MUON_, ELECTRON_, PHOTON_, JET_, LOWPTJET_, MC_};
       int channel_;
       int sample_;

       bool isDebugMode_;
       bool isReleasedCutsMode_;
       int sampleMode_;
       vector <bool> doAnalizeSample_;

       
       

       int nTotal_;
       int nBeforeLeptonLoop_;
       int nLeptons_;
       int nLeptonsPassed_; 
       int nMoreVetoPassed_;
       int nWMtPassed_;
       int nPhotons_;
       int nPhotonsPassed_;
       int nPhoLepPassed_;      

       float lumiWeight_;
       float debugModeWeight_;
       float totalWeight_;
       int nLe_;
       int inputFileN_;
//       float WMt_[kMaxnMu+kMaxnEle];
//       float lePhoDeltaR_[kMaxnMu+kMaxnEle][kMaxnPho];
         //kMaxnMu,kMaxnEle,kMaxnPho - 
         //constant fields of TEventTree
//       const static float WMtCut_ = 70.;//70.;
//       const static float lePhoDeltaRCut_ = 0.7;//0.7;

       const static int debugModeNEntries_=100000;

       zgamma::PhosphorCorrectionFunctor* photonCorrector_;
       TString selectedTreeFileName_;
  };

#endif //#ifndef WGammaSelection_h
