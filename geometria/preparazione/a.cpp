#include <iostream>
#include <fstream>
#include <string>

using namespace std;

int main(){

    ifstream input("pmt_positions.csv");
    ofstream output("pmt_pos_hamamatsu.csv");

    int i;
    double x, y, z;
    string a;
    while(input >> i >> x>> y>> z >>a){

        if(a=="Hamamatsu"){
            output << i <<" " << x << " " << y << " " << z <<" " << a << "\n";
        }
    }

    input.close();
    output.close();

    return 0;
}