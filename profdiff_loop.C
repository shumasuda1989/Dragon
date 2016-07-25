#include "profdiff.C"

void profdiff_loop() 
{

  TCanvas *c1=new TCanvas("c1","c1",700*3/2,500*3/2);
  c1->Divide(5,5);

  TFile *f1=new TFile("offset_00001.root");

  int pospad=1;
  for( int i=2;i<221;i+=10){

    TFile *f2=new TFile(Form("offset_%05d.root",i));
    TProfile *p=profdiff(f2,f1);
    p->SetDirectory(0);
    p->SetMarkerStyle(7);
    p->GetYaxis()->SetRangeUser(-15,15);
    c1->cd(pospad);
    p->Draw();

    f2->Close();

    pospad++;

  }

  f1->Close();

}
