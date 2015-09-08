#include <iostream>
#include <fstream>
#include <cmath>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
using namespace std;
//keyPoints are major locations on the grid by which keyLengths are created.
struct keyPoint{
    int id;
    int loc[2];   // locations of point
};
//keyLengths are a combination of keyPoints
struct keyLength{
    int id;
    int points[2];   // keypoints Id
};
// the difference between a keylength and length is a length has a start time
// and a duration. a keyLength is more of a possibility where as a length is
// the real thing.
struct length{
    double start;
    int id;
    double startpos[2];
    double endpos[2];
    double points[2];
    double duration;
};
//A path is a collection of lengths chained together by time.
struct path{
        int id;
        vector<length> lengths;
    
};
//Global Variables
vector<keyPoint> keyPoints;
vector<keyLength> keyLengths;
//functiond to load grid data
vector<keyPoint> loadKeyPoints(string loc){
    vector<keyPoint> p;
	ifstream fin(loc.c_str());//Need double backslaches
	if (!fin.is_open()){
		cout << "file error" << endl;
		system("Pause");
		return p; //end main
	}
	
    int id, x, y;
    id = 1;
	while (!fin.eof()){					//Loop while NOT the end of file
		fin >> x >> y;			//Same operator used with cin
		keyPoint point;
		point.id = id;
		id++;
		point.loc[0] = x;
		point.loc[1] = y;
		p.push_back(point);
	}
	//Close the input file
	fin.close();
    return p;
}
vector<keyLength> loadKeyLengths(string loc){
    vector<keyLength> l; //temp vector to hold file input data
	ifstream fin(loc.c_str());
	if (!fin.is_open()){
		cout << "file error" << endl;
		system("Pause");
		return l; //end main
	}
	
    int id, p1, p2;
    id = 0;
	while (!fin.eof()){					//Loop while NOT the end of file
		fin >> p1 >> p2;			//Same operator used with cin
		keyLength length; // create new length
		length.id = id; //sets id based on line
		length.points[0] = p1;
		length.points[1] = p2;
		l.push_back(length);
		id++; //sets id to next line
	}
	//Close the input file
	fin.close();
    return l;
}
//Path and length creation functions
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
length newRandomLength(int newid,double t,path p){
    //create a new length
    length l;
    l.id = newid;
    vector<keyLength> possibleLengths;
    bool finishCheck = false;
    bool firstlength = false;
    if(p.lengths.size() == 0){//if first length of path
        l.start = t;
        possibleLengths = keyLengths;
        firstlength = true;
    }else{//else
        //get last length
        length lastLength = p.lengths[p.lengths.size() - 1];
        l.start = lastLength.start + lastLength.duration + 1;
        //find possible connections
        for(int i=0; i < keyLengths.size(); i++){
            if(lastLength.points[1] == keyLengths[i].points[0]){
                if(lastLength.points[0] != keyLengths[i].points[1]){
                    possibleLengths.push_back(keyLengths[i]);
                }
            }
        }
        if(possibleLengths.size() == 0){
            finishCheck = true;//ends function + path if no possible lengths
        }
    }
    //if possible new random length
    if(finishCheck == false){
        int r = rand() % possibleLengths.size();
        //the id of keypoint the length starts at
        int startpoint = possibleLengths[r].points[0];//the ID of point 1
        l.points[0] = startpoint;//the ID of point 2
        //the id of keypoint the length ends at
        int endpoint = possibleLengths[r].points[1];
        l.points[1] = endpoint;
        //find keypoints with id (enpoint/starptoint)
        keyLength rp = possibleLengths[r];
        keyPoint p1;
        keyPoint p2;
        for(int i=0; i<keyPoints.size(); i++){
            if(keyPoints[i].id == startpoint){
                p1 = keyPoints[i];
                l.startpos[0] = keyPoints[i].loc[0];
                l.startpos[1] = keyPoints[i].loc[1];
            }
            if(keyPoints[i].id == endpoint){
                p2 = keyPoints[i];
                l.endpos[0] = keyPoints[i].loc[0];
                l.endpos[1] = keyPoints[i].loc[1];
            }
        }
        double x1,y1,x2,y2;
        x1 = l.startpos[0];
        y1 = l.startpos[1];
        x2 = l.endpos[0];
        y2 = l.endpos[1];
        //decide if half length
        if(firstlength == true){
            int r2 = rand() % 4;//75% of breaking in half
            while(r2 != 0){
                //find midpoint for halflLengths
                double mx=0.00,my=0.00;
                mx = ((x1 + x2)/2);
                my = ((y1 + y2)/2);
                //creats a half length
                l.startpos[0] = mx;
                l.startpos[1] = my;
                x1 = mx;
                y1 = my;
                //reflip
                r2 = rand() % 4;
            }
        }
        //find distance to caclulate duration
        double d;
        d = pow(((pow((x2-x1),2)) + (pow((y2-y1),2))),0.5);
        d = floor(d*100+0.5)/100;
        l.duration = floor((d/20)*100+0.5)/100;
    }else{
        l.start = -1;
    }
    /*
    cout<< p.id << ": ";
    cout << l.points[0] << ", ";
    cout << l.points[1] << " / ";
    cout << l.start << ", ";
    cout << l.duration << " ";
    cout << endl;
    */
    return l;
}
path newRandomPath(int newid,double t){
    path p;
    p.id = newid;
    //Random chance of adding another length to path
    int r = 0;
    while(r == 0){
        length l = newRandomLength(p.id,t,p);
        if(l.start != -1){
            p.lengths.push_back(l);
            r = rand() % 2;//Heads(0) or Trails(1)?
        }else{
            r = -1;
        }
    }
    return p;
}
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
int main(){
    //declare trace creation variables
    int carQuantity; //how many cars(paths) do you want to make?
    int timeQuantity; //Over how many seconds?
    cout << "How many Cars? (Paths)" << endl;
    cin >> carQuantity;
    cout << "Over how much time?" << endl;
    cin >> timeQuantity;
    string keyPointLoc, keyLengthLoc;
    cout << "Path to Key Point locations file?" << endl;
    cin >> keyPointLoc;
    cout << "Path to Key Length connection file?" << endl;
    cin >>keyLengthLoc ;
    cout << endl << carQuantity << " " <<
              timeQuantity << " " <<
              keyPointLoc << " " <<
              keyLengthLoc << " " <<
              endl;
    //loads preset keyPoints
    keyPoints = loadKeyPoints(keyPointLoc);
    //loads preset lengths
    keyLengths = loadKeyLengths(keyLengthLoc);
    
    //create paths
    vector<path> paths;
    srand (time(NULL)); /* initialize random seed: */
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    for(int i=0; i <= carQuantity - 1; i++){
        /* generate random time between 1 and timeQuantity*/
        double t = rand() % timeQuantity + 1;
        double q = paths.size();
        if(q <= (carQuantity * 0.15)){
            t = 0;  
        }
        //creats path with random time;
        path p = newRandomPath(q,t);
        paths.push_back(p);
    }
    ////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////
    //orginize and display paths (trace data)
    
	//Declare an output file stream
	ofstream fout("out.txt"); //file will be created for you
    fout << "var tracedata = [" << endl;
    for(int i=0; i<paths.size(); i++){
        path p = paths[i];
        //cout << "---" << p.id << ": " << endl;
        for(int c=0; c<p.lengths.size();c++){
           fout << "\"" << p.lengths[c].start << " " << p.lengths[c].id;
           fout << " " << p.lengths[c].startpos[0] << " " << p.lengths[c].startpos[1];
           fout << " " << p.lengths[c].endpos[0] << " " << p.lengths[c].endpos[1];
           fout << " " << p.lengths[c].duration << "\"," << endl;
        }
        //cout << endl;
    }
    fout << "];" << endl;
    cout << "Done" << endl;
    return 0;
}