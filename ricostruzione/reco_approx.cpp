// Build: g++ -O3 -std=c++17 -Wall -Wextra reco_approx.cpp -o reco_approx.exe $(root-config --cflags --libs) -lMinuit2
#include <TFile.h>
#include <TTree.h>
#include <TParameter.h>
#include <TNamed.h>
#include <Math/Factory.h>
#include <Math/Functor.h>
#include <Math/Minimizer.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
using namespace std;
constexpr double invv=1.60/299.792458, logfloor=-20.72326583694641;
struct PDF {
 int64_t size;double low,step;vector<double> value,slope;
 explicit PDF(const char*path){ifstream f(path,ios::binary);f.read((char*)&size,8);f.read((char*)&low,8);f.read((char*)&step,8);if(!f||size<100||size>10000000||step<=0)throw runtime_error("Invalid PDF header");value.resize(10*size);slope.resize(10*size);f.read((char*)value.data(),value.size()*8);f.read((char*)slope.data(),slope.size()*8);if(!f)throw runtime_error("Invalid PDF table");}
 double logp(double t,int k)const{
  double pos=(t-low)/step;if(pos<0||pos>=size-1)return logfloor;
  auto j=static_cast<int64_t>(pos);double u=pos-j,u2=u*u,u3=u2*u;auto b=k*size+j;
  double lp=(2*u3-3*u2+1)*value[b]+(u3-2*u2+u)*step*slope[b]+(-2*u3+3*u2)*value[b+1]+(u3-u2)*step*slope[b+1];
  return max(logfloor,lp);
 }
};
struct PMT{double x,y,z;bool ham;};
struct Hit{int pmt,k;double time,q;};
struct Event{vector<Hit> hits;double stored_t0=0;bool has_t0=false;};
struct Result{array<double,4> p{};double nll=INFINITY,edm=INFINITY;int status=-1,ncalls=0;bool boundary=false;};
vector<PMT> read_geometry(const char*path){ifstream f(path);vector<PMT> out;int id;double x,y,z;string type;while(f>>id>>x>>y>>z>>type){if(id==numeric_limits<int>::min()||abs(id)!=int(out.size()))throw runtime_error("Geometry must be indexed by consecutive absolute IDs");out.push_back({x,y,z,type=="Hamamatsu"});}if(out.empty()||!f.eof())throw runtime_error("Invalid geometry");return out;}
struct Likelihood{
 const Event&e;const vector<PMT>&pmts;const PDF&pdf;
 double operator()(const double*p)const{double sum=0;for(const auto&h:e.hits){const auto&m=pmts[h.pmt];double x=p[0]-m.x,y=p[1]-m.y,z=p[2]-m.z;double r=h.time-invv*sqrt(x*x+y*y+z*z)-p[3];sum-=pdf.logp(r,h.k);}return sum;}
};
Result fit(const Likelihood&nll,array<double,4> start){
 ROOT::Math::Functor fun([&](const double*p){return nll(p);},4);
 auto minim=[&](const char*method,array<double,4> seed,double tol,int calls){
  unique_ptr<ROOT::Math::Minimizer> m(ROOT::Math::Factory::CreateMinimizer("Minuit2",method));if(!m)throw runtime_error("Minuit2 unavailable");
  m->SetFunction(fun);m->SetErrorDef(.5);m->SetPrintLevel(-1);m->SetStrategy(1);m->SetTolerance(tol);m->SetMaxFunctionCalls(calls);m->SetMaxIterations(calls);m->SetPrecision(1e-12);
  bool simplex=string(method)=="Simplex";
  for(int j=0;j<4;++j){double bound=j==3?500:20000;double step=j==3?(simplex?10:1):(simplex?1000:100);m->SetLimitedVariable(j, j==0?"x":j==1?"y":j==2?"z":"t0",clamp(seed[j],-bound+.01,bound-.01),step,-bound,bound);}
  m->Minimize();Result r;copy(m->X(),m->X()+4,r.p.begin());r.nll=m->MinValue();r.status=m->Status();r.edm=m->Edm();r.ncalls=m->NCalls();for(int j=0;j<4;++j)if(abs(r.p[j])>(j==3?499.9:19999.9))r.boundary=true;return r;
 };
 auto coarse=minim("Simplex",start,.1,5000);auto fine=minim("Migrad",coarse.p,.01,10000);int calls=coarse.ncalls+fine.ncalls;
 if(fine.status>1||!isfinite(fine.nll)||fine.edm>.001){auto retry=minim("Migrad",fine.p,.001,30000);calls+=retry.ncalls;if(retry.nll<=fine.nll||!isfinite(fine.nll))fine=retry;}
 if(coarse.nll+1e-6<fine.nll){fine=coarse;fine.status=10;}
 fine.ncalls=calls;return fine;
}
int main(int argc,char**argv){try{
 if(argc<5||argc>7)throw runtime_error("Usage: reco_approx.exe input.root geometry pdf_table.bin output.root [max_events=0] [second_start=0]");
 auto begin=chrono::steady_clock::now();auto pmts=read_geometry(argv[2]);PDF pdf(argv[3]);int limit=argc>5?stoi(argv[5]):0;bool second=argc>6?stoi(argv[6])!=0:false;
 TFile input(argv[1],"READ");if(input.IsZombie())throw runtime_error("Cannot open input");auto*t=input.Get<TTree>("pmtRes");auto*zs=input.Get<TParameter<double>>("source_z_mm");auto*ni=input.Get<TParameter<double>>("refractive_index");if(!t||!zs||!ni||abs(ni->GetVal()-1.60)>1e-8)throw runtime_error("Missing input tree or invalid metadata");double truth_z=zs->GetVal();
 vector<double>*res=nullptr,*tzero=nullptr,*q=nullptr,*eid=nullptr;int pid;
 auto bind=[&](const char*name,auto*ptr){if(t->SetBranchAddress(name,ptr)<0)throw runtime_error(string("Invalid branch ")+name);};bind("pmt_id",&pid);bind("residual_times",&res);bind("t0",&tzero);bind("charge",&q);bind("evtID",&eid);
 map<Long64_t,Event> events;set<int> seen_pmts;
 for(Long64_t row=0;row<t->GetEntries();++row){
  if(t->GetEntry(row)<=0||!res||!tzero||!q||!eid||res->size()!=q->size()||res->size()!=eid->size()||res->size()!=tzero->size())throw runtime_error("Invalid vectors/read");
  if(pid<0||size_t(pid)>=pmts.size()||!pmts[pid].ham||!seen_pmts.insert(pid).second)throw runtime_error("Invalid/duplicate PMT");
  auto&m=pmts[pid];double nominal_tof=invv*sqrt(m.x*m.x+m.y*m.y+(m.z-truth_z)*(m.z-truth_z));set<Long64_t> seen;
  for(size_t j=0;j<res->size();++j){double qq=(*q)[j],e=(*eid)[j],tt=(*tzero)[j],r=(*res)[j];if(!isfinite(qq)||qq<=0||!isfinite(e)||e<0||e>1e15||floor(e)!=e||!isfinite(tt)||abs(tt)>1e8||!isfinite(r))throw runtime_error("Invalid hit");auto id=static_cast<Long64_t>(e);if(!seen.insert(id).second)throw runtime_error("Duplicate event/PMT");auto&ev=events[id];if(ev.has_t0&&abs(ev.stored_t0-tt)>1e-8)throw runtime_error("Inconsistent t0");ev.has_t0=true;ev.stored_t0=tt;
   ev.hits.push_back({pid,qq>=10?9:max(0,int(floor(qq))-1),r+nominal_tof,qq});
  }
 }
 t->ResetBranchAddresses();delete res;delete tzero;delete q;delete eid;input.Close();
 vector<Long64_t> ids;for(auto&[id,e]:events)ids.push_back(id);const auto total_events=ids.size();
 if(limit>0&&ids.size()>size_t(limit)){mt19937 rng(20261001);shuffle(ids.begin(),ids.end(),rng);ids.resize(limit);sort(ids.begin(),ids.end());}
 auto read_done=chrono::steady_clock::now();TFile output(argv[4],"RECREATE");if(output.IsZombie())throw runtime_error("Cannot create output");
 Long64_t event_id=0;int nhit=0;double x,y,z,t0,stored_t0,fmin,edm,nll_stored_nominal,nll_best_nominal,seed_x,seed_y,seed_z;int status,calls;bool boundary,success;double alt_gain,alt_distance;int alt_status;
 {
 TTree tree("reco","Approximate JVertex WITHOUT correction; x,y,z,t0 free");
 tree.Branch("evtID",&event_id);tree.Branch("nhit",&nhit);tree.Branch("x",&x);tree.Branch("y",&y);tree.Branch("z",&z);tree.Branch("t0",&t0);tree.Branch("stored_t0",&stored_t0);tree.Branch("nll",&fmin);tree.Branch("edm",&edm);tree.Branch("status",&status);tree.Branch("calls",&calls);tree.Branch("boundary",&boundary);tree.Branch("success",&success);tree.Branch("nll_stored_nominal",&nll_stored_nominal);tree.Branch("nll_at_nominal_fitted_t0",&nll_best_nominal);tree.Branch("seed_x",&seed_x);tree.Branch("seed_y",&seed_y);tree.Branch("seed_z",&seed_z);tree.Branch("second_start_nll_gain",&alt_gain);tree.Branch("second_start_distance_mm",&alt_distance);tree.Branch("second_start_status",&alt_status);
 size_t done=0,good=0;
 for(auto id:ids){event_id=id;auto&e=events.at(id);nhit=e.hits.size();Likelihood nll{e,pmts,pdf};array<double,4> seed{};double charge=0;
  for(auto&h:e.hits){auto&m=pmts[h.pmt];seed[0]+=h.q*m.x;seed[1]+=h.q*m.y;seed[2]+=h.q*m.z;charge+=h.q;}
  for(int j=0;j<3;++j){seed[j]/=charge;}
  seed_x=seed[0];seed_y=seed[1];seed_z=seed[2];
  // Seed uses charge barycenter and t0=0, never nominal source position or stored t0.
  auto r=fit(nll,seed);alt_gain=NAN;alt_distance=NAN;alt_status=-1;
  if(second){auto other=fit(nll,{0,0,0,0});alt_gain=r.nll-other.nll;alt_distance=hypot(hypot(r.p[0]-other.p[0],r.p[1]-other.p[1]),r.p[2]-other.p[2]);alt_status=other.status;if(other.nll<r.nll)r=other;}
  x=r.p[0];y=r.p[1];z=r.p[2];t0=r.p[3];fmin=r.nll;edm=r.edm;status=r.status;calls=r.ncalls;boundary=r.boundary;stored_t0=e.stored_t0;
  success=(status==0||status==1)&&isfinite(fmin)&&edm<=.001&&!boundary&&nhit>=20;
  array<double,4> at_truth={0,0,truth_z,stored_t0};nll_stored_nominal=nll(at_truth.data());at_truth[3]=t0;nll_best_nominal=nll(at_truth.data());
  if(tree.Fill()<0)throw runtime_error("Output fill failed");
  ++done;if(success)++good;
  if(done%100==0||done==ids.size())cout<<argv[1]<<": "<<done<<'/'<<ids.size()<<", success "<<good<<", fit seconds "<<chrono::duration<double>(chrono::steady_clock::now()-read_done).count()<<'\n'<<flush;
 }
 output.cd();if(tree.Write()<=0)throw runtime_error("Output write failed");
 }
 TParameter<double>("source_z_mm",truth_z).Write();TParameter<double>("refractive_index",1.60).Write();TParameter<Long64_t>("input_events",total_events).Write();TParameter<Long64_t>("fitted_events",ids.size()).Write();TNamed("model","Approximate screenshot PDFs; no correction; straight ToF; free x,y,z,t0; barycenter of selected Hamamatsu charge; t0 seed zero").Write();output.Close();
 cout<<"Read "<<chrono::duration<double>(read_done-begin).count()<<" s; total "<<chrono::duration<double>(chrono::steady_clock::now()-begin).count()<<" s\n";
 }catch(const exception&e){cerr<<e.what()<<'\n';return 1;}return 0;}
