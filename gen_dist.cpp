// filename    : gen_dist.cpp
// author      : george corser, 2015-02-10
// description : this program reads a sorted trace file
//               then computes the distances between vehicles at time, t==0,
//               and writes the output to a distance matrix file
// input       : sorted trace file, e.g. rural.srt, urban.srt, city.srt
// output      : distance matrix file, e.g. rural.srtd, urban.srtd, city.srtd

// overall design...
    // declare and open files
    // declare variables
    // set counter to number of vehicles in time t==0
    // populate xPoint and yPoint 1d arrays
    // populate the 2d array matrix with distances
    // print the contents of the matrix
	// close files

// preprocessor directives
#include <fstream>    // for ifstream, ofstream
#include <string>     // for string data types
#include <iomanip>    // for formatting test setw, setprecision
#include <iostream>   // for endl, fixed
#include <cmath>      // for sqrt()
using namespace std;  // to make code more readable (no "std::" prefix)

double dist(double x1, double y1, double x2, double y2);

// function: main ------------------------------------------------------------
// prints distance matrix for vehicles in trace file at time, t==0
int main()
{

    // declare and open files
    ifstream inData;		      // declare input file object
    ofstream outData;		      // declare output file object
    inData.open("rural.srt"); 	  // open infile
    outData.open("rural.srtd");   // open outfile
	if (!inData.is_open()) cout << "Input file is not open" << endl;
	if (!outData.is_open()) cout << "Output file is not open" << endl; 

    // declare variables
    int t, v;                     // time, vehicle number
    float x, y;                   // geographical coordinates
	int counter = 0;
	int ti;
	
    // set counter to number of vehicles in time t==0
    t = 0; ti = 0;                // time counter
    while (t==ti)                 // loop through all vehicles where t==0	
    {
        // read values from input file into variables
		// assume at least one record in file
        inData >> t >> v >> x >> y;
		counter++;
    }
	cout << counter << endl;

    double xPoint[counter];
    double yPoint[counter];
    double matrix[counter][counter];
    
    // populate xPoint and yPoint 1d arrays
    inData.close();
    inData.open("rural.srt"); 	 // open input file
    t = 0; ti = 0;               // time counter
    int index = 0;
    while (t==ti)                // loop through all vehicles where t==0	
    {
        // read values from input file into variables
		// assume at least one record in file
        inData >> t >> v >> x >> y;
        xPoint[index] = x;
        yPoint[index] = y;
        index++;
    }
    
    // populate the 2d array matrix with distances
    for(int i=0; i<counter; i++)
    {
      for(int j=0; j<counter; j++)
      {
        matrix[i][j] = dist(xPoint[i], yPoint[i], xPoint[j], yPoint[j]);
      }
    }
    
    // print the contents of the matrix
    outData << setprecision(4) << fixed;
    for(int i=0; i<counter; i++)
    {
      for(int j=0; j<counter; j++)
      {
        outData << setw(10) << matrix[i][j] << " " ;
      }
      outData << std::endl;
    }
    outData << std::endl;
    
	// close files
	inData.close();
	outData.close();
}

// function: distance --------------------------------------------------------
// returns distance between two sets of x,y coordinates
double dist(double x1, double y1, double x2, double y2)
{
  return sqrt((x2-x1)*(x2-x1) + (y2-y1)*(y2-y1));
}