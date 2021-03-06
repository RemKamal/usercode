#include "TTemplatesRandCone.h"
  //this class
//#include "../Include/TPhotonCuts.h"
//#include "../Include/TMathTools.h"
//#include "../Configuration/TConfiguration.h"
//#include "../Configuration/TInputSample.h"
//#include "../Configuration/TAllInputSamples.h"

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include "RooRealVar.h"
#include "RooArgList.h"
#include "RooArgSet.h"
#include "RooDataSet.h"
#include "RooDataHist.h"
#include "RooHistPdf.h"
#include "RooExtendPdf.h"
#include "RooAddPdf.h"
#include "RooPlot.h"
#include "RooChi2Var.h"
#include "RooAbsReal.h"
#include "TCanvas.h"
#include "TString.h"
#include "TText.h"
#include "TLatex.h"
#include "TLine.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TVectorD.h"

using namespace RooFit ;

TTemplatesRandCone::TTemplatesRandCone()
{
}

TTemplatesRandCone::TTemplatesRandCone(TemplatesRandConePars pars)
{
  SetPars(pars);
}

TTemplatesRandCone::~TTemplatesRandCone()
{
  //if (_pars.fOutForSave->IsOpen()) _pars.fOutForSave->Close();
}

void TTemplatesRandCone::SetPars(TemplatesRandConePars pars)
{
  _pars=pars;
  _pars.cutWeight=_pars.varWeight;

  _pars.fOutForSave = new TFile(_pars.strFileOutName,"recreate");
}

void TTemplatesRandCone::ComputeBackground(bool noPrint, bool noPlot)
{
  if (!noPrint) PrintPars();
  for (int ikin=0; ikin<=_pars.nKinBins; ikin++){
  //for (int ikin=0; ikin<=1; ikin++){
    ComputeBackgroundOne(ikin,_COMMON,noPrint);
  }
  if (!noPlot) PlotTemplates();
  PrintYieldsAndChi2();
  SaveYields();
}

void TTemplatesRandCone::PrintPars()
{
  std::cout<<"==============================="<<std::endl;
  std::cout<<"||||========== TemplatesRandcone parameters"<<std::endl;
  std::cout<<"varKin: "<<_pars.varKin<<std::endl;
  std::cout<<"varTrueTempl: "<<_pars.varTrueTempl<<std::endl;
  std::cout<<"varFakeTempl: "<<_pars.varFakeTempl<<std::endl;
  std::cout<<"varSideband: "<<_pars.varSideband<<std::endl;
  std::cout<<"varPhoEta: "<<_pars.varPhoEta<<std::endl;
  std::cout<<"varWeight: "<<_pars.varWeight<<std::endl;
//  std::cout<<"fTrue: "<<_pars.fTrue->GetTitle()<<std::endl;
//  std::cout<<"fFake: "<<_pars.fFake->GetTitle()<<std::endl;
//  std::cout<<"fData: "<<_pars.fData->GetTitle()<<std::endl;
//  std::cout<<"fSign: "<<_pars.fSign->GetTitle()<<std::endl;
  std::cout<<"nKinBins="<<_pars.nKinBins<<std::endl;
  std::cout<<"kin bin lims; nFitBins; fit range; sideband"<<std::endl;
  for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
    std::cout<<StrLabelEta(ieta)<<std::endl;
    for (int ik=0; ik<=_pars.nKinBins; ik++){
      std::cout<<StrLabelKin(ik)<<"; "<<_pars.nFitBins[ik][ieta]<<"; "<<_pars.minVarFit[ik][ieta]<<"-"<<_pars.maxVarFit[ik][ieta]<<"; "<<_pars.sideband[ik][ieta]<<"-"<<_pars.sidebandUp[ik][ieta]<<std::endl;
    }//end of loop over ik
  }//end of loop over ieta
  std::cout<<"cutAdd="<<_pars.cutAdd.GetTitle()<<std::endl;
  std::cout<<"cutBarrel="<<_pars.cutBarrel.GetTitle()<<std::endl;
  std::cout<<"cutEndcap="<<_pars.cutEndcap.GetTitle()<<std::endl;
  std::cout<<"cutNominal="<<_pars.cutNominal.GetTitle()<<std::endl;
  std::cout<<"cutNominalExceptSidebandVar="<<_pars.cutNominalExceptSidebandVar.GetTitle()<<std::endl;
  std::cout<<"cutSidebandVarNominalRange="<<_pars.cutSidebandVarNominalRange.GetTitle()<<std::endl;
  std::cout<<"cutWeight="<<_pars.cutWeight.GetTitle()<<std::endl;
  std::cout<<"noLeakSubtr="<<_pars.noLeakSubtr<<std::endl;
  std::cout<<"strFileOutName="<<_pars.strFileOutName<<std::endl;
  for (int ieta=_BARREL; ieta<=_COMMON; ieta++){
    std::cout<<StrLabelEta(ieta)<<":"<<std::endl;
    std::cout<<"strTrueYieldsTot="<<_pars.strTrueYieldsTot[ieta]<<std::endl;
    std::cout<<"strTrueYields1D="<<_pars.strTrueYields1D[ieta]<<std::endl;
    std::cout<<"strFakeYieldsTot="<<_pars.strFakeYieldsTot[ieta]<<std::endl;
    std::cout<<"strFakeYields1D="<<_pars.strFakeYields1D[ieta]<<std::endl;
  }//end of loop over ieta
  std::cout<<"|||| end of TemplatesRandcone parameters"<<std::endl;
  std::cout<<"==============================="<<std::endl;
}// end of TTemplatesRandCone::PrintPars()

void TTemplatesRandCone::ComputeBackgroundOne(int ikin, int ieta, bool noPrint)
{
  // if ieta == _COMMON - compute BARREL, ENDCAP and SUM
  // otherwise compute just for ieta
  if (!noPrint){
    std::cout<<std::endl;
    std::cout<<"==============================="<<std::endl;
    std::cout<<"||||========== ComputeBackgroundOne - "<<StrLabelKin(ikin)<<" starts"<<std::endl;
  }

  for (int ieta1=_BARREL; ieta1<=_ENDCAP; ieta1++){
    if ((ieta!=_COMMON) && (ieta!=ieta1)) continue;
    SetHists(ikin, ieta1, noPrint);
    FitOne(ikin, ieta1, noPrint);
  }//end of loop over ieta

  ComputeYieldOneKinBin(ikin,ieta,noPrint);


  if (!noPrint){
    std::cout<<"||||========== ComputeBackgroundOne - "<<StrLabelKin(ikin)<<" ends"<<std::endl;
    std::cout<<"==============================="<<std::endl;
    std::cout<<std::endl;
  }

}

void TTemplatesRandCone::SetHists(int ikin, int ieta, bool noPrint){

  if (!noPrint){
    std::cout<<std::endl;
    std::cout<<"SetHists for: "<<StrLabelKin(ikin)<<", "<<StrLabelEta(ieta)<<std::endl;
  }
  TString name=TString("h")+StrLabelKin(ikin)+StrLabelEta(ieta); 
  
  TCut cutSideband=SidebandCut(ikin,ieta);
  TCut cutSignal=SidebandVarNominalCut();

  TCut cutEta = CutEtaBin(ieta);
  TCut cutKin = CutKinBin(ikin);
  TCut cut;

  int nFitBins=_pars.nFitBins[ikin][ieta];
  double max=_pars.maxVarFit[ikin][ieta];
  double min=_pars.minVarFit[ikin][ieta];
  double unit = (max-min)/nFitBins;
  // one bin [-unit,0] is reserved for events for which Iso==0 

  double fitBinLims[nFitBins+1];

  if (!noPrint){
    std::cout<<std::endl;
    std::cout<<"min="<<min<<", max="<<max<<", unit="<<unit<<", nFitBins="<<nFitBins<<std::endl;
    std::cout<<"bin lims:";
  }
  for (int i=0; i<=nFitBins; i++){
    fitBinLims[i]=min+i*unit;
    if (!noPrint) std::cout<<fitBinLims[i]<<", ";
  }
  std::cout<<std::endl;

  //_pars.kinBinLims[ikin] is upper limit
  //_pars.kinBinLims[ikin-1] is lower limit

  name+="_True"; 
  if (_pars.combineTrueTempl[ikin][ieta] && ikin>0){
     cutKin="1";
     for (int i=1; i<=_pars.nKinBins; i++){
       if (_pars.combineTrueTempl[i][ieta]) cutKin = cutKin || CutKinBin(i);
     }
  }
  cut = _pars.cutAdd && cutEta && cutKin && SidebandVarNominalCut();// && cutSignal;;
  if (!noPrint) {
    std::cout<<"cutEta="<<cutEta.GetTitle()<<std::endl;
    std::cout<<"cutKin="<<cutKin.GetTitle()<<std::endl;
//    std::cout<<"cutSignal="<<cutSignal.GetTitle()<<std::endl;
    std::cout<<"cutWeight="<<_pars.cutWeight.GetTitle()<<std::endl;
    std::cout<<"true template cut=(cutAdd && cutEta && cutKin)*cutWeight="<<cut.GetTitle()<<std::endl;
  }


  _pars.fOutForSave->cd();
  _hTrue[ikin][ieta] = new TH1D(name,name,nFitBins,fitBinLims);
  _hTrue[ikin][ieta]->Sumw2();

  SetTemplate(1,_hTrue[ikin][ieta],cut,noPrint);
  // "1" for true gamma template

  cutKin=CutKinBin(ikin);

  name.ReplaceAll("_True","_Fake");
  if (_pars.combineFakeTempl[ikin][ieta] && ikin>0){
     cutKin="1";
     for (int i=1; i<=_pars.nKinBins; i++){
       if (_pars.combineFakeTempl[i][ieta]) cutKin = cutKin || CutKinBin(i);
     }
  }
  cut = _pars.cutAdd && cutEta && cutKin && cutSideband;
  if (!noPrint) {
    std::cout<<"cutSideband="<<cutSideband.GetTitle()<<std::endl;
    std::cout<<"fake template cut=(cutAdd && cutEta && cutKin && cutSideband)*cutWeight="<<cut.GetTitle()<<std::endl;
  }
  if (!noPrint) std::cout<<"Fake template cut="<<cut.GetTitle()<<std::endl;
  _hFake[ikin][ieta] = new TH1D(name,name,nFitBins,fitBinLims);
  _hFake[ikin][ieta]->Sumw2();
  name.ReplaceAll("_Fake","_Leak");
  _hLeak[ikin][ieta] = new TH1D(name,name,nFitBins,fitBinLims);
  _hLeak[ikin][ieta]->Sumw2();
  name.ReplaceAll("_Leak","_Data");
  _hData[ikin][ieta] = new TH1D(name,name,nFitBins,fitBinLims);
//  _hData[ikin][ieta]->Sumw2();
  SetTemplate(0,_hFake[ikin][ieta], cut, noPrint, _hLeak[ikin][ieta]);
  // "0" for fake gamma template

  cutKin=CutKinBin(ikin);

  _hFakeReference[ikin][ieta] = (TH1D*)_hFake[ikin][ieta]->Clone(_hFake[ikin][ieta]->GetName()+TString("_Ref"));
  _hFakeReference[ikin][ieta]->SetTitle(_hFakeReference[ikin][ieta]->GetName());
  _hTrueReference[ikin][ieta] = (TH1D*)_hTrue[ikin][ieta]->Clone(_hTrue[ikin][ieta]->GetName()+TString("_Ref"));
  _hTrueReference[ikin][ieta]->SetTitle(_hTrueReference[ikin][ieta]->GetName());

  if (!noPrint)     std::cout<<std::endl;

}//end of SetHists()

void TTemplatesRandCone::SetTemplate(bool isTrueGamma, TH1D* hTemplate, TCut cut, bool noPrint, TH1D* hLeak)
{
  TString name=hTemplate->GetName();

  if (isTrueGamma) _pars.treeTrue->Draw(_pars.varTrueTempl+(">>")+name,cut*_pars.cutWeight,"goff");

  else _pars.treeFake->Draw(_pars.varFakeTempl+(">>")+name,cut*_pars.cutWeight,"goff");
  if (!noPrint) {
    std::cout<<std::endl;
    std::cout<<"hTemplate->GetName()="<<hTemplate->GetName()<<std::endl;
    std::cout<<"hTemplate->GetSumOfWeights()="<<hTemplate->GetSumOfWeights()<<std::endl;
    if (isTrueGamma) std::cout<<"_pars.treeTrue->GetEntries(cut)="<<_pars.treeTrue->GetEntries(cut*_pars.cutWeight)<<std::endl;
    else std::cout<<"_pars.treeFake->GetEntries(cut)="<<_pars.treeFake->GetEntries(cut*_pars.cutWeight)<<std::endl;
    
  }

  if (isTrueGamma) return;
  if (_pars.noLeakSubtr) return;


  TString strNameLeak=hLeak->GetName();
  _pars.treeSign->Draw(_pars.varFakeTempl+TString(">>")+strNameLeak,cut*_pars.cutWeight,"goff");
  hTemplate->Add(hLeak,-1);

  if (!noPrint){

    std::cout<<"(cut*_pars.cutWeight)->GetTitle()="<<(cut*_pars.cutWeight).GetTitle()<<std::endl;  
    hLeak->Print();
    std::cout<<"hLeak->GetName()="<<hLeak->GetName()<<std::endl;  
    std::cout<<"hLeak->GetSumOfWeights()="<<hLeak->GetSumOfWeights()<<std::endl;  
    std::cout<<"_pars.treeSign->GetEntries(cut)="<<_pars.treeSign->GetEntries(cut*_pars.cutWeight)<<std::endl; 
    std::cout<<"hTemplate->GetSumOfWeights()="<<hTemplate->GetSumOfWeights()<<std::endl; 
/*
    TH1F* histTempr = new TH1F("histTempr","histTempr",1,_pars.treeSign->GetMinimum(_pars.varWeight)-1,_pars.treeSign->GetMaximum(_pars.varWeight)+1);
    histTempr->Sumw2();
    _pars.treeSign->Draw(_pars.varWeight+TString(">>histTempr"),cut*_pars.cutWeight,"goff");
    std::cout<<std::fixed;
    std::cout<<hLeak->GetTitle()<<" Template, mean weight =";
    std::cout<<std::setprecision(2)<<histTempr->GetMean()<<"+-";
    std::cout<<std::setprecision(2)<<histTempr->GetRMS()<<"; ";
    std::cout<<"_pars.treeSign->GetEntries(cut*_pars.cutWeight)="<<_pars.treeSign->GetEntries(cut*_pars.cutWeight);
    std::cout<<std::endl;
    std::cout<<std::endl;
    delete histTempr;
*/
  }
}

void TTemplatesRandCone::RandomizeTemplates(int ikin, int ieta)
{
  for (int ib=1; ib<_hTrueReference[ikin][ieta]->GetNbinsX(); ib++){
    float mean = _hTrueReference[ikin][ieta]->GetBinContent(ib);
    float sigma = _hTrueReference[ikin][ieta]->GetBinError(ib);
    _hTrue[ikin][ieta]->SetBinContent(ib,_random.Gaus(mean,sigma));
    _hTrue[ikin][ieta]->SetBinError(ib,sigma);
  }
  for (int ib=1; ib<_hFakeReference[ikin][ieta]->GetNbinsX(); ib++){
    float mean = _hFakeReference[ikin][ieta]->GetBinContent(ib);
    float sigma = _hFakeReference[ikin][ieta]->GetBinError(ib);
    _hFake[ikin][ieta]->SetBinContent(ib,_random.Gaus(mean,sigma));
    _hFake[ikin][ieta]->SetBinError(ib,sigma);
  }
}

void TTemplatesRandCone::DeleteHists(int ikin, int ieta){

  delete _hTrue[ikin][ieta];
  delete _hFake[ikin][ieta]; 

}

TString TTemplatesRandCone::StrLabelEta(int ieta){
  if (ieta==_BARREL) return "_Barrel_";
  else if (ieta==_ENDCAP) return "_Endcap_";
  else if (ieta==_COMMON) return "_EtaCommon_";
  return "_EtaUnknown_";
}

TString TTemplatesRandCone::StrLabelKin(int ikin){
  if (ikin<0 || ikin>_pars.nKinBins) return TString("_")+_pars.varKin+TString("Unknown_");
  if (ikin==0) return TString("_")+_pars.varKin+TString("Total_");
  TString str="_";
  str+=_pars.varKin;
  str+=_pars.kinBinLims[ikin-1];
  str+="to";
  str+=_pars.kinBinLims[ikin];
  str+="_";
  return str;
}

void TTemplatesRandCone::FitOne(int ikin, int ieta, bool noPrint, bool noPlot)
{

  if (!noPrint){
    std::cout<<std::endl;
    std::cout<<"FitOne: "<<StrLabelKin(ikin)<<", "<<StrLabelEta(ieta)<<std::endl;
    std::cout<<std::endl;
  }

  double varMin=0;
  double varMax=0;
  int nBins=0;

  nBins = _hFake[ikin][ieta]->GetNbinsX();
  varMin = _hFake[ikin][ieta]->GetBinLowEdge(1);
  varMax = _hFake[ikin][ieta]->GetBinLowEdge(nBins)+_hFake[ikin][ieta]->GetBinWidth(nBins);

  double minVarSideband = 0;
  double maxVarSideband = 100000;
  double minPhoEta = -2.6;
  double maxPhoEta = 2.6;
  double minKin = -100000;
  double maxKin =  100000;
  double minWeight = 0;
  double maxWeight = 1;

  maxVarSideband = _pars.treeData->GetMaximum(_pars.varSideband);
  minVarSideband = _pars.treeData->GetMinimum(_pars.varSideband);
  maxPhoEta = _pars.treeData->GetMaximum(_pars.varPhoEta);
  minPhoEta = _pars.treeData->GetMinimum(_pars.varPhoEta);
  maxKin = _pars.treeData->GetMaximum(_pars.varKin);
  minKin = _pars.treeData->GetMinimum(_pars.varKin);
  maxWeight = _pars.treeData->GetMaximum(_pars.varWeight);
  minWeight = _pars.treeData->GetMinimum(_pars.varWeight);
  if (!noPrint){
    std::cout<<"sideband ("<<_pars.varSideband<<") range: "<<minVarSideband<<" - "<<maxVarSideband<<std::endl;
    std::cout<<_pars.varPhoEta<<" range: "<<minPhoEta<<" - "<<maxPhoEta<<std::endl;
    std::cout<<_pars.varKin<<" range: "<<minKin<<" - "<<maxKin<<std::endl;
    std::cout<<_pars.varWeight<<" range: "<<minWeight<<" - "<<maxWeight<<std::endl;
  }


  TCut sidebandCut = SidebandCut(ikin,ieta);
  TCut etaCut = CutEtaBin(ieta);
  TCut kinCut = CutKinBin(ikin);
  TCut sidebandVarNominalCut = SidebandVarNominalCut();
  if (!noPrint){
    std::cout<<"etaCut="<<etaCut.GetTitle()<<std::endl;
    std::cout<<"kinCut="<<kinCut.GetTitle()<<std::endl;
    std::cout<<"sidebandVarNominalCut="<<sidebandVarNominalCut.GetTitle()<<std::endl;
  }
  TCut cut = _pars.cutAdd && kinCut && etaCut && sidebandVarNominalCut;
	
//  TH1D* histTemp = (TH1D*)_hData[ikin][ieta]->Clone(); 
//  histTemp->SetName(TString("histTemp")+StrLabelKin(ikin)+StrLabelEta(ieta));
  TString varDraw=_pars.varFit+TString(">>")+_hData[ikin][ieta]->GetName();
  _pars.treeData->Draw(varDraw,cut*_pars.cutWeight,"goff");
//  for (int ib=1; ib<=_hData[ikin][ieta]->GetNbinsX(); ib++){
//    _hData[ikin][ieta]->SetBinContent(ib,histTemp->GetBinContent(ib));
//    _hData[ikin][ieta]->SetBinError(ib,histTemp->GetBinError(ib));
//  }


/*
  TH1F* histTemp = new TH1F("histTemp","histTemp",1,_pars.treeData->GetMinimum(_pars.varWeight)-1,_pars.treeData->GetMaximum(_pars.varWeight)+1);
  histTemp->Sumw2();
  _pars.treeData->Draw(_pars.varWeight+TString(">>hist"),cut,"goff");
  std::cout<<std::endl;
  std::cout<<std::fixed;
  std::cout<<"Data Template, mean weight =";
  std::cout<<std::setprecision(2)<<histTemp->GetMean()<<"+-";
  std::cout<<std::setprecision(2)<<histTemp->GetRMS()<<"; ";
  std::cout<<std::endl;
  std::cout<<std::endl;
  delete histTemp; 
*/
  if (!noPrint){
    std::cout<<"data hist TH1D* hist: "<<std::endl;
    _hData[ikin][ieta]->Print();
    std::cout<<"true template hist TH1D* hist: "<<std::endl;
    _hTrue[ikin][ieta]->Print();
    std::cout<<"fake template hist TH1D* hist: "<<std::endl;
    _hFake[ikin][ieta]->Print();
  }

  //create total pdf
  int nMax = _pars.treeData->GetEntries(cut);
  int nTrueStart = 0.5*nMax;
  int nFakeStart = 0.5*nMax;

  RooRealVar rooVarFit(_pars.varFit,_pars.varFit, varMin, varMax);
  if (nMax<=0){
    if (!noPrint) std::cout<<"ATTENTION: No data events for bin "<<kinCut<<", "<<etaCut<<std::endl;
    _nFakeFromFitVal[ikin][ieta]=0;
    _nFakeFromFitErr[ikin][ieta]=0;
    _nTrueFromFitVal[ikin][ieta]=0;
    _nTrueFromFitErr[ikin][ieta]=0;
    _plotter[ikin][ieta] = new RooPlot(rooVarFit,varMin,varMax,nBins);
    return;
  }
  RooRealVar rooVarSideband(_pars.varSideband,_pars.varSideband,minVarSideband,maxVarSideband);
  RooRealVar rooPhoEta(_pars.varPhoEta,_pars.varPhoEta,minPhoEta,maxPhoEta);
  RooRealVar rooKin(_pars.varKin,_pars.varKin,minKin,maxKin);
  RooRealVar rooWeight(_pars.varWeight,_pars.varWeight,minWeight,maxWeight);
  RooArgList argList;
  argList.add(rooVarFit);
  RooArgSet argSet;
  argSet.add(rooVarFit);
  if (!noPrint){
    std::cout<<"Histograms (hTrue, hTrueReference, hFake, hFakeReference, hLeak, hData):"<<std::endl;
    PrintOneHistogramBinByBin(_hTrue, ikin, ieta);
    PrintOneHistogramBinByBin(_hTrueReference, ikin, ieta);
    PrintOneHistogramBinByBin(_hFake, ikin, ieta);
    PrintOneHistogramBinByBin(_hFakeReference, ikin, ieta);
    PrintOneHistogramBinByBin(_hData, ikin, ieta);
  }
  RooDataHist dataHist("hist", "data set converted to hist", argList, _hData[ikin][ieta]);
  //create Fake and True PDFs
  RooDataHist trueDataHist("trueDataHist","true RooDataHist", argList, _hTrue[ikin][ieta]);
  RooHistPdf truePdf("truePdf",_pars.varTrueTempl+TString(" of true"), argSet, trueDataHist);
  RooDataHist fakeDataHist("fakeDataHist", "fake RooDataHist", argList, _hFake[ikin][ieta]);
  RooHistPdf fakePdf("fakePdf",_pars.varTrueTempl+TString(" of fake"), argSet, fakeDataHist);

  //load data
  RooArgSet argSetData(rooVarFit,rooVarSideband,rooPhoEta,rooKin,rooWeight);
  RooRealVar rooNTrue("nTrue","n True",nTrueStart,0,nMax);
  RooExtendPdf eTruePdf("eTrue","extended True",truePdf,rooNTrue);
  RooRealVar rooNFake("nFake","n Fake",nFakeStart,0,nMax);
  RooExtendPdf eFakePdf("eFake","extended Fake",fakePdf,rooNFake);
  RooAddPdf fullPdf("fitModel","fit model",RooArgList(eTruePdf,eFakePdf));
  //fit
  fullPdf.fitTo(dataHist,SumW2Error(kFALSE),Extended(kTRUE));

  //calc chi squared
  RooChi2Var chi2("chi2","chi2",fullPdf,dataHist);
  _chi2ToNDF[ikin][ieta] = chi2.getVal()/(nBins-2);

  //get nFake values from fit
  _nFakeFromFitVal[ikin][ieta]=rooNFake.getVal();
  _nFakeFromFitErr[ikin][ieta]=rooNFake.getError();
  _nTrueFromFitVal[ikin][ieta]=rooNTrue.getVal();
  _nTrueFromFitErr[ikin][ieta]=rooNTrue.getError();
  
  if (!noPrint){
    std::cout<<std::endl;
    std::cout<<"nFake="<<rooNFake.getVal()<<"+-"<<rooNFake.getError()<<", nTrue="<<rooNTrue.getVal()<<"+-"<<rooNTrue.getError()<<", chi2="<<chi2.getVal()/(nBins-2)<<std::endl;
    std::cout<<std::endl;
  }

  //for plotting
  _plotter[ikin][ieta] = new RooPlot(rooVarFit,varMin,varMax,nBins); 
  dataHist.plotOn(_plotter[ikin][ieta]);
  fullPdf.plotOn(_plotter[ikin][ieta],Name("sum"),LineColor(kRed));
  fullPdf.plotOn(_plotter[ikin][ieta],Components("truePdf"), Name("true"),
  LineColor(kGreen),LineStyle(9));
  fullPdf.plotOn(_plotter[ikin][ieta], Components("fakePdf"),Name("fake"),LineColor(kBlue),LineStyle(9));
  fullPdf.paramOn(_plotter[ikin][ieta]);
}

void TTemplatesRandCone::ComputeYieldOneKinBin(int ikin, int ieta, bool noPrint)
{

  ComputeOneYield(ikin, ieta, noPrint, 1,
	_nTrueYieldsVal[ikin], _nTrueYieldsErr[ikin], _nTrueFromFitVal[ikin], _nTrueFromFitErr[ikin]);

  ComputeOneYield(ikin, ieta, noPrint, 0,
	_nFakeYieldsVal[ikin], _nFakeYieldsErr[ikin], _nFakeFromFitVal[ikin], _nFakeFromFitErr[ikin]);

  if (!noPrint){
    std::cout<<"Yields in "<<StrLabelKin(ikin)<<": "<<std::endl;
    for (int ieta1=_BARREL; ieta1<=_ENDCAP; ieta1++){
      if ((ieta!=_COMMON) && (ieta!=ieta1)) continue;
      std::cout<<setprecision(2)<<std::endl;
      std::cout<<StrLabelEta(ieta1)<<": "<<std::endl;
      std::cout<<"_hData="<<_hData[ikin][ieta1]->GetSumOfWeights()<<"; true+fake="<<_nTrueYieldsVal[ikin][ieta1]+_nFakeYieldsVal[ikin][ieta1]<<std::endl; 
    }  
  }//end of if (!noPrint)
}

void TTemplatesRandCone::ComputeOneYield(int ikin, int ieta, bool noPrint, bool isTrueGamma,
	double*  nYieldsVal,double* nYieldsErr,double* nFromFitVal, double* nFromFitErr)
{
  int nPassed[2];
  int nPassedFitVar[2];
  int nTot[2];
  float eff[2];
  TCut cutSidebandVar;

  for (int ieta1=_BARREL; ieta1<=_ENDCAP; ieta1++){

    if (ieta!=_COMMON && ieta!=ieta1) continue;

    if (isTrueGamma) cutSidebandVar=_pars.cutSidebandVarNominalRange;
    else cutSidebandVar=SidebandCut(ikin,ieta1);

    TCut cutTot = _pars.cutAdd && cutSidebandVar && CutEtaBin(ieta1) && CutKinBin(ikin) && FitVarFitRangeCut(ikin,ieta1);
    TCut cutPassed = _pars.cutAdd && cutSidebandVar && CutEtaBin(ieta1) && CutKinBin(ikin) && _pars.cutNominalExceptSidebandVar;

    if (isTrueGamma){
      TH1F* hSignPassed = new TH1F("histPassed","histPassed",1,-3.0,3.0);
      _pars.treeSign->Draw(_pars.varPhoEta+TString(">>histPassed"),cutPassed*_pars.cutWeight,"goff");
      TH1F* hSignTot = new TH1F("histTot","histTot",1,-3.0,3.0);
      _pars.treeSign->Draw(_pars.varPhoEta+TString(">>histTot"),cutTot*_pars.cutWeight,"goff");
      TH1F* hEff = (TH1F*)hSignPassed->Clone();
      hEff->Divide(hSignTot);
      eff[ieta1]=hEff->GetBinContent(1);
      delete hSignPassed;
      delete hSignTot;
    }// end of isTrueGamma
    else{
      TH1F* hFakePassed = new TH1F("histPassedF","histPassedF",1,-3.0,3.0);
      _pars.treeFake->Draw(_pars.varPhoEta+TString(">>histPassedF"),cutPassed*_pars.cutWeight,"goff");
      TH1F* hFakeTot = new TH1F("histTotF","histTotF",1,-3.0,3.0);
      _pars.treeFake->Draw(_pars.varPhoEta+TString(">>histTotF"),cutTot*_pars.cutWeight,"goff");

      if (!_pars.noLeakSubtr){
        TH1F* hSignPassed = new TH1F("histPassed","histPassed",1,-3.0,3.0);
        _pars.treeSign->Draw(_pars.varPhoEta+TString(">>histPassed"),cutPassed*_pars.cutWeight,"goff");
        TH1F* hSignTot = new TH1F("histTot","histTot",1,-3.0,3.0);
        _pars.treeSign->Draw(_pars.varPhoEta+TString(">>histTot"),cutTot*_pars.cutWeight,"goff");
        hFakePassed->Add(hSignPassed,-1);
        hFakeTot->Add(hSignTot,-1);
        delete hSignPassed;
        delete hSignTot;        
      }
      TH1F* hEff = (TH1F*)hFakePassed->Clone();
      hEff->Divide(hFakeTot);
      eff[ieta1]=hEff->GetBinContent(1);
      delete hFakePassed;
      delete hFakeTot;
    }// end of else of if(isTrueGamma)

    nYieldsVal[ieta1]=nFromFitVal[ieta1]*eff[ieta1];
    nYieldsErr[ieta1]=nFromFitErr[ieta1]*eff[ieta1];
  }//end of loop over ieta

  if (ieta==_COMMON){
    nYieldsVal[_COMMON]=nYieldsVal[_BARREL]+nYieldsVal[_ENDCAP];
    double valsqr = nYieldsErr[_BARREL]*nYieldsErr[_BARREL]+nYieldsErr[_ENDCAP]*nYieldsErr[_ENDCAP];
    if (valsqr>=0) nYieldsErr[_COMMON]=sqrt(valsqr);
    else nYieldsErr[_COMMON]=0;
  }//end of if (ieta==_COMMON)

  if (!noPrint){
    for (int ieta1=_BARREL; ieta1<=_ENDCAP; ieta1++){
      if (ieta!=_COMMON && ieta!=ieta1) continue;
      std::cout<<setprecision(2)<<std::endl;
      std::cout<<StrLabelEta(ieta1)<<": "<<std::endl;
      std::cout<<"cutEta="<<CutEtaBin(ieta1).GetTitle()<<std::endl;
      std::cout<<"cutKin="<<CutKinBin(ikin).GetTitle()<<std::endl;
      std::cout<<"cutSidebandVar="<<cutSidebandVar.GetTitle()<<std::endl;
      std::cout<<"cutNominal="<<_pars.cutNominal.GetTitle()<<std::endl;
      TString strTrueOrFake;
      if (isTrueGamma) strTrueOrFake="nTrue";
      else strTrueOrFake="nFake";
      std::cout<<"fromFit*eff="<<strTrueOrFake<<"=("<<nFromFitVal[ieta1]<<"+-"<<nFromFitErr[ieta1]<<")*"<<eff[ieta1]<<"="<<nYieldsVal[ieta1]<<"+-"<<nYieldsErr[ieta1]<<";"<<std::endl;
    }//loop over ieta
  }//end of if (!noPrint)
}

void TTemplatesRandCone::PrintYieldsAndChi2()
{

  std::cout<<"Histograms:"<<std::endl;
  PrintHistogramsBinByBin(_hTrue);
  PrintHistogramsBinByBin(_hFake);
  PrintHistogramsBinByBin(_hTrueReference);
  PrintHistogramsBinByBin(_hFakeReference);
  std::cout<<std::endl;

  std::cout<<"Fake and True values from fit:"<<std::endl;
  std::cout<<"  bin            Fake-B     True-B  Leak%-B    Fake-E    True-E  Leak%-E"<<std::endl;
  for (int ikin=0; ikin<_pars.nKinBins+1; ikin++){
    std::cout<<ikin<<", "<<StrLabelKin(ikin)<<": ";
    for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
     std::cout<<std::setw(6)<<std::setprecision(0)<<std::fixed;
//      std::cout<<std::setw(6)<<std::setprecision(0)<<_nFakeFromFitVal[ikin][ieta]<<"+-";
//      std::cout<<std::setw(5)<<std::setprecision(0)<<_nFakeFromFitErr[ikin][ieta]<<"; ";
//      std::cout<<std::setw(6)<<std::setprecision(0)<<_nTrueFromFitVal[ikin][ieta]<<"+-";
//      std::cout<<std::setw(5)<<std::setprecision(0)<<_nTrueFromFitErr[ikin][ieta]<<"; ";
      std::cout<<_nFakeFromFitVal[ikin][ieta]<<"+-";
      std::cout<<_nFakeFromFitErr[ikin][ieta]<<"; ";
      std::cout<<_nTrueFromFitVal[ikin][ieta]<<"+-";
      std::cout<<_nTrueFromFitErr[ikin][ieta]<<"; ";
      if (_hFake[ikin][ieta]->GetSumOfWeights()!=0){
        std::cout<<100.0*_hLeak[ikin][ieta]->GetSumOfWeights()/(_hLeak[ikin][ieta]->GetSumOfWeights()+_hFakeReference[ikin][ieta]->GetSumOfWeights())<<"%; ";
      }
    }
    std::cout<<std::endl;
  }
  std::cout<<std::endl;

  std::cout<<"Chi2/NDF for different bins: "<<std::endl;
  for (int ikin=0; ikin<_pars.nKinBins+1; ikin++){
    std::cout<<"ikin="<<ikin<<", "<<StrLabelKin(ikin)<<": ";
    for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
      std::cout<<StrLabelEta(ieta)<<": chi2/ndf=";
      std::cout<<_chi2ToNDF[ikin][ieta]<<",  nFitBins="<<_pars.nFitBins[ikin][ieta]<<"; ";
    }
    std::cout<<std::endl;  
  }
  std::cout<<std::endl;

  std::cout<<"Fake yields:"<<std::endl;
  for (int ikin=0; ikin<_pars.nKinBins+1; ikin++){
    std::cout<<"ikin="<<ikin<<", "<<StrLabelKin(ikin);
    for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
      std::cout<<StrLabelEta(ieta)<<": "<<_nFakeYieldsVal[ikin][ieta]<<"+-"<<_nFakeYieldsErr[ikin][ieta]<<"; ";
    }
    std::cout<<std::endl;
  }
  std::cout<<"True yields:"<<std::endl;
  for (int ikin=0; ikin<_pars.nKinBins+1; ikin++){
    std::cout<<"ikin="<<ikin<<", "<<StrLabelKin(ikin);
    for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
      std::cout<<StrLabelEta(ieta)<<": "<<_nTrueYieldsVal[ikin][ieta]<<"+-"<<_nTrueYieldsErr[ikin][ieta]<<"; ";
    }
    std::cout<<std::endl;
  }

}

void TTemplatesRandCone::PrintHistogramsBinByBin(TH1D* hist[nKinBinsMax][3]){
  std::cout<<"TH1D* hists "<<hist[0][0]->GetTitle()<<": "<<std::endl;
  std::cout<<std::setprecision(0)<<std::fixed;
  for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
    for (int ikin=0; ikin<_pars.nKinBins+1; ikin++)
      PrintOneHistogramBinByBin(hist, ikin, ieta);
  }
}

void TTemplatesRandCone::PrintOneHistogramBinByBin(TH1D* hist[nKinBinsMax][3], int ikin, int ieta){
  std::cout<<hist[ikin][ieta]->GetTitle()<<": ";
//  std::cout<<StrLabelEta(ieta)<<StrLabelKin(ikin)<<": ";
  float sum=0;
  std::cout<<std::setprecision(0)<<std::fixed;
  for (int ifit=1; ifit<=hist[ikin][ieta]->GetNbinsX(); ifit++){
    std::cout<<std::setw(6)<<hist[ikin][ieta]->GetBinContent(ifit)<<"; ";
    sum+=hist[ikin][ieta]->GetBinContent(ifit);
  }
  std::cout<<"sum="<<sum<<std::endl;
}

void TTemplatesRandCone::PlotTemplates()
{

  for (int ikin=0; ikin<_pars.nKinBins+1; ikin++){
    for (int ieta=_BARREL; ieta<=_ENDCAP; ieta++){
      PlotOneTemplate(ikin, ieta);
    }//end of loop over ieta
  }//end of loop over ikin

}


void TTemplatesRandCone::PlotOneTemplate(int ikin, int ieta)
{

  TString strBin = StrLabelKin(ikin)+StrLabelEta(ieta);
  TString strChi = "#chi^{2}/ndf=";
  float chiFloat=(100*_chi2ToNDF[ikin][ieta]);
  int chiInt=chiFloat;    
  strChi+=(chiInt/100);
  strChi+=".";
  strChi+=(chiInt%100);
  TLatex* textChi2; 
  textChi2 =new TLatex(0.75,0.65,strChi);
  textChi2->SetNDC();

  TText* textBin; 
  textBin =new TText(0.55,0.75,strBin);
  textBin->SetNDC();


  TString strRatio="hRatio";
  strRatio+=ikin;
  strRatio+=ieta;
  TString strSum="hSum";
  strSum+=ikin;
  strSum+=ieta;
  _hRatio[ikin][ieta]=(TH1D*)_hData[ikin][ieta]->Clone(strRatio);
  _hRatio[ikin][ieta]->Sumw2();
  _hSumm[ikin][ieta]=(TH1D*)_hData[ikin][ieta]->Clone(strSum);
  
  float areaHistTrue=0;
  float areaHistFake=0;
  for (int ib=1; ib<=_hTrue[ikin][ieta]->GetNbinsX(); ib++){
    areaHistTrue+=_hTrue[ikin][ieta]->GetBinContent(ib);
    areaHistFake+=_hFake[ikin][ieta]->GetBinContent(ib);
  }

  for (int ib=1; ib<=_hSumm[ikin][ieta]->GetNbinsX(); ib++){

    float contT=0;
    float err1T=0;
    float err2T=0;
    float contF=0;
    float err1F=0;
    float err2F=0;

    if (areaHistTrue==0);
    else{
      contT = _hTrue[ikin][ieta]->GetBinContent(ib)*_nTrueFromFitVal[ikin][ieta]/areaHistTrue;
      err1T = (_hTrue[ikin][ieta]->GetBinError(ib)*_nTrueFromFitVal[ikin][ieta])/areaHistTrue;
      err2T = (_hTrue[ikin][ieta]->GetBinContent(ib)*_nTrueFromFitErr[ikin][ieta])/areaHistTrue;
      _hTrue[ikin][ieta]->SetBinContent(ib,contT);
      _hTrue[ikin][ieta]->SetBinError(ib,sqrt(err1T*err1T+err2T*err2T));
    }
    if (areaHistFake==0);
    else {
      contF = _hFake[ikin][ieta]->GetBinContent(ib)*_nFakeFromFitVal[ikin][ieta]/areaHistFake;
      err1F = (_hFake[ikin][ieta]->GetBinError(ib)*_nFakeFromFitVal[ikin][ieta])/areaHistFake;
      err2F = (_hFake[ikin][ieta]->GetBinContent(ib)*_nFakeFromFitErr[ikin][ieta])/areaHistFake;
      _hFake[ikin][ieta]->SetBinContent(ib,contF);
      _hFake[ikin][ieta]->SetBinError(ib,sqrt(err1F*err1F+err2F*err2F));
    }
//    _hTrue[ikin][ieta]->SetBinContent(ib,contS);
//    _hTrue[ikin][ieta]->SetBinError(ib,sqrt(err1S*err1S+err2S*err2S));

//    _hFake[ikin][ieta]->SetBinContent(ib,contB);
//    _hFake[ikin][ieta]->SetBinError(ib,sqrt(err1B*err1B+err2B*err2B));

    float err = sqrt(err1T*err1T+err2T*err2T+err1F*err1F+err2F*err2F);
    _hSumm[ikin][ieta]->SetBinContent(ib,contT+contF);
    _hSumm[ikin][ieta]->SetBinError(ib,err);

  }  

  _hRatio[ikin][ieta]->Divide(_hSumm[ikin][ieta]);


    TString cName=_pars.strPlotsDir+_pars.strPlotsBaseName;
    cName+=StrLabelKin(ikin);
    cName+=StrLabelEta(ieta);
    TCanvas* c1 = new TCanvas(cName,cName, 800, 600);

    c1->Divide(1,2);
    TPad* pad1 = (TPad*)c1->GetPad(1);
    TPad* pad2 = (TPad*)c1->GetPad(2);
    pad1->SetPad(0,0.3,1.0,1.0);
    pad2->SetPad(0,0,  1.0,0.28);
    pad1->SetLeftMargin(0.18);
    pad1->SetTopMargin(0.08);
    pad1->SetRightMargin(0.07);
    pad1->SetBottomMargin(0.01); // All X axis labels and titles are thus cut off
    pad2->SetLeftMargin(0.18);
    pad2->SetTopMargin(0.01);
    pad2->SetRightMargin(0.07);
    pad2->SetBottomMargin(0.45);
    pad1->cd();

    _plotter[ikin][ieta]->SetTitle("");
    _plotter[ikin][ieta]->Draw();

    _hSumm[ikin][ieta]->SetLineColor(7);
    _hSumm[ikin][ieta]->SetLineWidth(2);
    _hSumm[ikin][ieta]->Draw("HIST same"); 
    _hSumm[ikin][ieta]->SetLineColor(2);  
    _hSumm[ikin][ieta]->Draw("EP same"); 
    _hTrue[ikin][ieta]->SetLineWidth(2);
    _hTrue[ikin][ieta]->SetLineColor(3);
    _hTrue[ikin][ieta]->Draw("EP same");
    _hFake[ikin][ieta]->SetLineWidth(2);
    _hFake[ikin][ieta]->SetLineColor(4);
    _hFake[ikin][ieta]->Draw("EP same");
    _hData[ikin][ieta]->SetLineWidth(2);
    _hData[ikin][ieta]->Draw("EP same");

    textBin->Draw("same");
    textChi2->Draw("same");

    pad2->cd();
    _hRatio[ikin][ieta]->SetLineWidth(2);
    _hRatio[ikin][ieta]->SetStats(0);
    _hRatio[ikin][ieta]->GetYaxis()->SetLabelSize(0.1);
    _hRatio[ikin][ieta]->GetXaxis()->SetLabelSize(0.1);
    _hRatio[ikin][ieta]->GetXaxis()->SetTitleOffset(1.0);
    _hRatio[ikin][ieta]->GetXaxis()->SetTitleSize(0.12);
    _hRatio[ikin][ieta]->SetTitle(TString("; ")+_pars.varFit+TString(" ;"));
    _hRatio[ikin][ieta]->Draw();
    int nBins = _hRatio[ikin][ieta]->GetNbinsX();
    TLine* line = new TLine(_hRatio[ikin][ieta]->GetBinLowEdge(1),1,_hRatio[ikin][ieta]->GetBinLowEdge(nBins)+_hRatio[ikin][ieta]->GetBinWidth(nBins),1);
    line->SetLineWidth(2);
    line->SetLineStyle(9);
    line->Draw("same");
   
    cName.ReplaceAll("-1","total");
    cName+=".png";
    c1->SaveAs(cName);
    cName.ReplaceAll(".png",".pdf");
    c1->SaveAs(cName);
    cName.ReplaceAll(".pdf",".root");
    c1->SaveAs(cName);

//  cName+="plotter";
//  TCanvas* c2 = new TCanvas(cName);
//  _plotter[ikin][ieta]->Draw();
}// end of plotOneTemplate

TCut TTemplatesRandCone::SidebandCut(int ikin, int ieta)
{
  float lim=_pars.sideband[ikin][ieta]; 
  float limUp=_pars.sidebandUp[ikin][ieta]; 
  TString strCut=_pars.varSideband;
  strCut+=">";
  strCut+=lim;
  strCut+=" && ";
  strCut+=_pars.varSideband;
  strCut+="<";
  strCut+=limUp;
  TCut cut(strCut);
  if (ieta==_BARREL) cut = cut && _pars.cutBarrel;
  if (ieta==_ENDCAP) cut = cut && _pars.cutEndcap;
  return ( cut ); 
}

TCut TTemplatesRandCone::SidebandVarNominalCut()
{
  return _pars.cutSidebandVarNominalRange;
}

TCut TTemplatesRandCone::FitVarFitRangeCut(int ikin, int ieta)
{
  float lim=_pars.maxVarFit[ikin][ieta]; 
  TString strCut=_pars.varFit;
  strCut+="<";
  strCut+=lim;
  TCut cut(strCut);
  if (ieta==_BARREL) cut = cut && _pars.cutBarrel;
  if (ieta==_ENDCAP) cut = cut && _pars.cutEndcap;
  return ( cut );
}

void TTemplatesRandCone::SaveYields()
{
  _pars.fOutForSave->cd();

  for (int ieta=_BARREL; ieta<=_COMMON; ieta++){
    //write fake yields computed from fit
    //which is jets to gamma background
    TString strTot=_pars.strFakeYieldsTot[ieta];
    TH1F hTotFakeYield(strTot,strTot,1,_pars.kinBinLims[0],_pars.kinBinLims[_pars.nKinBins]);
    TString str1D=_pars.strFakeYields1D[ieta];
    TH1F h1DFakeYield(str1D,str1D,_pars.nKinBins,_pars.kinBinLims);
    hTotFakeYield.SetBinContent(1,_nFakeYieldsVal[0][ieta]);
    hTotFakeYield.SetBinError(1,_nFakeYieldsErr[0][ieta]);
    for (int i=1; i<_pars.nKinBins+1; i++){  
      h1DFakeYield.SetBinContent(i,_nFakeYieldsVal[i][ieta]);
      h1DFakeYield.SetBinError(i,_nFakeYieldsErr[i][ieta]);
    }
    hTotFakeYield.Write(strTot); 
    h1DFakeYield.Write(str1D); 

    //write true yields computed from fit
    //which is true gamma background plus signal
    strTot=_pars.strTrueYieldsTot[ieta];
    TH1F hTotTrueYield(strTot,strTot,1,_pars.kinBinLims[0],_pars.kinBinLims[_pars.nKinBins]);
    str1D=_pars.strTrueYields1D[ieta];
    TH1F h1DTrueYield(str1D,str1D,_pars.nKinBins,_pars.kinBinLims);
    hTotTrueYield.SetBinContent(1,_nTrueYieldsVal[0][ieta]);
    hTotTrueYield.SetBinError(1,_nTrueYieldsErr[0][ieta]);
    for (int i=1; i<_pars.nKinBins+1; i++){  
      h1DTrueYield.SetBinContent(i,_nTrueYieldsVal[i][ieta]);
      h1DTrueYield.SetBinError(i,_nTrueYieldsErr[i][ieta]);
    }
    hTotTrueYield.Write(strTot); 
    h1DTrueYield.Write(str1D); 
    if (ieta==_COMMON) continue;
    for (int i=0; i<_pars.nKinBins+1; i++){  
      _hTrue[i][ieta]->Write(TString("templateTrue")+StrLabelKin(i)+StrLabelEta(ieta));
      _hFake[i][ieta]->Write(TString("templateFake")+StrLabelKin(i)+StrLabelEta(ieta));
      _hData[i][ieta]->Write(TString("dataToFit")+StrLabelKin(i)+StrLabelEta(ieta));
      _plotter[i][ieta]->Write(TString("plotter")+StrLabelKin(i)+StrLabelEta(ieta));
    }//end of loop over i
  }//end of loop over ieta

 // if (_pars.fOutForSave->IsOpen()) _pars.fOutForSave->Close();
}

TCut TTemplatesRandCone::CutKinBin(int ikin){

  if (ikin<0 || ikin>_pars.nKinBins){
    TCut cut("0");
    return cut;
  }
  float min;
  float max;
  if (ikin==0){
    min=_pars.kinBinLims[0];
    max=_pars.kinBinLims[_pars.nKinBins];
  }
  else{
    min=_pars.kinBinLims[ikin-1];
    max=_pars.kinBinLims[ikin];
  }

  TString ikinCutStr;
  ikinCutStr=_pars.varKin;
  ikinCutStr+=">";
  ikinCutStr+=min;
  ikinCutStr+=" && ";
  ikinCutStr+=_pars.varKin;
  ikinCutStr+="<=";
  ikinCutStr+=max;
  TCut cut(ikinCutStr);
  return cut;
}

TCut TTemplatesRandCone::CutEtaBin(int ieta){
  if (ieta==_BARREL)      return _pars.cutBarrel;
  else if (ieta==_ENDCAP) return _pars.cutEndcap;
  else if (ieta==_COMMON) return (_pars.cutBarrel || _pars.cutEndcap);
  return "0";
}
