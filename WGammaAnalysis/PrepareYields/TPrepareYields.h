#ifndef TPrepareYields_h
#define TPrepareYields_h

#include "../Configuration/TConfiguration.h"
#include "../Configuration/TAllInputSamples.h"

#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TTree.h"

#include <vector>

class TPrepareYields
{
  public:
    TPrepareYields(int channel);
    virtual ~TPrepareYields();
    void PrepareYields();
    void SetYields();
    void SetYieldsOneSource(int iSource);
    void SetYieldsDDBkgTemplate();
    void SubtractBackground();
    void CompareFakeBkgDDvsMC();
    void CompareTotalDATAvsSIGplusBKG();
    void CompareTotalDATAvsMC();
    void CompareSignalDATAvsMC();
    void PrintYields();
    void StoreYields();

    void SetTotalYield(TTree* tr, TCut cut, float& val, float& err);

  private:
    TConfiguration _config;
    int _channel;
    TAllInputSamples* _INPUT;

    TFile* _fOut;

    TH1F* _dataYields;
    float _dataYieldTot;
    float _dataYieldTotErr;
    
    TH1F* _sigMCYields;
    float _sigMCYieldTot;
    float _sigMCYieldTotErr;


    TH1F* _sigMCGenYields;
    float _sigMCGenYieldTot;
    float _sigMCGenYieldTotErr;

    vector <TH1F*> _vecBkgMCYields;
    vector <float> _vecBkgMCYieldTot;
    vector <float> _vecBkgMCYieldTotErr;

    vector <TH1F*> _vecBkgMCTrueGammaYields;
    vector <float> _vecBkgMCTrueGammaYieldTot;
    vector <float> _vecBkgMCTrueGammaYieldTotErr;
    vector <TH1F*> _vecBkgMCFakeGammaYields;
    vector <float> _vecBkgMCFakeGammaYieldTot;
    vector <float> _vecBkgMCFakeGammaYieldTotErr;

    TH1F* _bkgDDFakeGammaYields;
    float _bkgDDFakeGammaYieldTot;
    float _bkgDDFakeGammaYieldTotErr;


    TH1F* _signalDataYields;
    float _signalDataYieldTot;
    float _signalDataYieldTotErr;

    TH1F* _floatingHist;


    TLegend* _legend;

    TCanvas* _canvDDvsMC;
    TCanvas* _canvTotalDATAvsSIGplusBKG;
    TCanvas* _canvSignalDATAvsMC;
    TCanvas* _canvTotalDATAvsMC;
    

};


#endif //TPrepareYields_h
