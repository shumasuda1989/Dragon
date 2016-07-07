#include <TFile.h>
#include <TTree.h>
#include <TProfile.h>

#include "Config.h"
#include "CDomino.h"
#include "DSubFunc.h"

int Rootify(const char *datfile, const TString pedfile=0);
bool SetOffset(const char *filename, Double_t **offset);
inline void Decoder(ifstream &fin, CDomino **d, COUNT *c, int rd, Double_t **offset=0);
template <typename T> inline void Reader(ifstream &fin, const int bytes, T &buf);
inline int ReadDepthChecker(ifstream &fin);
inline int EvNumChecker(ifstream &fin, int rd);

using namespace std;

#ifndef __CINT__
int main(int argc, char **argv)
{

  if(argc!=2 && argc!=3){
    cerr << "Usage: " << argv[0] << " datafile.dat [offset.root[dat]]" << endl;
    return 1;
  }

  TApplication app("app", &argc, argv);

  if(app.Argc()==3)
    return Rootify(app.Argv(1),app.Argv(2));
  else 
    return Rootify(app.Argv(1));

}
#endif

int Rootify(const char* datfile, const TString pedfile)
{
  const int Nch=16;

  Double_t **offset=0;

  ///// SET PEDESTAL TABLE /////
  if(!pedfile.IsNull()){

    offset=new Double_t*[Nch];
    for(int ch=0;ch<Nch;ch++)
      offset[ch]=new Double_t[CELLNUM];

    if(pedfile.EndsWith(".root")){
      TFile *of=new TFile(pedfile);
      TProfile *p;
      for(int ch=0;ch<Nch;ch++){
	p=(TProfile*)of->Get(Form("prof%cch%d",ch<8?'H':'L',ch%8));
	for(int cell=0;cell<CELLNUM;cell++)
	  offset[ch][cell]=p->GetBinContent(cell+1);
      }
      of->Close();
    } else if(pedfile.EndsWith(".dat")){
      if(!SetOffset(pedfile.Data(),offset)) 
	return 1;
    } else{
      cerr << "Use a *.dat or *.root for pedestal file" << endl;
      return 1;
    }

  }

  TString rootfile=datfile;
  if(!rootfile.EndsWith(".dat")) {
    cerr << "Use a *.dat file" << endl;
    return 1;
  }
  rootfile.ReplaceAll(".dat",".root");
  TFile f(rootfile,"RECREATE");

  ifstream fin(datfile, ios::in | ios::binary);
  if(!fin) {
    cerr << "Cannot Open Data File: " << datfile  << endl;
    return 1;
  }

  int rd=ReadDepthChecker(fin);
  int Nev=EvNumChecker(fin,rd);

  TTree tr("Events","");
  tr.SetAutoSave(Nev);
  tr.SetAutoFlush(0);

  CDomino *ADCch[Nch];
  COUNT *cnt=new COUNT;

  char hl[2]={'H','L'};
  for(int ch=0;ch<Nch;ch++){
    ADCch[ch] = new CDomino(rd);
    tr.Branch(Form("%cch%d",hl[ch/8],ch%8),"CDomino",&ADCch[ch]);
    ADCch[ch]->SetName(Form("%cch%d",hl[ch/8],ch%8));
    if(offset){
      ADCch[ch]->SetTitle(Form("%cch%d;Time slice;ADC (Pedestal Subtracted)",
			       hl[ch/8],ch%8));
      ADCch[ch]->SetIsRaw(false);
    }
    else{
      ADCch[ch]->SetTitle(Form("%cch%d;Time slice;ADC",hl[ch/8],ch%8));
      ADCch[ch]->SetIsRaw(true);
    }
  }
  tr.Branch("Count","COUNT",&cnt);

  ////// MAIN LOOP //////
  for(int ev=0;ev<Nev;ev++){
    Decoder(fin,ADCch,cnt,rd,offset);
    tr.Fill();

    cout << "\r" << ev << flush;
  }

  //cout << "\r" << Nev << endl;
  cout << endl;

  if(offset){
    for(int i=0;i<Nch;i++)
      delete[] offset[i];
    delete[] offset;
  }

  delete cnt;
  fin.close();

  f.Write();
  //tr.Print();

  return 0;

}

bool SetOffset(const char *filename, Double_t **offset)
{
  ifstream fin(filename);
  if(!fin)
    {
      cerr << "Cannot Open Offset File: " << filename  << endl;
      return false;
    }

  int ch[16]={0,8,2,10,4,12,6,14,1,9,3,11,5,13,7,5};

  for(int i=0;i<CELLNUM;i++)
    for(int j=0;j<16;j++)

      fin >> offset[ch[j]][i];


  fin.close();

  return true;

}

inline void Decoder(ifstream &fin, CDomino **d, COUNT *c, int rd, Double_t **offset)
{
  UInt_t buf;
  Reader(fin,2,buf); //for 2bytes skip

  Reader(fin,2,c->PPS);
  Reader(fin,4,c->ClkCnt10M);
  Reader(fin,4,c->EvtNum);
  Reader(fin,4,c->TrgCnt);
  Reader(fin,8,c->ClkCnt);

  Reader(fin,24,buf); //for 24bytes skip

  const int Nchip=8;
  int chip2ch[Nchip]={0,  8,2, 10,4, 12,6, 14}; //(i%2)*7+i
  //  chip2ch[Nchip]={0,0+8,2,2+8,4,4+8,6,6+8}; 

  for(int i=0;i<Nchip;i++){
    Reader(fin,2,buf);
    d[chip2ch[i]  ]->SetStopCell(buf);
    d[chip2ch[i]+1]->SetStopCell(buf);
  }

  int counter=0;

  for(int i=0;i<=1;i++) // 2 ch loop in a chip e.g. HG 0 & 1.
    for(int slice=0;slice<rd;slice++)
      for(int chip=0;chip<Nchip;chip++){

	Reader(fin,2,buf);

	if(offset){
	  int cellID=(d[chip2ch[chip]+i]->GetStopCell()+slice)%CELLNUM;
	  d[chip2ch[chip]+i]->SetPoint(slice,slice,
				       (Double_t)buf
				       -offset[chip2ch[chip]+i][cellID]);
	} else
	  d[chip2ch[chip]+i]->SetPoint(slice,slice,buf);

	counter++;

      }

}

template <typename T>
inline void Reader(ifstream &fin, const int bytes, T &ubuf)
{
  ubuf=0;

  for(int i=0;i<bytes;i++){
    ubuf <<= 8;
    if(!fin.read((char*)&ubuf,1)){
      cout << "EOF" << endl;
      return;
    }
  }

}

inline int ReadDepthChecker(ifstream &fin)
{
  fin.seekg(0,ios::beg);
  UInt_t buf;
  int counter=-2;
  while(1){
    Reader(fin,2,buf);
    if(counter>0 && buf==0xaaaa) break;
    Reader(fin,30,buf);
    counter++;
  }

  fin.seekg(0,ios::beg);

  cout << "Read Depth: " << counter<< endl;
  return counter; //bytes
}

inline int EvNumChecker(ifstream &fin, int rd)
{
  int eventnum;
  double tmp;
  size_t fileSize;
  fileSize = (size_t)fin.seekg(0, std::ios::end).tellg();
  int footer=4;
  int data_per_ev =16*(2*rd +footer);
  tmp = (double)fileSize/data_per_ev;
  eventnum = (int)tmp;
  cout << "Number of Events: " << eventnum << endl;

  fin.seekg(0,ios::beg);
  return eventnum;
}
