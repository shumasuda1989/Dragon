#include "CDomino.h"

using namespace std;

//CDomino
ClassImp(CDomino)

void CDomino::SetCell()
{
  for(int i=0;i<READDEPTH;i++)
    dCell[i]=(dStopCell+i)%CELLNUM;
}

/*
void CDomino::ExcludeSpike(COffset *o1,int firstbin, int lastbin)
{
  double comp = 98;// comp/2

  if(dStopCell%2==0)
    for(int i=(int)(firstbin/2);i<=(int)(lastbin/2);i++)
      {
	if(fY[i*2]+fY[i*2+1] > fY[i*2-1]+fY[i*2+2] +comp)
	  {
	    fY[i*2] -= o1->GetSpike(dCell[i*2]);
	    fY[i*2+1] -= o1->GetSpike(dCell[i*2+1]);
	  }
      }
  else
    for(int i=(int)((firstbin+1)/2);i<=(int)((lastbin+1)/2);i++)
      {
	if(fY[i*2-1]+fY[i*2] > fY[i*2-2]+fY[i*2+1] +comp)
	  {
	    fY[i*2-1] -= o1->GetSpike(dCell[i*2-1]);
	    fY[i*2] -= o1->GetSpike(dCell[i*2]);
	  }
      }

}

int CDomino::ExcludeSpike(COffset *o1,int firstbin, int lastbin,int *x)
{
  double comp = 98;// comp/2

        int spike = 0;
	int tmp[512];

  if(dStopCell%2==0)
	for(int i=(int)(firstbin/2);i<=(int)(lastbin/2);i++)
	  {
		if(fY[i*2]+fY[i*2+1] > fY[i*2-1]+fY[i*2+2] +comp)
		  {
		    fY[i*2] -=  o1->GetSpike(dCell[i*2]);
		    fY[i*2+1] -=  o1->GetSpike(dCell[i*2+1]);
		    tmp[spike]=i*2;
		    spike++;
		  }
	  }

  else
    for(int i=(int)((firstbin+1)/2);i<=(int)((lastbin+1)/2);i++)
      {
	if(fY[i*2-1]+fY[i*2] > fY[i*2-2]+fY[i*2+1] +comp)
	  {
	    fY[i*2-1] -= o1->GetSpike(dCell[i*2-1]);
	    fY[i*2] -= o1->GetSpike(dCell[i*2]);
		    tmp[spike]=i*2-1;
		    spike++;
	  }
      }


	if(spike!=0 && x!=0)
	  for(int i=0;i<spike;i++)
	    x[i]=tmp[i];

	return spike;
}
*/

double CDomino::GetPulseHeight(int fcell,int lcell)
{
  double max=0;
  for(int i=fcell;i<=lcell;i++)
    if(max<fY[i]) max=fY[i];
  return max;
}

double CDomino::GetCharge(int firstbin, int lastbin)
{
  double integ=0;
  for(int i=firstbin;i<lastbin+1;i++)   integ += fY[i];

  return integ;
}

int CDomino::GetTiming(int firstbin, int lastbin)
{
	int timing=firstbin;
	double peak=fY[firstbin];
	for(int i=firstbin;i<lastbin+1;i++)
	{
		if(peak < fY[i])
		{
			peak = fY[i];
			timing = i;
		}
	}

	return timing;
}

int CDomino::GetTiming3(int firstbin, int lastbin)
{
	int timing=firstbin;
	double peak=fY[firstbin-1]+fY[firstbin]+fY[firstbin+1];
	for(int i=firstbin+1;i<lastbin;i++)
	{
		if(peak < fY[i-1]+fY[i]+fY[i+1])
		{
			peak = fY[i-1]+fY[i]+fY[i+1];
			timing = i;
		}
	}

	return timing;
}

int CDomino::GetTiming5(int firstbin, int lastbin)
{
	int timing=firstbin;
	double peak=fY[firstbin-2]+fY[firstbin-1]+fY[firstbin]+fY[firstbin+1]+fY[firstbin+2];
	for(int i=firstbin+1;i<lastbin-2;i++)
	{
		if(peak < fY[i-2]+fY[i-1]+fY[i]+fY[i+1]+fY[i+2])
		{
		        peak = fY[i-2]+fY[i-1]+fY[i]+fY[i+1]+fY[i+2];
			timing = i;
		}
	}

	return timing;
}

double CDomino::GetAveTiming(int firstbin, int lastbin)
{

  double timing;

  TH1D* hTmp = new TH1D("hTmp","Time slices weighed by ADC",lastbin+1-firstbin,firstbin,lastbin+1);

  for(int i=firstbin;i<lastbin+1;i++)
    {
      hTmp->Fill(i,fY[i]);
    }

  timing = hTmp->GetMean();
  delete hTmp;

  return timing;
}

void CDomino::ChargeSearch(int firstbin, int lastbin, int window)
{
  double tmp;

  for(int i=firstbin;i<lastbin-window+1;i++)
    {
      tmp=this->GetCharge(i,i+window-1);
      if(i==firstbin || tmp>dIntegADC)
	{
	  dIntegADC=tmp;
	  dWindowLeft=i;
	  dWindowRight=i+window-1;
	}
    }

  dArrTime = this->GetAveTiming(dWindowLeft,dWindowRight);

}

//COUNT
ClassImp(COUNT)

COUNT::COUNT() : TObject()
{
  EvtCnt=0;
  PPS=0;
  TrgCnt=0;
  ClkCnt=0;
  ClkCnt10M=0;
}

