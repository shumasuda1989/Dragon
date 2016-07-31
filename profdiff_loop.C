#include "profdiff.C"

void profdiff_loop(TString dir=".")
{

  int Nfile=(gSystem->GetFromPipe(Form("ls -1 %s/offset_?????.root | wc -l",dir.Data()))).Atoi();
  const int Ncan =(Nfile+24)/25;
  int Npad=5;
  if(Ncan==1) Npad=sqrt(Nfile-2)+1;

  TCanvas *c1[100];
  for(int ican=0;ican<Ncan;ican++){
    c1[ican]=new TCanvas(Form("c%d",ican+1),Form("c%d",ican+1),700*3/2,500*3/2);
    c1[ican]->Divide(Npad,Npad);
  }

  TFile *f1=new TFile(Form("%s/offset_00001.root",dir.Data()));

  int curcan=0;
  int pospad=1;
  for( int i=2;i<=Nfile;i++){

    TFile *f2=new TFile(Form("%s/offset_%05d.root",dir.Data(),i));
    TProfile *p=profdiff(f2,f1);
    p->SetDirectory(0);
    p->SetMarkerStyle(7);
    p->GetYaxis()->SetRangeUser(-15,15);
    c1[curcan]->cd(pospad);
    p->Draw();

    delete f2;

    pospad++;
    if(pospad==26){
      pospad=1;
      curcan++;
    }


  }

  f1->Close();

}
