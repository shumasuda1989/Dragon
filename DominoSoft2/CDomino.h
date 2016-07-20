//KONNO Yusuke
//Dragon readout software
//Class header for Domino
//KONNO Yusuke
//Kyoto University
//last update: 2013.05.26

#ifndef CDomino_H
#define CDomino_H

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <cmath>
#include <vector>

#include <TApplication.h>
#include <TCanvas.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include <TStyle.h>
#include <TH1.h>
#include <TROOT.h>
#include <TF1.h>
#include <TFile.h>
#include <TTree.h>
#include <TString.h>

#include "Config.h"
#include "DSubFunc.h"


//class to process domino data
class CDomino : public TGraph
{
 private:
  bool dIsRaw;
  UShort_t dStopCell;
  int dCell[READDEPTH]; //!
  double dMinADC; //!
  double dArrTime; //!
  double dIntegADC; //!
  int dWindowLeft; //!
  int dWindowRight; //!

 public:
  CDomino(int rd=READDEPTH) : TGraph(rd) { dIsRaw =false;}
  ~CDomino(){}
  double *GetADC(){ return fY;}
  double GetADC(int slice){ return fY[slice];}
  double operator[](int slice){ return fY[slice];}
  int GetCell(int slice){ return dCell[slice];}
  void SetStopCell(int cell){ dStopCell=cell;}
  void SetCell();
  UShort_t GetStopCell(){ return dStopCell;}
  void SetIsRaw(bool tf){ dIsRaw=tf; }
  bool IsRaw(){ return dIsRaw; }
  //void ExcludeSpike(COffset*,int firstbin,int lastbin);
  //int ExcludeSpike(COffset*,int firstbin, int lastbin,int *x);
  double GetPulseHeight(int fcell,int lcell);
  double GetMinimum(){ return dMinADC;}
  double GetCharge(int firstbin, int lastbin);
  int GetTiming(int firstbin, int lastbin); //maximum search
  int GetTiming3(int firstbin, int lastbin); //maximum search by 3 point
  int GetTiming5(int firstbin, int lastbin); //maximum search by 5 point
  double GetAveTiming(int firstbin, int lastbin);
  void ChargeSearch(int firstbin, int lastbin, int window);
  double GetIntegADC(){ return dIntegADC;}
  double GetArrTime(){ return dArrTime;}
  int GetWindowLeft(){ return dWindowLeft;}
  //void Streamer(TBuffer &R__b);

  ClassDef(CDomino,1)

};

//container of event, trigger and clock counts
class COUNT : public TObject
{
 public:
  UShort_t PPS;
  UInt_t ClkCnt10M; //usec
  UInt_t EvtCnt;
  UInt_t TrgCnt;
  ULong64_t ClkCnt; //usec

  COUNT();
  ~COUNT(){}

  ClassDef(COUNT,1)

};

#endif
