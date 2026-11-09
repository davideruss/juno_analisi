#pragma once
#include <algorithm>
#include <cmath>
#include <fstream>
#include <stdexcept>
#include <vector>
// The angular coordinate is dot(vertex,PMT)/(|vertex|*|PMT|), not the ray angle.
struct CorrectionMap {
 std::vector<double> radii,cosines,values;
 explicit CorrectionMap(const char*path){
  std::ifstream f(path);int nr=0,nc=0;f>>nr>>nc;
  if(!f||nr<2||nc<2||nr>10000||nc>10000)throw std::runtime_error("Invalid correction dimensions");
  radii.resize(nr);cosines.resize(nc);values.resize(nr*nc);
  for(auto* v:{&radii,&cosines,&values})for(auto&x:*v)if(!(f>>x)||!std::isfinite(x))throw std::runtime_error("Invalid correction data");
  for(auto*v:{&radii,&cosines})for(size_t i=1;i<v->size();++i)if((*v)[i]<=(*v)[i-1])throw std::runtime_error("Unsorted correction axes");
  if(radii.front()!=0||cosines.front()<=0||cosines.back()>1)throw std::runtime_error("Invalid correction axes");
 }
 double angular(size_t i,double c)const{
  if(i==0||c<=0)return 0; // isotropic, continuous origin
  auto offset=i*cosines.size();
  if(c<=cosines.front())return values[offset]*c/cosines.front();
  if(c>=cosines.back())return values[offset+cosines.size()-1];
  auto j=std::upper_bound(cosines.begin(),cosines.end(),c)-cosines.begin()-1;
  double u=(c-cosines[j])/(cosines[j+1]-cosines[j]);
  return (1-u)*values[offset+j]+u*values[offset+j+1];
 }
 double at(double radius,double cosine)const{
  if(!std::isfinite(radius)||!std::isfinite(cosine))throw std::runtime_error("Non-finite map coordinate");
  if(radius<=0||cosine<=0)return 0;
  if(radius>=radii.back())return angular(radii.size()-1,cosine);
  auto i=std::upper_bound(radii.begin(),radii.end(),radius)-radii.begin()-1;
  double u=(radius-radii[i])/(radii[i+1]-radii[i]);
  return (1-u)*angular(i,cosine)+u*angular(i+1,cosine);
 }
 double for_pmt(double x,double y,double z,double px,double py,double pz)const{
  double radius=std::hypot(std::hypot(x,y),z),norm=std::hypot(std::hypot(px,py),pz);
  return radius>0&&norm>0?at(radius,std::clamp((x*px+y*py+z*pz)/(radius*norm),-1.,1.)):0;
 }
};
