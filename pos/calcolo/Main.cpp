#include "pmt.h"
#include <iostream>
#include <cmath>
#include <string>
#include <fstream>

using namespace std;

int main(int argc, char* argv[]){
    if(argc != 6){
        cout << "Usage: " << argv[0] << " <pmt_positions> <source position_x> <source position_y> <source position_z>  <output file>\n";
        return 1;
    }

    ifstream input(argv[1]);
    if(!input.is_open()){
        cout << "Error opening file: " << argv[1] << endl;
        return 1;
    }

    ofstream output(argv[5]);

    double x_s= atof(argv[2]);
    double y_s= atof(argv[3]);
    double z_s= atof(argv[4]);
    double n_w = 1.54;
    double n_s = 1.54;

    int i;
    double x, y, z;
    string a; 

    while(input >> i >> x >> y >> z >> a){
        Pmt temp= Pmt(i,x,y,z,a);
        temp.initialize(x_s, y_s, z_s, n_s, n_w);

        output << i<< " " << x << " " << y << " " << z << " " << temp.get_time() << "\n";

    }

    input.close();
    output.close();
    return 0;
}