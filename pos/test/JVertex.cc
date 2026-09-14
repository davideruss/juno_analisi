#include "JVertex.h"
#include "SniperKernel/ToolFactory.h"
#include "Event/CalibPmtChannel.h"
#include "Identifier/CdID.h"
#include "DataPathHelper/Path.hh"

#include <TVector3.h>

#include <cmath>
#include <TMinuit.h>
#include <TGraph.h>
#include <TF1.h>
#include <fstream>
#include <string>
#include <iomanip>

DECLARE_TOOL(JVertex);

static bool g_applyTIRConstraint = false;
static std::vector<double> g_tofMinWithTIR;
static std::vector<double> g_deltaTofWithTIR;
static std::vector<int> g_isTir;

JVertex::JVertex(const std::string &name): ToolBase(name){
	declProp("JVertexMinPE", MinPE);
	declProp("JVertexMaxPE", MaxPE);
	declProp("JVertexNeff", nEff);
	declProp("JVertexPDFPath", PDFPath);
    declProp("JVertexLSlevelFile", LS_levels_file);
	declProp("JVertexPhase", Phase);
    declProp("JVertexPmtType", PmtType);
	declProp("JVertexNwater", nWater);
	declProp("JVertexRinterface", RInterface);
	declProp("JVSourcePos", SourcePos);
	declProp("JVertexMaxSourceDistance", MaxSourceDistance);
	declProp("JVertexMaxRecoRadius", MaxRecoRadius);
}

JVertex::~JVertex(){
}

double JVertex::calculateTOF(double source_x, double source_y, double source_z, double pmt_x, double pmt_y, double pmt_z) const {

	double dx = pmt_x- source_x;
	double dy = pmt_y- source_y;
	double dz = pmt_z- source_z;

	double dist = std::sqrt(dx*dx + dy*dy + dz*dz);
	
	double source_r2 = source_x*source_x + source_y*source_y + source_z*source_z;
	double b = (source_x*dx + source_y*dy + source_z*dz)/dist;

	double delta = b*b - (source_r2 - R_interface*R_interface);
	
	if(delta<0){
		LogError << "negative discriminant in the equation\n";
		delta=0.;
	}

	double dist_LS = -b + std::sqrt(delta);
	return (dist_LS*(nEff - nWater) + dist*nWater)/light_speed;
}

bool JVertex::configure(){
	SniperPtr<RootWriter> rwsvc(getParent(), "RootWriter");

	if (rwsvc.invalid()){
        LogError << "Failed to get RootWriter service." << std::endl;
    	return false;
	}

    tofTree = rwsvc->bookTree(*getParent(), "USER_OUTPUT/tofTree", "tofTree");

	tofTree->Branch("evtID", &m_evtID);
	
	tofTree->Branch("SourcePos", &SourcePos);
	tofTree->Branch("NPE", &m_npe);
	
	// tofTree->Branch("reco_x", &m_recoX);
    // tofTree->Branch("reco_y", &m_recoY);
    // tofTree->Branch("reco_z", &m_recoZ);
	
	tofTree->Branch("pmtid", &m_pmtID);
   	tofTree->Branch("pmtx", &m_pmtX);
   	tofTree->Branch("pmty", &m_pmtY);
   	tofTree->Branch("pmtz", &m_pmtZ);
   	tofTree->Branch("pmtType", &m_pmtType);
	
	tofTree->Branch("tof_source_ns", &m_tofSource);
    // tofTree->Branch("tof_reco_ns", &m_tofReco);
    // tofTree->Branch("delta_ns", &m_deltaTof);

	tofTree->Branch("first_hit_time", &m_firstHitTime);
    tofTree->Branch("charge", &m_charge);
	
	return true;
}

void JVertex::rec(HitPMTVector &hitpmtvector, PMTTable &pmttable, RecInfo &recinfo){

	clearEventVectors();

    m_evtID = recinfo.evtID;
    m_npe = recinfo.totalcharge;
    // m_recoX = recinfo.vtx.X();
    // m_recoY = recinfo.vtx.Y();
    // m_recoZ = recinfo.vtx.Z();

	if (m_npe < MinPE || m_npe > MaxPE) {
        return;
    }

	// const TVector3 reconstructedPosition(m_recoX, m_recoY, m_recoZ);
    // const TVector3 sourcePosition(0.0, 0.0, SourcePos);

	// if ((reconstructedPosition - sourcePosition).Mag() >= MaxSourceDistance) {
    //     return;
    // }
	
    // if (reconstructedPosition.Mag() >= MaxRecoRadius) {
    //     return;
    // }

	for (int j = 0; j < hitpmtvector.size(); j++){

		double x_pmt = hitpmtvector[j]->pos.X();
		double y_pmt = hitpmtvector[j]->pos.Y();
		double z_pmt = hitpmtvector[j]->pos.Z();
			
		double pmt_id = hitpmtvector[j]->pmtcalib->pmtId();
		Identifier id = Identifier(pmt_id);
		int pmtId = CdID::module(id);

		double first_hit_time = hitpmtvector[j]->fht;
		double charge = hitpmtvector[j]->totq;
		double tofSource = calculateTOF(0., 0., SourcePos, pmtX, pmtY, pmtZ);
			
		if (hitpmtvector[j]->type == 1)					LogInfo << "Il pmt " << pmtId << " è un Hamamatsu" << std::endl;
		if (hitpmtvector[j]->type == 2 || hitpmtvector[j]->type == 3)	LogInfo << "Il pmt " << pmtId << " è un MCP" << std::endl;

		m_pmtID.push_back(pmtId);
        m_pmtX.push_back(x_pmt);
        m_pmtY.push_back(y_pmt);
        m_pmtZ.push_back(z_pmt);
		m_pmtType.push_back(hitpmtvector[j]->type);
        m_firstHitTime.push_back(hitpmtvector[j]->fht);
        m_charge.push_back(hitpmtvector[j]->totq);
        m_tofSource.push_back(tofSource);
        // m_tofReco.push_back(tofReco);
        // m_deltaTof.push_back(tofSource - tofReco);

	}
	
	if (tofTree) {tofTree->Fill();}
	m_evtID++;
	
	return;
}

void JVertex::clearEventVectors()
{
    m_pmtID.clear();
    m_pmtX.clear();
    m_pmtY.clear();
    m_pmtZ.clear();
    m_pmtType.clear();

    m_tofSource.clear();
    // m_tofReco.clear();
    // m_deltaTof.clear();

    m_firstHitTime.clear();
    m_charge.clear();
}

/*double JVertex::someFunction(double a, double b)
{
	return a+b;
}*/

