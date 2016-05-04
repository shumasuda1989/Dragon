//Before executing this macro, you should load libCDomino.so in the root interpreter
#ifndef __CINT__
#include "CDomino.h"
#include "TProfile.h"
#endif

using namespace std;

void EventDisp(TString rootfile, int ev=0);

#ifndef __CINT__
int main(int argc, char **argv)
{

  if(argc!=2 && argc!=3){
    cerr << "Usage: " << argv[0] << " rootfile [# event]" << endl;
    return 1;
  }

  TApplication app("app", &argc, argv);

  if(app.Argc()==3)
    EventDisp(app.Argv(1),atoi(app.Argv(2)));
  else 
    EventDisp(app.Argv(1));


  return 0;

}
#endif


void EventDisp(TString rootfile, int ev)
{

  gROOT->SetStyle("Plain");

  TFile *of=new TFile("offset.root");
  TProfile *p[16]={0};
  bool offsetflag=false;
  if(!of->IsOpen())
    cerr << "cannot open offset.root" << endl;
  else {
    for(int ch=0;ch<1;ch++)
      p[ch]=(TProfile*)of->Get(Form("prof%cch%d",ch<8?'H':'L',ch%8));
    offsetflag=true;
  }

  TFile *f=new TFile(rootfile);
  if(!f->IsOpen()){
    cerr << "cannot open" << endl;
    return;
  }

  TTree *tev=(TTree*)f->Get("Events");

  CDomino *d=0;
  COUNT *c=0;

  tev->SetBranchAddress("Hch0",&d);
  tev->SetBranchAddress("Count",&c);

  //cout << (CDomino*)d << ", " << (COUNT*)c << endl;

  int Nev=tev->GetEntries();
  cout << "# events: " << Nev << endl;

  TCanvas* c1 = new TCanvas();
  c1->Draw();
  c1->SetGrid();

  int event=ev;
  while(1){

    tev->GetEntry(event);

    cout << "evt:" << c->EvtNum << ", trg:" << c->TrgCnt << ", clk:" << c->ClkCnt << endl;
    cout << "stopcellID " << flush;
    cout << d->GetStopCell() << endl;
    if(d->IsRaw() && offsetflag)
      for(int cell=0;cell<d->GetN();cell++)
	d->GetY()[cell]-=p[0]->GetBinContent((d->GetStopCell()+cell)%CELLNUM+1);
    d->Draw("apl");
    /*
    d->GetXaxis()->SetTitle("Time slice");

    if(d->IsRaw())
      d->GetYaxis()->SetTitle("ADC");
    else
      d->GetYaxis()->SetTitle("ADC (Pedestal Subtracted)");
    */

    c1->Modified();
    c1->Update();

    if(!EvLoopHandler(event)) break;

  }

  f->Close();
  of->Close();
}
