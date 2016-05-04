void MakePlot2(TString file){

  TFile *f=new TFile(file);

  new TCanvas("c1","c1",700*4,500*2);
  c1->Divide(4,2);

  Double_t meanRMS=0.;

  for(int i=0;i<7;i++){
    c1->cd(i+1);
    TH1F *h=(TH1F*)f->Get(Form("noiselevel_ch%d",i));
    h->SetDirectory(0);
    h->Draw();
    meanRMS+=h->GetRMS();
    cout << h->GetRMS() << endl;
  }
  cout << "mean RMS: " << meanRMS/7. << endl;

  f->Close();

}
