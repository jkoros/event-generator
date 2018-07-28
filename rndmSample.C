/* 
   randomly sample from flux weighted distribution 
   and print list of events to output file
   
   outfile format:
   number incident neutrinos
   E_e \t cos_theta
   E_e \t cos_theta
   ...

   usage: ./rndmSample numEvents
   default numEvents = 100

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
  // set number of events to sample
  int numEvents;
  if (argc==2) sscanf(argv[1], "%i", &numEvents);
  else if (argc==1) numEvents = 100;
  else
  {
    std::cout << "Invalid input!" << std::endl;
    return 1;
  }

  // open masterfile
  TFile * masterfile = new TFile("./outfiles/diffxsections.root");

  // pointer to histo
  TH2D * fluxW = (TH2D*)gDirectory->Get("fluxW");

  // create output file
  FILE * eventFile = fopen("./outfiles/rndmEvents.txt", "w");

  // print header
  fprintf(eventFile, "%s", "E_e\t\tcos_theta\n");

  // print out numEvents to events file
  double randx, randy;
  for (int i=0; i<numEvents; i++)
  {
    fluxW->GetRandom2(randx,randy);
    fprintf(eventFile, "%f\t%f\n", randx, randy);
  }

  // close files
  fclose(eventFile);
  masterfile->Close();

  return 0;
}
