#include "SkimLeptonPhoton.h"

#include "TString.h"
#include "TBenchmark.h"

#include <iostream>

void auxSkim(TString fileToSkimName)
{

  TBenchmark time;
  time.Start("time");
  std::cout<<"CPU time = "<<time.GetCpuTime("time")<<", Real time = "<<time.GetRealTime("time")<<std::endl;  

//  TString fileToSkimFullName = "/mnt/hadoop/user/uscms01/pnfs/unl.edu/data4/cms/store/user/eavdeeva2/WGammaMC/" + fileToSkimName;

  std::cout<<"file "<<fileToSkimName<<" will be skimmed"<<std::endl;

  SkimLeptonPhoton skimmer(TConfiguration::MUON, TConfiguration::W_GAMMA, TConfiguration::DATA,fileToSkimName);
  skimmer.LoopOverInputTree();

  std::cout<<"file "<<fileToSkimName<<" was skimmed"<<std::endl;

  time.Stop("time");
  std::cout<<"CPU time = "<<time.GetCpuTime("time")<<", Real time = "<<time.GetRealTime("time")<<std::endl; 

}
