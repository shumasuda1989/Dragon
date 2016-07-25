TProfile *profdiff(TFile *_file0, TFile *_file1, int ch=0)
{
  static int Nprof=0;
  Nprof++;

  TH1::SetDefaultSumw2();

  TProfile *p1=(TProfile*)_file0->Get(Form("profHch%d",ch));
  TProfile *p2=(TProfile*)_file1->Get(Form("profHch%d",ch));
  TProfile *p=new TProfile(Form("p%d",Nprof),"diff",4096,0,4096);
  p->Add(p1,p2,1,-1);
  p->SetMarkerColor(kRed);
  //p->Draw();

  return p;

}
