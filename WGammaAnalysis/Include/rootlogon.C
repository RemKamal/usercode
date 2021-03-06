{
  gSystem->Load("../RooUnfold/libRooUnfold.so");

  gROOT->ProcessLine(".L ../Configuration/TConfiguration.C+");

  gROOT->ProcessLine(".L ../Configuration/TInputSample.C+");
  gROOT->ProcessLine(".L ../Configuration/TAllInputSamples.C+");

  gROOT->ProcessLine(".L ../PHOSPHOR_CORRECTION/PhosphorCorrectorFunctor.cc+");

  gROOT->ProcessLine(".L ../Include/TMathTools.C+");
  gROOT->ProcessLine(".L ../Include/TEventTree.C+");
  gROOT->ProcessLine(".L ../Include/TPhotonCuts.C+");
  gROOT->ProcessLine(".L ../Include/TPhotonScaleFactor.C+");
  gROOT->ProcessLine(".L ../Include/TElectronCuts.C+");
  gROOT->ProcessLine(".L ../Include/TMuonCuts.C+");
  gROOT->ProcessLine(".L ../Include/TMetTools.C+");
  gROOT->ProcessLine(".L ../Include/TPuReweight.C+");
  gROOT->ProcessLine(".L ../Include/TFullCuts.C+");

//  gROOT->ProcessLine(".L ../SearchForOverlapInMC/SearchForOverlap.C+");

  gROOT->ProcessLine(".L ../Selection/TSelectedEventsTree.C+");

  gROOT->ProcessLine(".L ../Selection/TSelectionPlots.C+");
  gROOT->ProcessLine(".L ../Selection/Selection.C+");

  gROOT->ProcessLine(".L ../DDBkgTemplateMethod/TTemplatesRandCone.C+");
  gROOT->ProcessLine(".L ../DDBkgTemplateMethod/TTemplatesRandConeSyst.C+");
  gROOT->ProcessLine(".L ../DDBkgTemplateMethod/AuxTemplatesRandCone.C+");

  gROOT->ProcessLine(".L ../PrepareYields/TPrepareYields.C+");
  gROOT->ProcessLine(".L ../PrepareYields/TSubtractBackground.C+");
  gROOT->ProcessLine(".L ../PrepareYields/AuxPrepareYields.C+");

  gROOT->ProcessLine(".L ../Unfolding/Unfolding.C+");

  gROOT->ProcessLine(".L ../AcceptanceAndEfficiency/CalcAccAndEff.C+");

//  gROOT->ProcessLine(".L ../CrossSection/CalcCrossSection.C+");

  gROOT->ProcessLine(".L ../FullChain/FullChain.C+");


  gROOT->ProcessLine(".L ../SkimSplitMerge/TInputOutputTree.C+");
  gROOT->ProcessLine(".L ../SkimSplitMerge/SkimLeptonPhoton.C+");
  gROOT->ProcessLine(".L ../SkimSplitMerge/SplitVDecayMC.C+");
  gROOT->ProcessLine(".L ../SkimSplitMerge/SkimNEvents.C+");

  gStyle->SetPalette(1);
  gStyle->SetPaintTextFormat("0.0f");
  gStyle->SetCanvasColor(kWhite); 

}
