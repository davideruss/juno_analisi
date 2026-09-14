#ifndef __Pmt__
#define __Pmt__
#include <string>
#include <cmath>
#include <iostream>

class Pmt {
public:
    Pmt(int index, double x_pos, double y_pos, double z_pos, const std::string& name){
        i = index;
        x = x_pos;
        y = y_pos;
        z = z_pos;
        a = name;
        r= std::sqrt(x*x+y*y+z*z);
    }
    void initialize(double x_s, double y_s, double z_s, double n_s, double n_w){
        double pos=x_s*x_s+ y_s*y_s+ z_s*z_s;
        dist_s= std::sqrt((x-x_s)*(x-x_s)+(y-y_s)*(y-y_s)+(z - z_s)*(z - z_s));

        double b= (x_s*(x-x_s) + y_s*(y-y_s) + z_s*(z-z_s))/dist_s; //questo sarebbe cos_alfa*sqrt(pos) perché è il termine che poi entrerà nell'equazione

        double R = 17700; //raggio del LS in mm

        //Adesso devo calcolare il tempo di volo per farlo, calcolo la distanza che percorre nel LS (a nel codice) e quella che percorre in acqua (dist_s-a)
        //per fare questo mi calcolo prima l'angolo formato dal vettore posizione della sorgente e il vettore distanza tra pmt e sorgente
        //poi posso usare il teorema del coseno per trovare la lunghezza del segmento che ci interessa
        
        double ris= eq_2(1, b , pos-R*R);

        t= (ris*(n_s-n_w)+dist_s*n_w)/299.792458; //tempo di volo in nanosecondi
    
    };

    double get_time() const { return t; }

    double get_pos_x() const { return x; }

    double get_pos_y() const { return y; }

    double get_pos_z() const { return z; }


private:
    int i;
    double x, y, z;
    std::string a;

    double r, dist_s, t; //raggio, distanza dalla sorgente e tempo di volo

    static double eq_2(double a, double b, double c) {
        double dis = b*b - a*c;
        
        // Controllo di sicurezza per evitare radici di numeri negativi
        if (dis < 0) {
            std::cerr << "Errore nella funzione eq_2: discriminante negativo\n";
            return 0.0; 
        }

        return (-b + std::sqrt(dis)) / a;
    }
};

#endif // __Pmt__