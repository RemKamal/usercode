#ifndef TPrepareYields_h
#define TPrepareYields_h

#include "TH1F.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TTree.h"
#include "THStack.h"
#include "TLatex.h"
#include "TCut.h"

#include <vector>

class TPrepareYields
{
  public:

    TPrepareYields();
    virtual ~TPrepareYields();

    enum {DATA, SIGMC, BKGMC, BKGMC_TRUE, BKGMC_FAKE, DATA_DRIVEN};//sourceType

    struct PrepareYieldsPars{
      TString varKin;
      int nKinBins; 
      float kinBinLims[100];
      float blindFraction;
      TString varWeight;
      TCut cutBarrel;
      TCut cutEndcap;
      TCut cutAdd;
      bool doLogX;
      bool doLogY;
      TString varKinLabel;
      TString strFileOut;
      TFile* fOut;//should be set up inside
      TCut cutKin;//should be set up inside
      TCut cutWeight;//should be set up inside
      TString strPlotsDir;
      TString strPlotsBaseName;
    };

    struct YieldsSource{
      int sourceType;
      TTree* tr;
      TString name;
      TString label;
      int color;
      TH1F* hist[3];//_BARREL, _ENDCAP, _COMMON
      float yieldTotVal[3];//_BARREL, _ENDCAP, _COMMON
      float yieldTotErr[3];
      TString strYieldsName1D[3];//must have default values
      TString strYieldsNameTot[3];//must have default values
    };

    void SetPars(PrepareYieldsPars pars);

    bool SetOneYieldSource(int sourceType, TString name, TString label, int color, TString fileName, TString treeName);
    bool SetOneYieldSource(int sourceType, TString name, TString label, int color, TString fileName, TString treeName, TString strYieldsName1D[3], TString strYieldsNameTot[3]);

    void PlotPrintSave();

  protected:

    enum {_BARREL=0, _ENDCAP=1, _COMMON=2};

    PrepareYieldsPars _pyPars;
    vector <YieldsSource> _sources;

    TString StrLabelEta(int ieta);
    TCut CutEta(int ieta);

    void CompareTotalDATAvsMC(int ieta);
    void CompareStackVsHist(TString plotTitle, TH1F* hist1, TH1F* hist2, TLegend* legend, TCanvas* canv, bool isStack=0, THStack* stack=0);

    void PrintYieldsOne(TString strYieldType, float totVal, float totErr, TH1F* yieldHist);



    TH1F* _sigMCGenYields[3];
    float _sigMCGenYieldTot[3];
    float _sigMCGenYieldTotErr[3];

    TCanvas* _canvTotalDATAvsMC[3];

};


#endif //TPrepareYields_h
