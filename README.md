# event-generator
Event generator for nu_e + d

# Getting Started
Versions used for web scraper: 

Python 2.7

Beautiful Soup 4 (install instructions and documentation: https://www.crummy.com/software/BeautifulSoup/bs4/doc/#installing-beautiful-soup)

Versions used for everything else:

C++ 11

GCC 4.4

ROOT 5

# Usage
executeAll.sh will run through all steps necessary to produce the final flux-weighted histogram for event generation. General outline of this process: diffscraper.py scrapes necessary input files from Gudkov tables (http://boson.physics.sc.edu/~gudkov/NU-D-NSGK/Netal/index.html); diff2poly.C uses these to fill TH2D histos with even bins and constant range; interpolate.C linearly interpolates between existing histos to give plots for every 0.1 MeV over full Enu range; fluxWeight.C plots SNS flux and uses this to weight over all Enu into a single flux-weighted histo, also integrates to give total flux-weighted cross section; rndmEvents.C generates specified number of events randomly sampled from this histo.

Other files: fluxWeight.root contains the final flux-weighted TH2D histo. The .root file containing all histos generated from this code is available on dropbox (https://www.dropbox.com/s/wgqn3319oceqxcz/diffxsections.root?dl=0). I have also included the makefile I adapted and the config file available on my machine, but unfortunately I don't currently have the experience to create more generalized versions, so hopefully these will be useful enough for now.

# More Information
I wrote this code as part of a project for the 2018 Duke Phyiscs REU Program. The details of my project, along with some of these plots, can be found in Koros-report.pdf.
