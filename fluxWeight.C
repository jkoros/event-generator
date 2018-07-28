/* 
   produces single flux weighted double differential cross section
   distribution from noramlized SNS flux and diff xsection histos.
   calculates total flux weighted cross section.
   Jes Koros, July 2018
*/

// C++ libraries
#include <iostream>
#include <string>
#include <cmath>
#include <cstring>
#include <fstream>
#include <cmath>

// ROOT libraries
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TString.h"
#include "TGraph.h"

// declare function to plot SNS flux
void SNSflux();

int main()
{
  // call function to plot SNS flux
  SNSflux();

  // open master infile with all xsection and flux pdfs
  TFile * masterfile = new TFile("./outfiles/diffxsections.root");

  // find max E_v from flux plt (assumes last bin in flux plt gives max Enu)
  TH1D *fluxpoint = (TH1D*)gDirectory->Get("SNSflux");
  int nbins = fluxpoint->GetNbinsX();
  double Enumax = fluxpoint->GetBinLowEdge(nbins) + fluxpoint->GetBinWidth(nbins);

  // create new flux weighted hist
  TH2D *histpoint = (TH2D*)gDirectory->Get("v1_5"); // pointer to any diffxsection histo
  int ybins = histpoint->GetNbinsY();
  double ymax = histpoint->GetYaxis()->GetBinLowEdge(ybins) + histpoint->GetYaxis()->GetBinWidth(ybins);
  double ymin = histpoint->GetYaxis()->GetBinLowEdge(1);
  // max Ee defined from Enumax by accounting for deuteron binding energy
  double E_emax = Enumax-1.44;
  int xbins = histpoint->GetXaxis()->FindBin(E_emax);
  double xmin = histpoint->GetBinLowEdge(1);
  double xmax = histpoint->GetBinLowEdge(xbins) + histpoint->GetBinWidth(xbins);

  TH2D * fluxW = new TH2D("fluxW", "SNS Flux Weighted Double Differential Cross Sections", xbins, xmin, xmax, ybins, ymin, ymax);
  fluxW->GetXaxis()->SetTitle("E_{e} (MeV)");
  fluxW->GetYaxis()->SetTitle("cos(#theta)");  

  // loop over all bins
  for (int i=1; i<=xbins; i++)
  {
    for (int n=1; n<=ybins; n++)
    {
      // loop over all energies up to max
      double E_v = 1.5;
      double E_v_last = 0;
      int ibin = histpoint->GetBin(i,n);
      double E_e = fluxW->GetXaxis()->GetBinCenter(i);
      double cos_theta = fluxW->GetYaxis()->GetBinCenter(n);
      double Wbinval = 0;    

      while (E_v < Enumax)
      {
	// pointer to this histo
	char hist[15];
	std::sprintf(hist, "%s%.1f", "v", E_v);
	std::string histString = hist;
	int pos = histString.find(".");
	std::string histName = histString.replace(pos, 1, "_");
	TH2D * histpoint = (TH2D*)gDirectory->Get(histName.c_str());

	// grab binval and flux
	double binval = histpoint->GetBinContent(ibin);
	int fluxbin = fluxpoint->FindBin(E_v);
	double flux = fluxpoint->GetBinContent(fluxbin);

	// calculate bin size
	double binsize = E_v - E_v_last;

	// calculate new flux weighted binval
	Wbinval += binval * flux * binsize / Enumax;

	// set next E_v
	E_v_last = E_v;
	E_v += 0.1;
      }

      // fill bin in flux averaged histo
      fluxW->Fill(E_e,cos_theta,Wbinval);
    }
  
  }

  // print total flux weighted cross section
  std::cout << "Total flux weighted cross section (scaled by 10^-50): " << fluxW->Integral("width") << std::endl;

  // save histo and close files
  TFile * masterwrite = new TFile("./outfiles/diffxsections.root", "UPDATE");
  fluxW->Write();
  TFile * outfile = new TFile("./outfiles/fluxWeight.root", "RECREATE");
  fluxW->Write();
  outfile->Close();
  masterwrite->Close();
  masterfile->Close();

  return 0;
}

void SNSflux()
{
  /*
    function to plot normalized SNS flux for electron neutrinos
    based on code provided by Kate Scholberg
  */  

  // create outputfile
  // TFile * file = new TFile("./outfiles/SNSflux.root", "RECREATE");

  // declare variables
  double flux;
  const double mmu = 105.66837;
  const double a = 2/mmu;
  double Enu;
  double ebinsize = 0.1;

  // find max Enu for non-negative flux
  for (double i=10; i<100; i+=0.1)
  {
    Enu = i;
    flux = 12 * a*Enu*a*Enu * (1-a*Enu) * a * ebinsize;
    if (flux <=0) break;
  }

  // create histo
  int nbins = Enu/.1;
  TH1D * fluxplt = new TH1D("SNSflux", "Normalized SNS Flux", nbins, 0, Enu);
  fluxplt->GetXaxis()->SetTitle("E_{v} (MeV)");
  fluxplt->GetYaxis()->SetTitle("Flux");

  // loop over bins and calculate flux for each Enu value
  for (int i=1; i<=nbins; i++)
  {
    Enu = fluxplt->GetBinCenter(i);
    flux = 12 * a*Enu*a*Enu * (1-a*Enu) * a * ebinsize;
    if (flux < 0) flux = 0;
    fluxplt->Fill(Enu,flux);
  }

  // normalize histo
  double area = fluxplt->Integral();
  if (area != 1)
  {
    double scale = 1/area;
    fluxplt->Scale(scale);
  }

  // write histo to outfile
  // fluxplt->Write();

  // write flux histo to masterfile
  TFile *masterfile = new TFile("./outfiles/diffxsections.root", "UPDATE");
  fluxplt->Write();
  masterfile->Close();
  // file->Close();
}
