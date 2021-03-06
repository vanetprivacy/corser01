// filename    : gen_stats1.cpp
// author      : george corser, 2015-02-10
// description : this program reads .srtt file (form: t, v, x, y, begin, end)
//               then computes stats for each vehicle (k, d, t)
//               and writes the output to a file
// input       : sorted trace file, e.g. rural.srtt, urban.srtt, city.srtt
//               command line arguments: ./a.out radius zone_qty
//               edit, recompile to access correct input file, e.g. rural
// output      : same as input file but with times appended to each line
//               (output form: v, k, d, t)
// model       : SMP-R
//
// rev:2015-03-21
// use ./gen.sh to run this program in batch mode
// silent period set to 30 seconds
//
// overall design: main() function...
    // initialize command line variables
    // declare and open files
    // declare variables
    // read input file line-by-line to find max vehicle id and max time
    // declare and initialize anonymity set arrays
    // declare and initialize vehicle stat arrays
    // re-read input file line-by-line and accumulate stats into vehicle arrays
        // set current zone based on current time
        // assign current vehicle begin, current, end values: x, y, t
        // assign anonymity set to vehicle, if vehicle in range of beacon
                    // assign anon set id to vehicle
                    // increment size of current anon set
                    // set assign time, x, t to current time, x, y
                    // set silent time to current time + assign_delay time
                    // set resume time to current time + assign_delay + silent 
        // collect stats: k, d, t 
    // write output file

// preprocessor directives
#include <fstream>    // for ifstream, ofstream
#include <string>     // for string data types
#include <iomanip>    // for formatting test setw, setprecision
#include <iostream>   // for endl, fixed
#include <cmath>      // for sqrt()
#include <stdlib.h>   // for exit()
using namespace std;  // to make code more readable (no "std::" prefix)

// function prototypes
bool inrange(float x1, float y1, float x2, float y2, float r);
float get_distance(float x1, float y1, float x2, float y2);
float get_avgdist(int v, int x, int y, int k_id, int k_size, int v_k_id[], 
    int v_current_x[], int v_current_y[], int v_terminated[], int maxv);

// function: main ------------------------------------------------------------
int main(int ArgumentsCount, char *argv[])
{
    if(ArgumentsCount!=4)
    {
        cout<<"ERROR: Please supply four arguments. (Had "<<ArgumentsCount<<")."<<endl;
        return 1;
    }
    // initialize command line variables
	int zone_qty   = atoi(argv[2]);  // 50 zones x 40 sec == 2000 sec
	int radius     = atoi(argv[1]);  // anon set radius, in meters
	int input_type = atoi(argv[3]);   // 0=rural, 1=urban, 2=city
	
    // declare and open files
    ifstream inData;		         // declare input file object
    switch (input_type)
    {
        case 0:
                inData.open("../vanet-srtt/rural.srtt"); 	     // open infile
                break;
        case 1:
                inData.open("../vanet-srtt/urban.srtt"); 	     // open infile
                break;
        case 2:
                inData.open("../vanet-srtt/city.srtt"); 	     // open infile
                break;
        default:
                cout << "invalid input_type: " << input_type << endl;
                return -1;
                break;
    }

	if (!inData.is_open()) cout << "Input file is not open" << endl;

    // declare variables
    int t, v, b, e;                  // time, vehicle number, begin, end
    float x, y;                      // geographical coordinates
	int maxv    = 0;
	int maxt    = 0;
	int k_id    = 0;                 // current anon set
	float out_k = 0.0;
	float out_d = 0.0;
	int out_t   = 0;
	float avg_k = 0.0, avg_d = 0.0, avg_t = 0.0;
    int counter = 0;
    int anoncount = 0;               // count of all anonymized vehicles

    // read input file line-by-line to find max vehicle id and max time
    while (inData >> t >> v >> x >> y >> b >> e)   // loop through all input 	
    {
		if (v>maxv) maxv=v;
		if (t>maxt) maxt=t;
    }
    int zone_time = maxt/zone_qty;
    int assign_delay = 0;
    // int silent = zone_time;
    int silent = 30; // rev:2015-03-21
    
    // declare and initialize anonymity set arrays
    int temp_x;
    int arr_x[3] = {2290, 1430, 390}; temp_x = arr_x[input_type];
    int temp_y;
    int arr_y[3] = {800, 2490, 1710}; temp_y = arr_y[input_type];
    int k_assign_x[zone_qty];
    int k_assign_y[zone_qty];
    int k_assign_r[zone_qty];
    int k_assign_t[zone_qty]; // time vehicle are assigned to anon. set
    int k_silent_t[zone_qty]; // start of silent period
    int k_resume_t[zone_qty]; // time vehicle is anonymized and LBS-connected
    int k_size[zone_qty];     // size of anon set
    int k_size_max[zone_qty]; // highest value of the anon set gpc20150829

    for (int i=0; i<zone_qty; i++) k_assign_x[i] = temp_x;
    for (int i=0; i<zone_qty; i++) k_assign_y[i] = temp_y;
    for (int i=0; i<zone_qty; i++) k_assign_r[i] = radius;
    for (int i=0; i<zone_qty; i++) k_assign_t[i] = i*zone_time;
    for (int i=0; i<zone_qty; i++) k_silent_t[i] = k_assign_t[i] + assign_delay;
    for (int i=0; i<zone_qty; i++) k_resume_t[i] = k_silent_t[i] + silent;
    for (int i=0; i<zone_qty; i++) k_size[i]     = 0; 
    for (int i=0; i<zone_qty; i++) k_size_max[i] = 0; // gpc20150829

    // declare and initialize vehicle stat arrays
    int v_begin_t[maxv+1];
    int v_begin_x[maxv+1];
    int v_begin_y[maxv+1];
    int v_current_t[maxv+1];
    int v_current_x[maxv+1];
    int v_current_y[maxv+1];
    int v_end_t[maxv+1];
    int v_end_x[maxv+1];
    int v_end_y[maxv+1];
    int v_terminated[maxv+1];
    
    int v_k_id[maxv+1];
    int v_k_assign_t[maxv+1];
    int v_k_assign_x[maxv+1];
    int v_k_assign_y[maxv+1];
    int v_k_silent_t[maxv+1];
    int v_k_silent_x[maxv+1];
    int v_k_silent_y[maxv+1];
    int v_k_resume_t[maxv+1];
    int v_k_resume_x[maxv+1];
    int v_k_resume_y[maxv+1];
    int      v_kksum[maxv+1];
    double   v_kdsum[maxv+1];
    
    for (int i=0; i<maxv+1; i++) v_begin_t[i]    = -1;
    for (int i=0; i<maxv+1; i++) v_begin_x[i]    = -1;
    for (int i=0; i<maxv+1; i++) v_begin_y[i]    = -1;
    for (int i=0; i<maxv+1; i++) v_current_t[i]  = -1;
    for (int i=0; i<maxv+1; i++) v_current_x[i]  = -1;
    for (int i=0; i<maxv+1; i++) v_current_y[i]  = -1;
    for (int i=0; i<maxv+1; i++) v_end_t[i]      = -1;
    for (int i=0; i<maxv+1; i++) v_end_x[i]      = -1;
    for (int i=0; i<maxv+1; i++) v_end_y[i]      = -1;
    for (int i=0; i<maxv+1; i++) v_terminated[i] = -1;
    
    for (int i=0; i<maxv+1; i++) v_k_id[i]       = -1;
    for (int i=0; i<maxv+1; i++) v_k_assign_t[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_assign_x[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_assign_y[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_silent_t[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_silent_x[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_silent_y[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_resume_t[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_resume_x[i] = -1;
    for (int i=0; i<maxv+1; i++) v_k_resume_y[i] = -1;
    for (int i=0; i<maxv+1; i++)    v_kksum[i]   = -1;
    for (int i=0; i<maxv+1; i++)    v_kdsum[i]   = -1.0;
    
    
    
    
    // re-read input file line-by-line and accumulate stats into vehicle arrays
    inData.clear();
    inData.seekg(0, inData.beg);  // reposition to beginning of input file
    while (inData >> t >> v >> x >> y >> b >> e)   // loop through all input 	
    {
        // set current zone based on current time
        k_id = t / zone_time;
        
        // assign current vehicle begin, current, end values: x, y, t
        if (v_begin_t[v] == -1) // if begin values not set, then set them
        {
            v_begin_t[v]= t; 
            v_begin_x[v]= x;
            v_begin_y[v]= y;
        }
        v_current_t[v]= t;  // always set current x, y, t
        v_current_x[v]= x;
        v_current_y[v]= y;
        if (e==t) // if e=end equals t=currenttime
        {
            v_end_t[v]= t; 
            v_end_x[v]= x;
            v_end_y[v]= y;
            if (v_terminated[v] == -1)
                if (v_k_id[v] != -1) k_size[v_k_id[v]]--; // decrement k_size
            v_terminated[v] = t;
        }
        
        // assign anonymity set to vehicle, if vehicle in range of beacon
        // assume beacon at center of anon zone
        if (v_k_id[v]==-1) // if vehicle not already a member of anon set
          if (v_terminated[v] == -1) // if not already terminated
            if (t==k_assign_t[k_id]) // if current time is beacon assign time
                if (inrange(x, y, k_assign_x[k_id], k_assign_y[k_id], 
                    k_assign_r[k_id])) // if vehicle is in range of beacon
                {
                    anoncount++;
                    // assign anon set id to vehicle
                    v_k_id[v] = k_id;
                    // increment size of current anon setls 
                    
                    k_size[k_id]++;
                    k_size_max[k_id]++; //gpc20150829
                    // set assign time, x, t to current time, x, y
                    v_k_assign_t[v] = t;
                    v_k_assign_x[v] = x;
                    v_k_assign_y[v] = y;
                    // set silent time to current time + assign_delay time
                    v_k_silent_t[v] = t + assign_delay;
                    // set resume time to current time + assign_delay + silent 
                    v_k_resume_t[v] = t + assign_delay + silent;
                }
                
                
        // collect stats: k, d, t 
        // if resume_time exists and current time >= resume time
        // and end time not exists, then update k, d, t
        if (v_k_resume_t[v]!=-1 && v_end_t[v]==-1 && t>=v_k_resume_t[v]) 
        {
            // update k
            if (v_kksum[v]==-1) v_kksum[v]=0; // initialize to zero
            else v_kksum[v] += k_size[v_k_id[v]]; // incr by anon set size
            // potential problem: k is incremented above for vehicle id's 
            // lower or equal to current vehicle, but not higher
            
            // update d
            if (v_kdsum[v]==-1) v_kdsum[v]=0; // initialize to zero
            else v_kdsum[v] += get_avgdist(v, v_current_x[v], v_current_y[v],
                v_k_id[v], k_size[v_k_id[v]], v_k_id, v_current_x, v_current_y,
                v_terminated, maxv); 
            
            // update t: always end time minus resume time
        }
    }
    
    // write output file
    for (int i=0; i<maxv+1;i++)
        if(v_kksum[i] > -1) 
        {
            out_t = v_end_t[i] - v_k_resume_t[i];
            out_k = float(v_kksum[i]) / out_t;
            out_d = float(v_kdsum[i]) / out_t;
            counter++;
            avg_t += out_t;
            avg_d += out_d;
            avg_k += out_k;
        }
    avg_k = float(avg_k) / counter;
    avg_d = float(avg_d) / counter;
    avg_t = float(avg_t) / counter;
    
    float avg_max_k = 0; //gpc20150829
    for (int i=0; i<zone_qty; i++) avg_max_k += k_size_max[i];  //gpc20150829
    avg_max_k = float(avg_max_k) / zone_qty; //gpc20150829
    
    if (input_type == 0) // rural
    cout << "smp-r density1 " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " " << avg_max_k << endl; //gpc20150829
    if (input_type == 1) // urban
        cout << "smp-r density2 " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " " << avg_max_k << endl; //gpc20150829
    if (input_type == 2) // city
        cout << "smp-r density3 " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " " << avg_max_k << endl; //gpc20150829




	// close files
	inData.close();
}

// ----- function: inrange --------------------------------------------------
// returns true if (x1,y1) is within distance r of (x2,y2); else returns false
bool inrange(float x1, float y1, float x2, float y2, float r)
{
    if (r >= sqrt ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) ) ) return true;
    else return false;
}

// ----- function: get_distance ---------------------------------------------
float get_distance(float x1, float y1, float x2, float y2)
{
    return sqrt ( (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2) );
}

// ----- function: get_avgdist ----------------------------------------------
float get_avgdist(int v, int x, int y, int k_id, int k_size, int v_k_id[], 
    int v_current_x[], int v_current_y[], int v_terminated[], int maxv)
{
    float dist_sum = 0.0;
    int counter = 0;
    for (int i=0; i<maxv+1;i++)
    {
        if (v_terminated[i]<0)
        {
            if (v_k_id[i] == k_id) dist_sum += get_distance(x, y, 
                v_current_x[i], v_current_y[i]);
            if (v_k_id[i] == k_id) counter++; 
        }
    }
    // verify k_size matches number of unterminated anon set members
    if (k_size != counter) cout << "error: anon set size mismatch: " 
        << k_size << " (array value) should be " << counter << " (counter)"
        << endl;
    return dist_sum / counter;
}