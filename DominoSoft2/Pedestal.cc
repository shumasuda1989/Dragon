//Before executing this macro, you should load libCDomino.so in the root interpreter
#ifndef __CINT__
#include "CDomino.h"
#include "TProfile.h"
#endif

using namespace std;

void Pedestal(TString rootfile, TString newname=0);

#ifndef __CINT__
int main(int argc, char **argv)
{

  if(argc!=2 && argc!=3){
    cerr << "Usage: " << argv[0] << " rootfile [output filename(default:offset.root)]" << endl;
    return 1;
  }

  TApplication app("app", &argc, argv);

  if(app.Argc()==2)
    Pedestal(app.Argv(1));
  else
    Pedestal(app.Argv(1),app.Argv(2));

  //app.Run();

  return 0;

}
#endif

void Pedestal(TString rootfile, TString newname)
{
  TFile *f=new TFile(rootfile);
  TTree *e=(TTree*)f->Get("Events");
  //e->SetDirectory(0);
  //f->Close();

  TH1::SetDefaultSumw2();

  TString filename;
  if(newname.IsNull()) filename="offset.root";
  else if(newname.EndsWith(".root")) filename=newname;
  else cerr << "file name should be *.root" <<endl;
  TFile *of=new TFile(filename,"RECREATE");
  TProfile *p[16];
  TObjArray Plist(0);
  CDomino *c[16]={0};
  COUNT *count=0;
  for(int ch=0;ch<16;ch++){
    e->SetBranchAddress(Form("%cch%d",ch<8?'H':'L',ch%8),&c[ch]);
    p[ch]=new TProfile(Form("prof%cch%d",ch<8?'H':'L',ch%8),"prof",CELLNUM,-0.5,CELLNUM-0.5);
    Plist.Add(p[ch]);
  }
  e->SetBranchAddress("Count",&count);

  int N=e->GetEntries();
  for(int i=0;i<N;i++){
    /* for(int i=200;i<N;i++){
         if(i%10000<200) continue; */
    e->GetEntry(i);

    if(count->EvtCnt <100) continue;
 
    for(int ch=0;ch<16;ch++)
      for(int cell=12;cell<c[ch]->GetN()-3;cell++)
	p[ch]->Fill((c[ch]->GetStopCell()+cell)%CELLNUM,c[ch]->GetY()[cell]);

    if(i%1000==0) cout << "\r" << i<< ": " << c[0]->GetY()[10] <<flush;
  }
  cout<<endl;

  for(int ch=0;ch<16;ch++)
    for(int bin=1;bin<=CELLNUM;bin++)
      if(p[ch]->GetBinEntries(bin)<1)
	cerr << "Entry of ch" << ch << " bin" << bin << " is 0" << endl;

  Plist.Write();
  of->Close();
  f->Close();
}
