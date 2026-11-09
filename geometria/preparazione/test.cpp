#include <iostream>
#include <fstream>
#include <string>
#include <cmath>

int main(int argc, char**argv){

    if(argc != 3){
        std::cout << "Usage: " << argv[0] << " <file di input> <file di output>\n"; 
        return -1;
    }

    std::ifstream input(argv[1]);
    std::ofstream output(argv[2]);

    int i;
    double x, y, z;
    std::string a;



    int count =0;

    while(input>>i >>x >> y >> z >>a){

        if(std::fabs(x)<260){

            std::cout << "x = " << x << " y = " << y << " z = " << z << "\n";
            output << i << " " << x << " " << y << " " << z << " " << a << "\n";
            count++;
        }
    }

    input.close();
    output.close();

    double r= 17700;
    double b = 504;
    double n = 2*M_PI*r/(504*sqrt(3));
    std::cout<<  "valore di n = " << n << "\n";
    std::cout << count << "\n\n";

    return 0;

}