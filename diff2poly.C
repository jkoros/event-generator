/* 
   Read data from text file to fill 2D histos. First puts data in TTree, then creates
   and fills TH2Poly histos from tree. TH2Poly designed to fill exactly one entry in 
   each bin. Then fills TH2D histo with regular binning from TH2Poly. For TH2D bins 
   smaller than TH2Poly bin, linearly interpolates between TH2Poly bins to fill 
   TH2D bins. Saves TH2D to masterfile. Option to save TTree, TH2Poly, & TH2D separately.

   create 2D histograms for cos_theta vs E_e for double differential cross section 
   of v_e + d -> e^- + p + p
   
   cross sections given as d^2sigma / dp_e domega_e (cm^2/srMeV) and converted to
   d^2sigma / de_e dcos_theta (cm^2/MeV)
   
   Usage: ./diff2poly [inputfile]

   Input file structure:
   E_v
   theta
   E_e(exp) p_e(exp) diffxsection(exp)
   E_e(exp) p_e(exp) diffxsection(exp)
   ...
   ...
   theta
   E_e(exp) p_e(exp) diffxsection(exp)
   E_e(exp) p_e(exp) diffxsection(exp)
   ...
   ...
   ...
   
   Jes Koros, June 2018
*/
   
// C++ libraries
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <stdlib.h>
#include <sstream>
#include <map>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <algorithm>

// ROOT libraries
#include "TROOT.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TString.h"
#include "TH2Poly.h"
#include "TMath.h"
#include "TRandom.h"

int main(int args, char * infile[] )
{
  // variable definitions
  Float_t         E_v;               // neutrino energy (MeV)
  Int_t           theta;             // theta (deg)
  Float_t         cos_theta;         // cos(theta)
  Float_t         E_e;               // electron energy (MeV)
  Float_t         p_e;               // electron momentum (MeV)
  Float_t         diff2;             // d^2sigma/dp_edomega_e (cm^2/srMeV)
  char            ignore;            // ignore parentheses when reading in from file
  int             exp1, exp2, exp3;  // variables to read in exponents

  // create output file
  std::string inString = infile[1];
  int pos = inString.find(".");
  
  /*
  std::string outfileName = inString.replace(pos, 10, ".root");
  std::string outName = "./outfiles/" + outfileName;
  TFile * outfile = new TFile(outName.c_str(),"RECREATE");
  */

  // create tree
  TTree *diff2tree = new TTree("diff2tree","double differential cross section information");
  // tree branches
  diff2tree->Branch("E_v",&E_v,"E_v/F");
  diff2tree->Branch("theta",&theta,"theta/I");
  diff2tree->Branch("cos_theta",&cos_theta,"cos_theta/F");
  diff2tree->Branch("E_e",&E_e,"E_e/F");
  diff2tree->Branch("p_e",&p_e,"p_e/F");
  diff2tree->Branch("diff2",&diff2,"diff2/F");

  // fill tree from infile
  // open infile
  inString = infile[1];
  std::string inName = "./infiles/" + inString;
  char *infileName = new char[inName.size()+1];
  infileName[inName.size()] = 0;
  memcpy(infileName,inName.c_str(),inName.size());
  FILE * inPoint = fopen(infileName,"r");

  char line[100];                // variable to store line
  fgets(line,100,inPoint);       // get first line
  sscanf(&line[0], "%f", &E_v);  // read in E_v from line
  
  float E_hi=0;   // to find max E_e for TH2Poly
  char *p;        // pointer to fine newline position in char array
  int nline=0;    // line counter

  // read in rest of file
  while(fgets(line,100,inPoint))
  {
    nline ++;
    p = std::find(line, line+100, '\n'); // find position of newline

    if (p<line+10) // condition for theta lines
    {
      sscanf(&line[0], "%i", &theta);             // read in theta
      cos_theta = cos(theta * 3.14159265 / 180);  // calculate cos
    }
    else 
    {
      sscanf(&line[0], "%f %c %i %c %f %c %i %c %f %c %i %c", &E_e, &ignore, &exp1, &ignore, &p_e, &ignore, &exp2, &ignore, &diff2, &ignore, &exp3, &ignore); // read in data
      // apply exponents
      E_e = E_e * pow(10,exp1);         // total E_e
      p_e = p_e * pow(10,exp2);
      exp3 = exp3 + 50;                 // scale cross section by 10^-50
      diff2 = diff2 * pow(10,exp3);
      // convert cross section to d^2sigma / dE_e dcos_theta
      if (diff2 != 0) diff2 = diff2 * 2 * M_PI * E_e / p_e;
      E_e = E_e - 0.511;                // subtract rest mass to get kinetic energy
      if (E_e < 0) E_e = 0;             // first E_e falls just below 0
      diff2tree->Fill();                // fill tree
      if (E_e > E_hi)
      {
	// find max E_e value for constructing TH2Poly
	E_hi = E_e + 0.001;
      }
    } 
  }

  // close input file
  fclose(inPoint);

  // diff2tree->Print(); // print out tree information
  // diff2tree->Write(); // write tree to outfile

  // fill histo from ttree

  // get bin widths for variable bin sizes from tree entries
  Float_t E_low = 0; // const xlow value

  // create TH2Poly histo
  char title[50];
  std::sprintf(title, "%s %.1f %s", "Double Differential Cross Section: E_{v} =", E_v, "MeV");
  TH2Poly * diff2Poly = new TH2Poly("diff2Poly", title, 10, E_low, E_hi, 10, -1.01, 1.01);
  diff2Poly->GetXaxis()->SetTitle("E_{e} (MeV)");
  diff2Poly->GetYaxis()->SetTitle("cos(#theta)");

  // essentially will have different x-axis for each y bin
  int NBinsY = 37;                     // constant number of ybins
  int NBins = diff2tree->GetEntries(); // total number of bins equal to number of data points
  Double_t edgesX[NBins + NBinsY];     // array of X edges for bins
  
  // construct array of edges for x bins
  // works for variable number of x bins per y value
  edgesX[0] = E_low;
  edgesX[NBins+NBinsY-1] = E_hi;
  int entryNum = 1;
  for (int i=1; i<(NBins+NBinsY); i++)
  {
    diff2tree->GetEntry(entryNum);
    float e1 = E_e;
    if (entryNum == NBins) break;
    if (e1 < edgesX[i-1])
    {
      edgesX[i] = E_hi;
      i++;
      edgesX[i] = E_low;
      entryNum++;
    }
    else
    {
      diff2tree->GetEntry(entryNum-1);
      edgesX[i] = E_e + 0.5*(e1 - E_e);
      entryNum++;
    }
  }

  // construct Yaxis
  float thetas[NBinsY];
  for (int i=0; i<NBinsY; i++)
  {
    thetas[i] = cos((5*i) * 3.14159265 / 180);
  }

  Double_t edgesY[NBinsY+1];
  edgesY[0] = 1.01;
  edgesY[NBinsY] = -1.01;
  for (int i=1; i<NBinsY; i++)
  {
    edgesY[i] = thetas[i-1] + 0.5 * (thetas[i] - thetas[i-1]);
  }
  
  // make bins
  int n = 0;
  for (int i=0; i<NBins+NBinsY; i++)
  {
    diff2Poly->AddBin(edgesX[i], edgesY[n], edgesX[i+1], edgesY[n+1]);
    if (edgesX[i+2]<edgesX[i+1])
    {
      n++;
      i++;
    }
  }

  // loop over tree entries and fill histo
  int entries = diff2tree->GetEntries();
  Double_t integral[entries];
  for (int i=0; i<entries; i++)
  {
    diff2tree->GetEntry(i);
    diff2Poly->Fill(E_e, cos_theta, diff2); // fill histo
    // diff2Poly->Fill(E_e,cos_theta);       // check if bins are double filling
  }

  // write histo to file
  // diff2Poly->Write();

  // fill TH2D with smaller bins from TH2Poly
  int nbinsx = 1695; 
  int nbinsy = 100;
  double xmax = 170;
  double xmin = 0.5;
  double ymax = 1.01;
  double ymin = -1.01;

  // create TH2D histo
  std::string histName = inString.erase(pos, 20);
  TH2D *diff2D = new TH2D(histName.c_str(), title, nbinsx, xmin, xmax, nbinsy, ymin, ymax);
  diff2D->GetXaxis()->SetTitle("E_{e} (MeV)");
  diff2D->GetYaxis()->SetTitle("cos(#theta)");

  // fill TH2D histo from TH2Poly
  Double_t x,y,w,w_last,w_interp;
  Int_t ibin, ibin_last=0;
  int NmultiBins=0;

  for (int n=1; n<=nbinsy; n++)
  {
    y = diff2D->GetYaxis()->GetBinCenter(n);   // get y coordinate
    ibin_last = 0;
    for (int i=1; i<=nbinsx; i++)
    {
      x = diff2D->GetXaxis()->GetBinCenter(i); // get x coordinate
      ibin = diff2Poly->FindBin(x,y);          // get bin number for (x,y)
      w = diff2Poly->GetBinContent(ibin);
      
      // fill 0 if overflow or underflow bin
      if (ibin < 0)
      {
	w = 0;
	diff2D->Fill(x,y,w);
      }
      
      // don't interpolate if the bin value is zero because it's physically zero
      else if (w==0) diff2D->Fill(x,y,w);
      
      // interpolate if multiple TH2D bins fall in same TH2Poly bin
      else if (ibin==ibin_last)
      {
	// check next bin until ibin is different
	while(ibin == ibin_last)
	{
	  NmultiBins++;
	  x = diff2D->GetXaxis()->GetBinCenter(i+NmultiBins);
	  ibin = diff2Poly->FindBin(x,y);
	}

	// fill last TH2D bin in ibin with w
	w = diff2Poly->GetBinContent(ibin_last);
	diff2D->SetBinContent(i+NmultiBins-1,n,w);

	// interpolate for empty bins in between two filled bins
	w_last = diff2Poly->GetBinContent(ibin_last-1);
        for (int m=1; m<=NmultiBins; m++)
	{
	  w_interp = w_last + m *  (w - w_last) / (NmultiBins+1);
	  diff2D->SetBinContent(i-2+m,n,w_interp);
	}

	// set i to next unfilled bin
	i += NmultiBins-1;
	NmultiBins = 0;
      }
      
      // otherwise fill bin as usual
      else
      {
	diff2D->Fill(x,y,w);
	ibin_last = ibin;
      }
    }
  }

  // diff2D->Write();   // write TH2D to outfile  

  // write TH2D to masterfile
  TFile *masterfile = new TFile("./outfiles/diffxsections.root", "UPDATE");
  diff2D->Write();
  masterfile->Close();
	       
  // outfile->Close();  // close outfile

  return 0;
}
