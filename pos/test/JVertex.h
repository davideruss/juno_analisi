#ifndef JVERTEX_H
#define JVERTEX_H

#include "RecTools/IRecTool.h"
#include "SniperKernel/ToolBase.h"
#include "SniperKernel/SniperPtr.h"
#include "Geometry/IPMTParamSvc.h"
#include "PMTCalibSvc/IPMTCalibSvc.hh"
#include "QCtrRecAlg/IWaterPhaseVertexSvc.h"
#include "QCtrRecAlg/IMixedPhaseVertexSvc.h"
#include "RootWriter/RootWriter.h"

#include <vector>
#include <numeric>

#include <TFile.h>
#include <TTree.h>
#include <TString.h>
#include <TSystem.h>
#include <TStopwatch.h>

#include "Math/MinimizerOptions.h"
#include "Math/Minimizer.h"
#include "Math/Factory.h"
#include "Math/Functor.h"

class JVertex: public ToolBase, public IRecTool{
    public:
        JVertex(const std::string &); 
        virtual ~JVertex();

		bool configure() override;
    	void rec(HitPMTVector &hitpmtvector, PMTTable &pmttable, RecInfo &); 

    private:
		double calculateTOF(double source_x, double source_y, double source_z, double pmt_x, double pmt_y, double pmt_z) const;
		
		void clearEventVectors();

        double light_speed = 299.792458; // mm/ns
        double nEff = 1.54;
		double nWater = 1.33;
		double R_interface = 17820.0; 

        double SourcePos;

		std::string PDFPath, LS_levels_file, Phase, PmtType, pathToCorr;

		// double JVertex::someFunction(double a, double b);

		double MinPE = 1300;
    	double MaxPE = 1900;
    	// double MaxSourceDistance = 1000.0;
		double MaxRecoRadius=17700;

		int m_evtID = 0;
		double m_npe=0.;
		// double m_recoX = 0.;
		// double m_recoY = 0.;
		// double m_recoZ = 0.;

		TTree* tofTree = nullptr;

    	std::vector<int> m_pmtID;
    	std::vector<double> m_pmtX;
    	std::vector<double> m_pmtY;
    	std::vector<double> m_pmtZ;
    	std::vector<int> m_pmtType;

		std::vector<double> m_tofSource;
		std::vector<double> m_firstHitTime;
		std::vector<double> m_charge;
};

#endif
