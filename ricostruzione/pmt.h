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

    void add_hit(double time, double t_0, double c, double e){
        t_res.push_back(time - t);
        t0.push_back(t_0);
        charge.push_back(c);
        evt_id.push_back(e);
    }


    // Trasferisce i buffer al writer senza copiare gli hit. Il PMT viene svuotato.
    void take_hits(std::vector<double>& times, std::vector<double>& zeros,
                   std::vector<double>& charges, std::vector<double>& events) {
        times.swap(t_res);
        zeros.swap(t0);
        charges.swap(charge);
        events.swap(evt_id);
        // Libera i buffer della riga precedente ricevuti tramite swap.
        std::vector<double>().swap(t_res);
        std::vector<double>().swap(t0);
        std::vector<double>().swap(charge);
        std::vector<double>().swap(evt_id);
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

    const std::vector<double>& get_charge() const
    {
        return charge;
    }

    const std::vector<double>& get_evt_id() const
    {
        return evt_id;
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
    std::vector<double> charge;
    std::vector<double> evt_id;
};

#endif // __Pmt__