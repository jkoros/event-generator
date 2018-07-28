/*
  Linear interpolation of double differential cross sections
  between existing E_v histos.
  usage: ./interpolate EnuMin EnuMax EnuSpacing NumInterps
  
  eg: ./interpolate 10 30 5 4
  would interpolate between 10 and 30 MeV, with existing histos
  every 5 MeV and with 4 new interpolated histos in every 5 MeV gap
  ie you end up with a histo every 1 MeV
  
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

int main(int argc, char* argv[])
{
  // set variables from input
  float EnuMin, EnuMax;  // E_v max and min 
  int NumInterps;        // number of interpolations per gap
  float EnuSpacing;      // regular E_v spacing between existing histos
  if (argc==5)
  {
    sscanf(argv[1], "%f", &EnuMin);
    sscanf(argv[2], "%f", &EnuMax);
    sscanf(argv[3], "%f", &EnuSpacing);
    sscanf(argv[4], "%i", &NumInterps);
  }
  else
  {
    std::cout << "Invalid arguments!" << std::endl;
    return 1;
  }
  if (EnuMin > EnuMax) 
  {
    std::cout << "Invalid Energies!" << std::endl;
    return 1;
  }

  // loop over neutrino energies from EnuMin to EnuMax
  int numFiles = round((EnuMax - EnuMin) / EnuSpacing);  
  for (int c=0; c<numFiles; c++)
  {
    float n = EnuMin + c*EnuSpacing;
    std::cout << n << std::endl;
    
    // open masterfile
    TFile * masterfile = new TFile("./outfiles/diffxsections.root");
    
    char hist1[15];
    std::sprintf(hist1, "%s%.1f", "v", n);
    std::string hist1String = hist1;
    int pos = hist1String.find(".");
    std::string hist1Name = hist1String.replace(pos, 1, "_");
    TH2D * hist1point = (TH2D*)gDirectory->Get(hist1Name.c_str());  // pointer to first histo

    char hist2[15];
    std::sprintf(hist2, "%s%.1f", "v", n+EnuSpacing);
    std::string hist2String = hist2;
    pos = hist2String.find(".");
    std::string hist2Name = hist2String.replace(pos, 1, "_");
    TH2D * hist2point = (TH2D*)gDirectory->Get(hist2Name.c_str());  // pointer to second histo

    /*
    // create outfile for interpolated histos
    char out[30];
    std::sprintf(out, "%s%.1f%s%.1f%s", "v", n, "-", n+EnuSpacing, ".root");
    std::string outString = out;
    pos = outString.find(".");
    std::string outNameintm = outString.replace(pos, 1, "_");
    pos = outNameintm.find(".");
    std::string outNameString = outNameintm.replace(pos, 1, "_");
    std::string outName = "./outfiles/" + outNameString;
    TFile * outfile = new TFile(outName.c_str(), "RECREATE");
    outfile->Close();
    */

    // loop over EnuSpacing, creating NumInterps per gap
    for (int i=1; i<NumInterps+1; i++)
    {
      // neutrino energy for this histo
      float E_v = n + i*(EnuSpacing/(NumInterps+1));

      // histo name
      char hist[15];
      std::sprintf(hist, "%s%.1f", "v", E_v);
      std::string histString = hist;
      pos = histString.find(".");
      std::string histName = histString.replace(pos, 1, "_");

      // create histo
      int nbinsx = hist1point->GetNbinsX();
      double xmax = hist1point->GetBinLowEdge(nbinsx) + hist1point->GetBinWidth(nbinsx);
      double xmin = hist1point->GetBinLowEdge(1);
      int nbinsy = hist1point->GetNbinsY();
      int ymax = hist1point->GetYaxis()->GetBinLowEdge(nbinsy) + hist1point->GetBinWidth(nbinsy);
      int ymin = hist1point->GetYaxis()->GetBinLowEdge(1);
      char title[50];
      std::sprintf(title, "%s%.1f%s", "Interpolated Cross Section: E_{v} = ", E_v, " MeV");
      TH2D *interpHist = new TH2D(histName.c_str(), title, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
      interpHist->GetXaxis()->SetTitle("E_{e} (MeV)");
      interpHist->GetYaxis()->SetTitle("cos(#theta)");

      // loop over all histo bins
      float binsize = hist1point->GetBinWidth(1);
      for (int ybin=1; ybin<=nbinsy; ybin++)
      {
	for (int xbin=1; xbin<=nbinsx; xbin++)
	{
	  int ibin = interpHist->GetBin(xbin,ybin);

	  // find xbin and ibin for lower E_v histo
	  int hist1xbin = xbin - (i*EnuSpacing/(NumInterps+1))/binsize;
	  int hist1ibin = hist1point->GetBin(hist1xbin,ybin);
	  
	  // find xbin and ibin for higher E_v histo
	  int hist2xbin = xbin + (EnuSpacing/binsize) - (EnuSpacing*i/(NumInterps+1))/binsize;
	  int hist2ibin = hist2point->GetBin(hist2xbin,ybin);

	  // linear interpolation
	  Double_t binval1 = hist1point->GetBinContent(hist1ibin);  // get binval from first histo
	  Double_t binval2 = hist2point->GetBinContent(hist2ibin);  // get binval from second histo
	  Double_t binvali = binval1 + i * (binval2-binval1) / (NumInterps+1); // calculate weight
	  if (binvali<0) std::cout << "negative bin value!" << std::endl; // check for invalid binval

	  // fill interpolated histo
	  interpHist->SetBinContent(xbin, ybin, binvali);
	}
      }

      /*
      // write interpolated histo to outfile
      TFile *outfile = new TFile(outName.c_str(), "UPDATE");      
      interpHist->Write();
      outfile->Close();
      */

      // write interpoalted histo to masterfile
      TFile *masterfile = new TFile("./outfiles/diffxsections.root", "UPDATE");
      interpHist->Write();
      masterfile->Close();
    }  
 
  }

  return 0;
}
