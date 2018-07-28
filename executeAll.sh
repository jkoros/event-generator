#! /bin/csh

# make directories
mkdir infiles outfiles

# scrape files
echo "\nScraping..."
./diffscraper.py

# compile necessary files
echo "\nCompiling...\n"
make all -s

# run diff2poly for each file in infiles
foreach filename (./infiles/*.txt)
    echo "Reading $filename:t"
    ./diff2poly $filename:t
end

# interpolate to get histo every .1 MeV
echo "\nInterpolating..."
./interpolate 1.6 10 0.2 1
./interpolate 10 20 0.5 4
./interpolate 20 170 10 99

# plot flux and create flux weighted distribution
echo "\nPlotting flux weighted distribution..."
./fluxWeight

# create sample of random events
echo "\nSampling events..."
./rndmSample

echo "\nAll done!\n"
