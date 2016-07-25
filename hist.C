#include <TH1F.h>
#include <TProfile.h>
#include <TF1.h>

TH1F *hist(TProfile *p, bool smooth=false)
{

  static int Nhist=0;
  Nhist++;

  TH1F *h=new TH1F(Form("h%d",Nhist),"",1000,-100,100);
  TAxis *hx=h->GetXaxis();

  TF1 *f=new TF1("f","gausn",-100,100);

  for(int i=1;i<=p->GetXaxis()->GetNbins();i++){
    if(smooth){

      f->SetParameters(1,p->GetBinContent(i),p->GetBinError(i));

      for(int j=1;j<=hx->GetNbins();j++){

	double LowE=hx->GetBinLowEdge(j);
	double dInt=f->Integral(LowE,LowE+hx->GetBinWidth(j));

	h->AddBinContent(j,dInt);

      }


    } else
      h->Fill( p->GetBinContent(i) );

    cout << '\r' << i << flush;

  }

  cout << endl;

  delete f;

  return h;

}
