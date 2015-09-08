vector<int> randtimes(int q){
    vector<int> t;  //Vector of random times created
    srand (time(NULL)); /* initialize random seed: */
    for(int x = 0; x  <= q; x++){
        int z = rand() % 2000 + 1;    /* generate secret number between 1 and 2000*/
        for(int i=0; i < times.size(); i++){
           cout << times[i] << endl;
        }
        t.push_back(z);
    }
    return t;
}
    
    vector<int> times = randtimes(timeQuantity);
    for(int i=0; i < times.size(); i++){
       cout << times[i] << endl;
    }   }
    }
    cout << keyLengths[r].points[0] << ", ";
    cout << keyLengths[r].points[1] <<endl;
    cout << "__" << endl;
    //////////////////////////////////////////////////
    
       
        for(int i=0; i < keyLengths.size(); i++){
            if(lastLength.points[0] == keyLengths[i].points[0]){
                possibleLengths.push_back(keyLengths[i]);
            }
        }
        
        
        
        Rural 
            Keypoints
            0 1694
            381 0
            1168 3000
            1470 1350
            2141 0
            2199 3000
            2290 800
            3000 395
            3000 1243
            Keylengths
            1 4
            4 1
            2 4
            4 2
            3 4
            4 3
            6 4
            4 6
            4 7
            7 4
            7 9
            9 7
            7 8
            8 7
            5 7
            7 5
        City
            Keypoints
            0 1343
            0 2000
            10 1350
            110 1100
            280 820
            344 3000
            350 1240
            390 1710
            420 2200
            669 3000
            800 1080
            928 0
            1150 2390
            1680 1100
            2092 3000
            2236 0
            3000 932
            Keylengths
            1 3
            3 1
            1 4
            4 1
            4 7
            7 4
            4 5
            5 4
            5 12
            12 5
            5 11
            11 5
            7 11
            11 7
            11 16
            16 11
            3 8
            8 3
            8 2
            2 8
            8 9
            9 8
            8 14
            14 8
            9 6
            6 9
            9 13
            13 9
            13 10
            10 13
            13 15
            15 13
            13 14
            14 13
            14 17
            17 14