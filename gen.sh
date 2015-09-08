#!/bin/bash
echo "$(date)"
echo "Processing 01..."
c++ gen_stats1.cpp
sh ./gen1.sh > 01-smp-r-rural.out
sh ./gen2.sh > 01-smp-r-urban.out  
sh ./gen3.sh > 01-smp-r-city.out
echo "$(date)"
echo "Processing 02..."
c++ gen_stats2.cpp
sh ./gen1.sh > 02-smp-i-rural.out
sh ./gen2.sh > 02-smp-i-urban.out  
sh ./gen3.sh > 02-smp-i-city.out
echo "$(date)"
echo "Processing 03..."
c++ gen_stats3.cpp
sh ./gen1.sh > 03-otfp-r-rural.out
sh ./gen2.sh > 03-otfp-r-urban.out  
sh ./gen3.sh > 03-otfp-r-city.out
echo "$(date)"
echo "Processing 04..."
c++ gen_stats4.cpp
sh ./gen1.sh > 04-otfp-i-rural.out
sh ./gen2.sh > 04-otfp-i-urban.out  
sh ./gen3.sh > 04-otfp-i-city.out
echo "$(date)"
echo "Processing 05..."
c++ gen_stats5.cpp
sh ./gen1.sh > 05-glrp-rural.out
sh ./gen2.sh > 05-glrp-urban.out  
sh ./gen3.sh > 05-glrp-i-city.out
echo "$(date)"
echo "Complete."