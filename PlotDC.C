//load libCDomino.so before excuting this macro.

void PlotDC(TString dir){

  gROOT->SetStyle("Plain");

  TFile *of=new TFile("offset.root");
  TProfile *p[16];
  for(int ch=0;ch<1;ch++){
    p[ch]=(TProfile*)of->Get(Form("prof%cch%d",ch<8?'H':'L',ch%8));
    p[ch]->SetDirectory(0);
  }
  of->Close();

  int targetSC=1038;
  int pos_f=470;

  TGraphErrors *g=new TGraphErrors();
  g->SetName("DCcal");
  g->SetTitle(Form("HG ch0 ID %d-%d;DC input [V];ADC",targetSC+pos_f,targetSC+pos_f+99));
  TCanvas* c1 = new TCanvas("c1","c1");
  c1->Draw();
  c1->SetGrid();

  int NDac=0;
  for(int dcDAC=-15;dcDAC<=115;dcDAC+=5){
    TFile *f=new TFile(dir+Form("/com08_dc%03d_.root",dcDAC));

    if(!f->IsOpen()){
      cerr << "cannot open" << endl;
      return;
    }

    TTree *tev=(TTree*)f->Get("Events");

    CDomino *d=0;

    tev->SetBranchAddress("Hch0",&d);

    int Nev=tev->GetEntries();
    cout << "# of events: " << Nev << endl;

    TProfile *pdc=new TProfile("pdc",Form("HG ch0 ID %d-%d",targetSC+pos_f,targetSC+pos_f+99),100,targetSC+pos_f-0.5,targetSC+pos_f+99.5);
    pdc->SetDirectory(0);

    int event=600;
    int num=0;
    while(1){

      tev->GetEntry(event);

      event++;
      if( d->GetStopCell()<targetSC-5 || d->GetStopCell()>targetSC+5) continue;

      int delta=d->GetStopCell()-targetSC;

      for(int cell=pos_f-delta;cell<pos_f-delta+100;cell++){
	if(d->IsRaw())
	  pdc->Fill(d->GetStopCell()+cell,d->GetY()[cell]-p[0]->GetBinContent((d->GetStopCell()+cell)%4096+1));
	else
	  pdc->Fill(d->GetStopCell()+cell,d->GetY()[cell]);

	if(d->GetY()[cell]>=4094.5) ;
      }

      num++;
      if(num==100) break;

    }

    if(num<100)
      cerr<< "# of events of " << NDac << "< 100" <<endl;

    //pdc->Draw();
    f->Close();
    for(int i=0;i<100;i++){
      g->SetPoint(100*NDac+i,dcDAC/100.,pdc->GetBinContent(i+1));
      g->SetPointError(100*NDac+i,0,pdc->GetBinError(i+1));
    }
    NDac++;
  }

  g->Draw("ap");
  TF1 *func=new TF1("func","pol1",0.04,0.91);
  func->SetLineColor(kRed);
  g->Fit(func,"R");

  TCanvas *c2=new TCanvas("c2","c2",700,300);
  c2->Draw();
  c2->SetGrid();
  TGraphErrors *gdiff=new TGraphErrors(g->GetN());
  gdiff->SetName("gdiff");
  gdiff->SetTitle("residual;;residual");
  for(int i=0;i< g->GetN();i++){
    gdiff->SetPoint(i,g->GetX()[i],
		    (g->GetY()[i]-(*func)(g->GetX()[i]))/TMath::Abs((*func)(g->GetX()[i])));
    gdiff->SetPointError(i,g->GetEX()[i],g->GetEY()[i]/TMath::Abs((*func)(g->GetX()[i])));
  }

  gdiff->Draw("ap");

}
