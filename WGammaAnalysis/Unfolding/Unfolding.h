#ifndef Unfolding_h
#define Unfolding_h

#include "../Configuration/TConfiguration.h"
#include "../Configuration/TAllInputSamples.h"
#include "../Include/TEventTree.h"
#include "../Include/TPuReweight.h"
  //this package
#include "../Include/PhosphorCorrectorFunctor.hh"
  //taken from http://cmssw.cvs.cern.ch/cgi-bin/cmssw.cgi/UserCode/CPena/src/PHOSPHOR_Corr_v2/
#include "TTree.h" 
#include "TMath.h" 
  //ROOT package

class Unfolding: public TEventTree
  {
     public:
       Unfolding (int channel, string configfile="../Configuration/config.txt", bool isPuReweight=1, bool isDebugMode=0);

       virtual ~Unfolding();
       void    LoopOverInputFiles();
       void    LoopOverTreeEvents();
       void    PlotAndSaveOutput();
//       bool    CheckMaxNumbersInTree();
//       void    PrintErrorMessageMaxNumberOf(int particle);

     private:

       TAllInputSamples* INPUT_;

//       enum {MUON_, ELECTRON_, PHOTON_, JET_, LOWPTJET_, MC_};
       int channel_;

       bool isDebugMode_;
       bool isPuReweight_;

       float lumiWeight_;
       float debugModeWeight_;
       float totalWeight_;

       int nLe_;

       float nEvents_;
       float nAccPassed_;
       float nEventsInAcc_;
       float nEffPassed_;
       float nEventsErr_;
       float nAccPassedErr_;
       float nEventsInAccErr_;
       float nEffPassedErr_;

       float acc_;
       float eff_;
       float accErr_;
       float effErr_;

       vector <float> vecPhoPtLimits_;

       vector <float> vecnEvents_;
       vector <float> vecnAccPassed_;
       vector <float> vecnEventsInAcc_;
       vector <float> vecnEffPassed_;
       vector <float> vecnEventsErr_;
       vector <float> vecnAccPassedErr_;
       vector <float> vecnEventsInAccErr_;
       vector <float> vecnEffPassedErr_;

       vector <float> vecacc_;
       vector <float> veceff_;
       vector <float> vecaccErr_;
       vector <float> veceffErr_;


       const static int debugModeNEntries_=100000;

       zgamma::PhosphorCorrectionFunctor* photonCorrector_;
       TPuReweight* puWeight_;
       TConfiguration config_;
  };

#endif 
  //#ifndef 