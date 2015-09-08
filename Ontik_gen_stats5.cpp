// filename    : gen_stats5.cpp
//               similar to gen_stats.cpp except continuous times
//               AND contiguous locations (GLRZ: group leader relay zone)
//               members join in pairs
// author      : george corser, rev:2015-03-04
// description : this program reads .srtt file (form: t, v, x, y, begin, end)
//               then computes stats for each vehicle (k, d, t)
//               and writes the output to a .stat file
//               (form: v, k, d, t)
// input       : sorted trace file, e.g. rural.srtt, urban.srtt, city.srtt
//               command line arguments: ./a.out radius zone_qty
//               edit, recompile to access correct input file, e.g. rural
// output      : same as input file but with times appended to each line

// overall design: main() function...
    // initialize command line variables
    // declare and open files
    // declare variables
    // declare anonymity set arrays rev:2015-02-28
    // read input file line-by-line to find max vehicle id and max time
    // initialize anonymity set arrays - RESET WHEN RE-READING INPUT FILE
    // declare and initialize vehicle stat arrays
    // re-read input file line-by-line and accumulate stats into vehicle arrays
        // RESET ANONYMITY SET ARRAY ELEMENTS rev:2015-03-04
        // set current zone BASED ON NEAREST GROUP LEADER
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
int gl_inrange(int x, int y, int k_assign_x[], int k_assign_y[], int zone_qty,
    int radius); // rev:2015-03-04
bool is_leader(int v, int maxv, int zone_qty); // rev:2015-03-04
void rand_times(int k_assign_t[], int maxt, int zone_qty); // rev:2015-02-27
bool inrange(float x1, float y1, float x2, float y2, float r);
float get_distance(float x1, float y1, float x2, float y2);
float get_avgdist(int v, int x, int y, int k_id, int k_size, int v_k_id[], 
    int v_current_x[], int v_current_y[], int v_terminated[], int maxv);

// function: main ------------------------------------------------------------
int main(int argc, char *argv[])
{
    // initialize command line variables
	int zone_qty= atoi(argv[2]);  // 50 zones x 40 sec == 2000 sec
	int radius  = atoi(argv[1]);  // anon set radius, in meters
    bool debug = false;            // 3=test requires debug = true
	int input_type = atoi(argv[3]);// 0=rural, 1=urban, 2=city, 3=test
	
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
    int tempint = 0;
    float tempfloat = 0;
    
    // declare anonymity set arrays
    int temp_x;
    int arr_x[3] = {2290, 1430, 390}; temp_x = arr_x[input_type];
    int temp_y;
    int arr_y[3] = {800, 2490, 1710}; temp_y = arr_y[input_type];
    int k_assign_gl[zone_qty]; // group leader of zone rev:2015-03-04
    int k_assign_gfw[zone_qty]; // group follower waiting rev:2015-03-04
    int k_assign_x[zone_qty];
    int k_assign_y[zone_qty];
    int k_assign_r[zone_qty];
    int k_assign_t[zone_qty]; // time vehicle are assigned to anon. set
    int k_silent_t[zone_qty]; // start of silent period
    int k_resume_t[zone_qty]; // time vehicle is anonymized and LBS-connected
    int k_size[zone_qty];     // size of anon set

    // read input file line-by-line to find max vehicle id and max time
    while (inData >> t >> v >> x >> y >> b >> e)   // loop through all input 	
    {
		if (v>maxv) maxv=v;
		if (t>maxt) maxt=t;
    }
    int zone_time = maxt/zone_qty;
    int assign_delay = 0;
    // int silent = zone_time;
    int silent = 30;
    if(debug) cout << maxv << " " << maxt << endl;
    if(debug) silent = 0;
    if (debug) zone_time = 3;
    if (debug) zone_qty = 1;

    // initialize anonymity set arrays - RESET WHEN RE-READING INPUT FILE
    // create active group leader zone array, k_assign_gl rev:2015-03-04
    // if there are 1000 vehicles and 50 zones, then every 20th car is a GL
    for (int i=0; i<zone_qty; i++) k_assign_gl[i] = i * maxv / zone_qty + 1; 
    for (int i=0; i<zone_qty; i++) k_assign_gfw[i] = -1; // grp follwer waitng
    for (int i=0; i<zone_qty; i++) k_assign_x[i] = -1;
    for (int i=0; i<zone_qty; i++) k_assign_y[i] = -1;
    for (int i=0; i<zone_qty; i++) k_assign_r[i] = radius;
    for (int i=0; i<zone_qty; i++) k_assign_t[i] = -1;
    for (int i=0; i<zone_qty; i++) k_silent_t[i] = k_assign_t[i] + assign_delay;
    for (int i=0; i<zone_qty; i++) k_resume_t[i] = k_silent_t[i] + silent;
    for (int i=0; i<zone_qty; i++) k_size[i]     = 0; 

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
    int v_k_g[maxv+1]; // group that v is following rev:2015-03-04
    int v_k_gw[maxv+1]; // group that v is waiting to follow rev:2015-03-04
    int v_k_gm[maxv+1]; // vehicle's fellow group member (join in pairs)
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
    for (int i=0; i<maxv+1; i++) v_k_g[i]       = -1; // rev:2015-03-04
    for (int i=0; i<maxv+1; i++) v_k_gw[i]      = -1; // rev:2015-03-04
    for (int i=0; i<maxv+1; i++) v_k_gm[i]      = -1; // rev:2015-03-04
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
    
    // For GLRP: Total Overhead = Announcement + Join Req + Key Gen + Join Confirm + LBS Relay
    double TotalOverheadCounter;
    double AnnouncementOverhead = maxt;
    double JoinReqOverhead = 0;
    double KeyGenOverhead = 0;
    double JoinConfirmOverhead = 0;
    double LBSRelayOverhead;
    
    // re-read input file line-by-line and accumulate stats into vehicle arrays
    inData.clear();
    inData.seekg(0, inData.beg);  // reposition to beginning of input file
    while (inData >> t >> v >> x >> y >> b >> e)   // loop through all input 	
    {
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
            // gl anon set size always == 2 because join in pairs
            //if (v_terminated[v] == -1)
                //if (v_k_id[v] != -1) k_size[v_k_id[v]]--; // decrement k_size
            if (v_terminated[v] == -1) 
            {
                if (debug) cout << v << " terminated at " << t << endl;
                v_terminated[v] = t;
                v_k_g[v] = -2;
                // terminate other vehicle in group
                if (v_k_gm[v] > 0) v_terminated[v_k_gm[v]] = t;
                if (v_k_gm[v] > 0) v_k_g[v_k_gm[v]] = -2;
            }
        }
        
        // set current zone BASED ON NEAREST GROUP LEADER
        // assume all vehicles are either leaders or followers rev:2015-03-04
        if ( is_leader(v, maxv, zone_qty) )  // if current vehicle is a leader
        {
            if (debug) cout << v << " is a leader" << endl;
            // PROCESS LEADER (gl)
            tempint = ( (v-1) % (maxv/zone_qty) );
            k_assign_gl[tempint] = v;              // group leader == self
            k_assign_x[tempint] = x;               // update x,y,t
            k_assign_y[tempint] = y;
            k_assign_r[tempint] = radius;
            k_assign_t[tempint] = t; 
            k_silent_t[tempint] = k_assign_t[tempint] + assign_delay;
            k_resume_t[tempint] = k_silent_t[tempint] + silent;
            
            // terminate gl if necessary
            if (e==t) // if e=end equals t=currenttime
            // values below are used in gl_inrange function
            // when gl terminates, all members terminate next time period
            {
                if (debug) cout << v << " terminated at " << t << endl;
                k_assign_x[tempint] = -2;
                k_assign_y[tempint] = -2;
                v_terminated[v] = t;
            }
            
            continue;                        // get next vehicle
        }
        else                                 // current vehicle is a follower
        {
            if (debug) cout << v << " is a follower" << endl;
            if (v_k_g[v] > -1) // if v already belongs to a group (aka:zone)
            {
                if (debug) cout << v << " belongs to group " << v_k_g[v]<< endl;
                if (!inrange(x,y,k_assign_x[v_k_g[v]],
                    k_assign_y[v_k_g[v]],radius))  // if moved out of gl range
                {
                    if (debug) cout << v << " moved out of range at:" 
                    << t << endl;
                    // TERMINATE FOLLOWER
                    // note: vehicle terminates anon period in 2 ways, 
                    // 1. it exits the region, and 2. it moves out of gl range
                    // vehicles join in pairs but do not terminate in pairs
                    // when one follower moves out of range
                    // because while there is only one of two vehicles in group
                    // attacker does not know which one
                    v_terminated[v] = t -1; // terminate anonymity period
                    //v_end_t[v] = t; // but do not update end of trajectory
                    //v_end_x[v] = x;
                    //v_end_y[v] = y;
                    //if (v_terminated[v]==-1) // if not already terminated
                    //    k_size[v_k_id[v]]--; // decrement anon set size
                    v_k_g[v] = -2;
                    continue;
                }
                // else, in range, so: COLLECT STATS BELOW
            }
            else // v has no group (zone)
            {
                if (debug) cout << v << " has no group leader" << endl;
                if (v_terminated[v] < 0) // if v is NOT already terminated
                {
                    // find nearest group leader
                    tempint = gl_inrange(x,y,k_assign_x,k_assign_y,
                        zone_qty,radius);
                    if (debug) cout << v << " nearest leader is group:" 
                        << tempint << endl;
                    if (tempint > -1)              // if there is a gl in range
                    {
                        if (k_assign_gfw[tempint] > -1)   // if gl has gfw
                        {
                            // JOIN
                            // v and gfw join group
                            if (debug) cout << v << " joined group:" 
                                << tempint << endl;
                            v_k_g[v] = tempint;
                            v_k_gw[v] = -2;
                            v_k_gm[v] = k_assign_gfw[tempint];
                            v_k_assign_t[v] = t;
                            v_k_assign_x[v] = x;
                            v_k_assign_y[v] = y;
                            v_k_silent_t[v] 
                                = t + assign_delay;
                            v_k_resume_t[v] 
                                = t + assign_delay + silent;
                            v_k_id[v] = tempint;
                            
                            if (debug) cout << k_assign_gfw[tempint] 
                                << " joined group:" << tempint << endl;
                            v_k_g[k_assign_gfw[tempint]] = tempint;
                            v_k_gw[k_assign_gfw[tempint]] = -2;
                            v_k_gm[k_assign_gfw[tempint]] = v;
                            v_k_assign_t[k_assign_gfw[tempint]] = t;
                            v_k_assign_x[k_assign_gfw[tempint]] = x;
                            v_k_assign_y[k_assign_gfw[tempint]] = y;
                            v_k_silent_t[k_assign_gfw[tempint]] 
                                = t + assign_delay;
                            v_k_resume_t[k_assign_gfw[tempint]] 
                                = t + assign_delay + silent;
                            v_k_id[k_assign_gfw[tempint]] = tempint;
                            
                            k_assign_gfw[tempint]= -1;
                            k_size[tempint] = 2;
                            
                            anoncount += 2;
                            JoinReqOverhead += 2; // Join Request = N (Number of Anon Vehicles)
                            JoinConfirmOverhead += 2; // Same as above
                            KeyGenOverhead += 2; // Same as above
                            
                            continue; // collect stats next time around
                        }
                        else // no gfw... must wait (join in pairs)
                        {
                            if (debug) cout << v << " waiting" << endl;
                            // WAIT
                            // set group follower waiting (gfw)
                            k_assign_gfw[tempint] = v;
                            // set group this vehicle is waiting for
                            v_k_gw[v] = tempint; 
                            continue;
                        }
                    }
                    else continue; // no gl in range... must keep seeking gl
                }
                else continue; // already terminated
            }
        }

        // at this point, v_k_id[v] (anon set) is tempint == k_id
        //k_id = tempint;
        
        if (debug) cout << "collecting stats for:" << v << endl;
        if (debug) cout 
            << "  resume time:" << v_k_resume_t[v] << endl
            << "     end time:" << v_end_t[v] << endl
            << " current time:" << t  << endl
            << " term time:" << v_terminated[v] << endl;
        
        // collect stats: k, d, t 
        // if resume_time exists and current time >= resume time
        // and end time not exists, then update k, d, t
        // and not terminated
        if (v_k_resume_t[v]!=-1 && v_end_t[v]==-1 && t>=v_k_resume_t[v]
            && v_terminated[v]<0) 
        {
            if (debug) cout << " computing kdt for: " << v << endl;
            // update k
            if (v_kksum[v]==-1) v_kksum[v]=0; // initialize to zero
            //v_kksum[v] += k_size[v_k_id[v]]; // incr by anon set size
            v_kksum[v] += 2; // incr by anon set size

            // update d
            if (v_kdsum[v]==-1) v_kdsum[v]=0; // initialize to zero
            tempfloat = get_distance(x,y,v_current_x[v_k_gm[v]],
                v_current_y[v_k_gm[v]]) / 2;
            v_kdsum[v] += tempfloat;

            // update t: always terminated time minus resume time
        }
        
    } // end while
    
    // calculating LBS Overhead
    LBSRelayOverhead = (anoncount*maxt)/2;
    // For GLRP: Total Overhead = Announcement + Join Req + Key Gen + Join Confirm + LBS Relay
    TotalOverheadCounter = AnnouncementOverhead + JoinReqOverhead + KeyGenOverhead + JoinConfirmOverhead + LBSRelayOverhead;
    
    if(debug) cout << "writing output file" << endl;

    // write output file
    for (int i=0; i<maxv+1;i++)
        if(v_kksum[i] > -1) 
        {
            if (debug) cout << v << " " << v_kksum[i] << " " << v_kdsum[i] << endl;
            //out_t = v_end_t[i] - v_k_resume_t[i];
            out_t = v_terminated[i] - v_k_resume_t[i];
            if (out_t <= 0) continue;
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
    avg_k = 2; // to fix rounding errors

    if (input_type == 0) // rural
    cout << "glrp-2 density1 " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " Total Overhead " << TotalOverheadCounter << " " << endl;
    if (input_type == 1) // urban
        cout << "glrp-2 density2 " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " Total Overhead " << TotalOverheadCounter << " " << endl;
    if (input_type == 2) // city
        cout << "glrp-2 density3 " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " Total Overhead " << TotalOverheadCounter << " " << endl;
    if (input_type == 3) // test
        cout << "glrp-2 test " << radius << " " << zone_qty << setprecision(5) 
        << fixed << " " << avg_k << " " << avg_d << " " << avg_t 
        << " " << anoncount << " " << maxv << " Total Overhead " << TotalOverheadCounter << " " <<  endl;

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
    /* k always 2 for glrz
    // verify k_size matches number of unterminated anon set members
    if (k_size != counter) cout << "error: anon set size mismatch: " 
        << k_size << " (array value) should be " << counter << " (counter)"
        << endl;
    */
    return dist_sum / counter;
}

// ----- function: rand_times ----------------------------------------------
void rand_times(int k_assign_t[], int maxt, int zone_qty) // rev:2015-02-27
{
    int X[maxt];                         // 1. array of all possible times
    for (int i=0; i<maxt; i++) X[i] = i; // 2. initialize in sequence
    for (int i = 0; i < maxt; i++)
        swap(X[i],X[i+rand()%(maxt-i)]); // 3. randomize, put out-of-sequence
    int Y[zone_qty];                     // 4. array of randomly selected times
    
    // 5. assign the first "zone_qty" values in X to Y
    for (int i=0; i<zone_qty; i++) Y[i] = X[i];
    
    // 6. sort Y and set first time to zero
    for(int i=0;i<zone_qty;i++)
        for(int j=0;j<zone_qty;j++)
            if(Y[i]<Y[j]) 
                swap(Y[i],Y[j]);
    Y[0]=0;
    
    // 7. assign Y to k_assign_t
    for (int i=0; i<zone_qty; i++) k_assign_t[i] = Y[i];
}

// ----- function: is_leader ------------------------------------------------
bool is_leader(int v, int maxv, int zone_qty)
{
    return ( (v-1) % (maxv/zone_qty) ) == 0;
}
// explanation of function: is_leader() 
// every (maxv/zone_qty) vehicle is a leader 
// but vehicle numbering starts at 1, not 0,
// so if (v-1) mod (maxv/zone_qty) is zero, 
// then vehicle is a group leader

// ----- function: gl_inrange -----------------------------------------------
int gl_inrange(int x, int y, int k_assign_x[], int k_assign_y[], int zone_qty,
    int radius)
{
    int gl = -1;
    float dist1 = 9000000.0;
    float dist2 = 9000000.0;

    // find nearest group leader    
    for (int i=0; i<zone_qty; i++)
    {
        if (k_assign_x[i]>-1)
        {
            dist2 = get_distance(x,y,k_assign_x[i],k_assign_y[i]);
            if (dist1 > dist2) 
            {
                dist1 = dist2;
                gl = i;
            }
        }
    }
    
    if (dist1 > radius) gl = -1;

    return gl;
}
// returns index number (zone number) of nearest group leader, 
// within radius r, or -1 if none