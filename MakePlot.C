void MakePlot(TFile *f, TString outfilename){

  TTree *tev=(TTree*)f->Get("Events");

  CDomino *d[16]={0};
  for(int i=0;i<16;i++)
    tev->SetBranchAddress(Form("%cch%d",i<8?'H':'L',i%8),&d[i]);

  COUNT *c=0;
  tev->SetBranchAddress("Count",&c);

  int Nev=tev->GetEntries();
  cout << "# events: " << Nev << endl;

  TFile out(outfilename,"RECREATE");
  TGraph *g[8];
  for(int i=0;i<8;i++){
    g[i]=new TGraph(Nev);
    g[i]->SetName(Form("stopcell%d",i));
    g[i]->SetTitle(Form("StopCell chip %d (%cG ch %d,%d);Event Num;StopCell",
			i+1,i%2==0?'H':'L',i/2*2,i/2*2+1));
  }

  TH1F *h[16];
  for(int i=0;i<16;i++)
    h[i]=new TH1F(Form("noiselevel_ch%d",i),
		  Form("Noise Level %cG ch %d;ADC;",i<8?'H':'L',i%8),
		  100,-50.,50.);
  int ch2chip[8]={0,2,4,6,1,3,5,7};

  for(int i=0;i<Nev;i++){

    tev->GetEntry(i);
    if(c->EvtNum <10) continue;

    for(int ch=0;ch<16;ch++){
      if(ch%2==0)
	g[ch2chip[ch/2]]->SetPoint(i,i+1,d[ch]->GetStopCell());
      for(int cell=3;cell<d[ch]->GetN()-3;cell++)
	if(d[ch]->GetY()[cell]>-50. && d[ch]->GetY()[cell]<50.)
	  h[ch]->Fill(d[ch]->GetY()[cell]);
    }
    cout << "\r" << i << flush;

  }
  cout<<endl;

  for(int i=0;i<8;i++)
    g[i]->Write();

  out.Write();
  out.Close();

}
