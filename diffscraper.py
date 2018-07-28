#! /usr/bin/env python

"""
web scraper to get double differential cross section
information for v_e + d -> e^- + p + p
from Gudkov tables at:
http://boson.physics.sc.edu/~gudkov/NU-D-NSGK/Netal/index.html

scraped file format:
E_v
theta
E_e(exp) p_e(exp) diffxsection(exp)
E_e(exp_ p_e(exp) diffxsection(exp)
...
...
theta
E_e(exp) p_e(exp) diffxsection(exp)
E_e(exp_ p_e(exp) diffxsection(exp)
...
...
...

Jes Koros, June 2018
"""

# import libraries
from __future__ import division, print_function
import urllib2
from bs4 import BeautifulSoup
import numpy as np
from concurrent.futures import TimeoutError
import os.path

# get html from page url and parse
quote_page = "http://boson.physics.sc.edu/~gudkov/NU-D-NSGK/Netal/electron/e-2nd-table/index.html"
page = urllib2.urlopen(quote_page,None,None)
soup = BeautifulSoup(page, "html.parser")

# list neutrino energies
vEnergies = soup("a")
i = -1    # initialize counter

# array of angles
angles = np.arange(0,185,5)

# loop over neutrino energies
for link in soup("a"):
    i += 1   # increment counter
    n = -1   # reset angle counter

    # create output file
    E_v = vEnergies[i].get_text() # get neutrino energy
    filename = "v" + E_v
    if "." in filename:
        filename = filename.replace(".","_",1)
    else:
        filename = filename + "_0"
    filename = "./infiles/" + filename + ".txt"

    # check if file exists
    if os.path.isfile(filename):
        continue # don't rewrite if file exists
    else:
        outfile = open(filename, "w")
        
    # write E_v to file
    outfile.write(E_v + "\n")
    print("Now reading E_v: ", E_v)
    
    # get next page url
    linkExtend2 = link.get("href")
    link2 = quote_page.replace("index.html",linkExtend2)

    # get new html and parse
    # now on the page of angles
    retry=0
    while retry<100:
        retry+=1
        try:
            page2 = urllib2.urlopen(link2,None,None)
            soup2 = BeautifulSoup(page2, "html.parser")
            break
        except urllib2.URLError:
            print("Retrying...")
            continue

    # read in angle and follow next link
    for link in soup2("a"):
        # get angle
        n += 1
        theta = angles[n]
        # write angle to file
        outfile.write(str(theta) + "\n")
   
        # get html
        linkExtend3 = link.get("href")
        link3 = link2 + "/" + linkExtend3
        retry = 0
        while retry<100:
            retry+=1
            try:
                page3 = urllib2.urlopen(link3,None,None)
                soup3 = BeautifulSoup(page3, "html.parser")
                break
            except urllib2.URLError:
                print("Retrying...")
                continue

        # print out data to file, skipping header row
        data = soup3("tr")
        z = 0
        try:
            for x in data:
                z += 1
                outfile.write(data[z].get_text() + "\n")
        except IndexError:
            # continue when all entries read
            continue
            
# close outputfile
outfile.close()
