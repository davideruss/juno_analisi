#ifndef __Pmt__
#define __Pmt__
#include <string>
#include <cmath>
#include <limits>
#include <iostream>

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
        sin_theta =std::sqrt(1-cos_theta*cos_theta);
        theta= std::acos(cos_theta);
        cos_phi=x/(r*sin_theta);
        sin_phi=y/(r*sin_theta);
        theta_lim =std::acos(R/r);
        theta_min = std::max(0.0, theta-theta_lim);
        theta_max = std::min(M_PI, theta+theta_lim);
    }
    
    
    void tof_approx(double x_s, double y_s, double z_s, double n_s, double n_w){
        double pos=x_s*x_s+ y_s*y_s+ z_s*z_s;
        dist_s= std::sqrt((x-x_s)*(x-x_s)+(y-y_s)*(y-y_s)+(z - z_s)*(z - z_s));
        
        double b= (x_s*(x-x_s) + y_s*(y-y_s) + z_s*(z-z_s))/dist_s; //questo sarebbe cos_alfa*sqrt(pos) perché è il termine che poi entrerà nell'equazione
        
        //Adesso devo calcolare il tempo di volo per farlo, calcolo la distanza che percorre nel LS (a nel codice) e quella che percorre in acqua (dist_s-a)
        //per fare questo mi calcolo prima l'angolo formato dal vettore posizione della sorgente e il vettore distanza tra pmt e sorgente
        //poi posso usare il teorema del coseno per trovare la lunghezza del segmento che ci interessa
        
        double ris= eq_2(1, b , pos-R*R);
        
        t= (ris*(n_s-n_w)+dist_s*n_w)/299.792458; //tempo di volo in nanosecondi
        
    };
    
    void tof_true(double z_s, double n_s, double n_w, double step){

        double t_temp;
        double t_min = std::numeric_limits<double>::infinity();

        // per calcolare il tempo di volo prendo una serie di theta e controllo qual è il punto che minimizza il cammino
        for(double theta=theta_min; theta<=theta_max; theta+= step){
            
            t_temp=t_cal(z_s, theta, n_s, n_w);
            if(t_temp< t_min){
                t_min = t_temp;
                theta_true = theta;
            }
        }
        
        t_true =t_min/299.792458;

    }
    double t_cal(double z_s, double theta, double n_s, double n_w){
        double x_temp;
        double y_temp;
        double z_temp;
        
        x_temp = R*sin(theta)*cos_phi;
        y_temp = R*sin(theta)*sin_phi;
        z_temp = R*cos(theta);
            
        return module(x_temp, y_temp, z_temp-z_s)*n_s + module(x-x_temp, y-y_temp, z-z_temp)*n_w;

    }
    void tof_true2(double z_s, double n_s, double n_w, double error){
        double step = 0.001;
        tof_true(z_s, n_s, n_w, step);
        
        double a, b;
        //if theta true is at the limit of the range, we don't need to do anything else
        if(theta_true==theta_min){
            a = theta_true;
            b = theta_true + step;
        }else if(theta_true>theta_max- step){
            a = theta_true - step;
            b = theta_true;
        }else{
            a = theta_true - step;
            b = theta_true + step;
        }

        double r = (1. + std::sqrt(5))/2. -1;
        double s = 1-r;

        double t_min;
        
        double x1 , x2, t1, t2;
        while((b-a)>error){

            x1 = a + (b-a)*s;
            x2 = a + (b-a)*r;

            t1= t_cal(z_s, x1, n_s, n_w);
            t2 = t_cal (z_s, x2, n_s, n_w);

            if(t1 < t2){
                b= x2;
                t_min = t1;
    
                
            } else {
                a=x1;
                t_min = t2;
            }
        }
        
        theta_true = (a+b)/2;
        t_true = t_min/299.792458;
    }

    bool check_theta(double z_s, double n_s, double n_w){
        
        double der = n_s*R*z_s*std::sin(theta_true)/std::sqrt(R*R+z_s*z_s-2.0*R*z_s*std::cos(theta_true)) + n_w*r*R*std::sin(theta_true-theta)/std::sqrt(r*r+R*R-2.0*r*R*std::cos(theta_true-theta));
        
        // std::cout<<"der: "<<der<< "\nSul pmt "<< i << "\n" << z_s << "\n\n";
        if(std::abs(der)>0.1){
            std::cerr << "Derivata = " << der << " per il PMT " << i << " con z_s = " << z_s << "\n\n";
        }

        return std::abs(der)<0.1;
        
    }

    void initialize(double x_s, double y_s, double z_s, double n_s, double n_w, double err){

        tof_approx(x_s, y_s, z_s, n_s, n_w);
        tof_true2(z_s, n_s, n_w, err);
        
        //bool a = check_theta(z_s, n_s, n_w);
        // if (!check_theta(z_s, n_s, n_w)){
        //     std::cerr << "Derivata non nulla per il PMT " << i << " con z_s = " << z_s << "\n\n";
        // }
    }

    double get_time() const { return t; }

    double get_time_true() const { return t_true; }

    double get_pos_x() const { return x; }

    double get_pos_y() const { return y; }

    double get_pos_z() const { return z; }

    int get_index() const { return i; }

    bool get_type() const {return type;}

    double get_cos_theta() const { return cos_theta; }


private:
    int i;
    double x, y, z;
    bool type;

    double cos_phi, sin_phi, cos_theta, sin_theta, theta;
    double r, dist_s, t, t_true; //modulo posizione pmt, distanza dalla sorgente e tempo di volo
    const double R = 17820; //raggio del LS in mm
    double theta_min, theta_max;
    double theta_true;

    double theta_lim;
    static double eq_2(double a, double b, double c) {
        double dis = b*b - a*c;
        
        // Controllo di sicurezza per evitare radici di numeri negativi
        if (dis < 0) {
            std::cerr << "Errore nella funzione eq_2: discriminante negativo\n";
            return 0.0; 
        }

        return (-b + std::sqrt(dis)) / a;
    }

    static double module(double a, double b, double c){
        return std::sqrt(a*a + b*b+ c*c);
    }
};

#endif // __Pmt__