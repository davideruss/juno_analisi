#ifndef __Pmt__
#define __Pmt__
#include <string>
#include <cmath>
#include <limits>
#include <iostream>
#include <vector>

class Pmt {
public:
    Pmt(int index, double x_pos, double y_pos, double z_pos, const std::string& name){
        i = index;
        x = x_pos;
        y = y_pos;
        z = z_pos;
        type = (name=="Hamamatsu");
        r= std::sqrt(x*x+y*y+z*z);
        cos_theta =z/r;

    }
    
    void tof_approx1(double z_s, double n_s){
        dist_s= std::sqrt(x*x+y*y+(z - z_s)*(z - z_s));

        t= dist_s*n_s/299.792458;
    }
    
    void initialize(double z_s, double n_s){

        dist_s= std::sqrt(x*x+y*y+(z - z_s)*(z - z_s));

        t= dist_s*n_s/299.792458;
        
    }

    void add_t_res(double time){
        t_res.push_back(time - t);
    }
    void add_t0(double t_0){
        t0.push_back(t_0);
    }

    double get_time() const { return t; }

    const std::vector<double>& get_residual_times_ns() const
    {
        return t_res;
    }

    const std::vector<double>& get_t0() const
    {
        return t0;
    }

    double get_pos_x() const { return x; }

    double get_pos_y() const { return y; }

    double get_pos_z() const { return z; }

    int get_index() const { return i; }

    bool get_type() const {return type;}

    double get_cos_theta() const { return cos_theta;}


private:
    int i;
    double x, y, z;
    bool type;

    double cos_theta;
    double r, dist_s, t; //modulo posizione pmt, distanza dalla sorgente e tempo di volo
    std::vector<double> t_res;
    std::vector<double> t0;
};

#endif // __Pmt__