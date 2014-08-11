#ifndef FUZZYTOOLS_H
#define FUZZYTOOLS_H

#include <vector>
#include <set>
#include <math.h>
#include <string>

#include "fastjet/ClusterSequence.hh"
#include "fastjet/PseudoJet.hh"

#include "Pythia8/Pythia.h"

#include "myFastJetBase.h"
#include "ROOTConf.h"

#ifdef WITHROOT
#include "TMatrix.h"
#endif

typedef std::vector<fastjet::PseudoJet> vecPseudoJet;

// declare helper functions to CentralMoments so they are accessible
// to any client code
double totalMass(vecPseudoJet particles, vector<unsigned int> indices);
double totalpT(vecPseudoJet particles, vector<unsigned int> indices);

class MatTwo {
public:
    double xx;
    double xy;
    double yx;
    double yy;

    MatTwo(double xx_in, double xy_in, double yx_in, double yy_in) {
        xx = xx_in;
        xy = xy_in;
        yx = yx_in;
        yy = yy_in;
    }

    double determinant() const {
        return (xx * yy) - (xy * yx);
    }
};

/* TODO:
 * There is no need to pass around constants giving cluster size etc,
 * as they can be extracted locally out of the C++ vector, which gives
 * O(1) size operation. At the moment though, everything is working,
 * so I won't bother to change this.
 *
 * Pass debug flags and print useful information.
 */
class FuzzyTools {
 public:
    enum Kernel {
        NOKERNEL,
        GAUSSIAN,
        TRUNCGAUSSIAN,
        UNIFORM
    };

    enum Mode {
        NOCLUSTERINGMODE,
        RECOMBINATION,
        FIXED
    };

 private:
    int m_test;
    double alpha;
    double R;
    int max_iters;

    string directory_prefix;

    bool learn_weights;
    bool learn_shape;

    double merge_dist;
    double min_weight;
    double min_sigma;

    int cluster_count;
    Mode clustering_mode;
    Kernel kernel_type;
    vecPseudoJet seeds;

 public:
    FuzzyTools();

    // methods
    vecPseudoJet Initialize(vecPseudoJet particles,
                            int k,
                            vecPseudoJet jets);

    void SetLearnWeights(bool b) {
        learn_weights = b;
    }

    void SetR(double r) {
        R = r;
    }

    void SetKernelType(Kernel k) {
        kernel_type = k;
    }

    void SetMergeDistance(double d) {
        merge_dist = d;
    }

    void SetMinimumWeight(double m) {
        min_weight = m;
    }

    void SetLearnShape(bool s) {
        learn_shape = s;
    }

    void SetMinimumSigma(double s) {
        min_sigma = s;
    }

    void SetPrefix(string p) {
        directory_prefix = p;
    }

    vector<double> CentralMoments(vecPseudoJet const& particles,
                                  vector<vector<double> > const& weights,
                                  unsigned int cluster_id,
                                  unsigned int moment_count,
                                  double (*f) (vecPseudoJet, vector<unsigned int>));

    void SetClusteringMode(Mode m) {
        clustering_mode = m;
    }

    void SetAlpha(double a) {
        alpha = a;
    }

    set<unsigned int> ClustersForRemovalGaussian(vecPseudoJet const& mGMM_jets,
                                                 vector<MatTwo> const& mGMM_jets_params,
                                                 vector<double> const& mGMM_weights);

    set<unsigned int> ClustersForRemovalUniform(vecPseudoJet const& mUMM_jets,
                                                vector<double> const& mUMM_weights);

    void SetSeeds(vecPseudoJet s) {
        seeds = s;
    }

    vector<vector<double> > InitWeights(vecPseudoJet const& particles,int k);

    double MDist(double x1, double x2, double mu1, double mu2, MatTwo const& sigma);

    double doGaus(double x1, double x2, double mu1, double mu2, MatTwo const& sigma);

    double doTruncGaus(double x1, double x2, double mu1, double mu2, MatTwo const& sigma);

    vector<MatTwo> Initializeparams(vecPseudoJet const& particles,
                                     int k);

    void ComputeWeightsGaussian(vecPseudoJet const& particles,
                                vector<vector<double> >* weights,
                                int cluster_count,
                                vecPseudoJet const& mGMM_jets,
                                vector<MatTwo> const& mGMM_jets_params,
                                vector<double> const& mGMM_weights);

    vecPseudoJet UpdateJetsGaussian(vecPseudoJet const& particles,
                                    vector<vector<double> > const& weights,
                                    int cluster_count,
                                    vector<MatTwo>* mGMM_jets_params,
                                    vector<double>* mGMM_weights);

    vecPseudoJet ClusterFuzzyGaussian(vecPseudoJet const& particles,
                                      vector<vector<double> >* weights,
                                      vector<MatTwo>* mGMM_jets_params_out,
                                      vector<double>* mGMM_weights_out);

    vecPseudoJet UpdateJetsGaussianC(vecPseudoJet const& particles,
                                     vector<vector<double> > const& weights,
                                     int cluster_count,
                                     vector<MatTwo>* mGMMc_jets_params,
                                     vector<double>* mGMMc_weights);

    vecPseudoJet ClusterFuzzyGaussianC(vecPseudoJet const& particles,
                                       vector<vector<double> >* weights,
                                       vector<MatTwo>* mGMMc_jets_params_out,
                                       vector<double>* mGMMc_weights_out);

    void ComputeWeightsUniform(vecPseudoJet const& particles,
                               vector<vector<double> >* weights,
                               int cluster_count,
                               vecPseudoJet const& mUMM_jets,
                               vector<double> const& mUMM_weights);

    vecPseudoJet UpdateJetsUniform(vecPseudoJet const& particles,
                                   vector<vector<double> > const& weights,
                                   int cluster_count,
                                   vector<double>* mUMM_weights);

    vecPseudoJet ClusterFuzzyUniform(vecPseudoJet const& particles,
                                     vector<vector<double> >* weights,
                                     vector<double>* mUMM_weights_out);

    void ComputeWeightsTruncGaus(vecPseudoJet const& particles,
                                 vector<vector<double> >* weights,
                                 int cluster_count,
                                 vecPseudoJet const& mTGMM_jets,
                                 vector<MatTwo> const& mTGMM_jets_params,
                                 vector<double> const& mTGMM_weights);

    vecPseudoJet UpdateJetsTruncGaus(vecPseudoJet const& particles,
                                     vector<vector<double> > const& weights,
                                     int cluster_count,
                                     vector<MatTwo>* mTGMM_jets_params,
                                     vector<double>* mTGMM_weights);

    vecPseudoJet ClusterFuzzyTruncGaus(vecPseudoJet const& particles,
                                       vector<vector<double> >* weights,
                                       vector<MatTwo>* mTGMM_jets_params_out,
                                       vector<double>* mTGMM_weights);

    void EventDisplay(vecPseudoJet const& particles,
                      vecPseudoJet const& ca_jets,
                      vecPseudoJet const& tops,
                      vecPseudoJet const& mGMM_jets,
                      vector<vector<double> > const& weights,
                      int which,
                      vector<MatTwo> const& mGMM_jets_params,
                      std::string const& out,
                      int iter);

    void NewEventDisplay(vecPseudoJet const& particles,
                         vecPseudoJet const& ca_jets,
                         vecPseudoJet const& tops,
                         vecPseudoJet const& mGMM_jets,
                         vector<vector<double> > const& weights,
                         int which,
                         vector<MatTwo> const& mGMM_jets_params,
                         vector<double> const& mGMM_weights,
                         std::string const& out,
                         int iter);

    void NewEventDisplayUniform(vecPseudoJet const& particles,
                                vecPseudoJet const& ca_jets,
                                vecPseudoJet const& tops,
                                vecPseudoJet const& mUMM_jets,
                                vector<vector<double> > const& weights,
                                int which,
                                vector<double> const& mUMM_weights,
                                std::string const& out,
                                int iter);

    void JetContributionDisplay(vecPseudoJet particles,
                                vector<vector<double> > weights,
                                int which,
                                int m_type,
                                std::string out,
                                int iter);

    double MLpT(vecPseudoJet particles,
                vector<vector<double> > weights,
                int jet_index,
                int k,
                int m_type);

    double MLlpTGaussian(vecPseudoJet const& particles,
                         fastjet::PseudoJet const& jet,
                         MatTwo const& jet_params,
                         double jet_weight,
                         int m_type);

    double MLlpTUniform(vecPseudoJet const& particles,
                        fastjet::PseudoJet const& jet,
                        double jet_weight,
                        int m_type);

    double MLlpTTruncGaus(vecPseudoJet const& particles,
                          fastjet::PseudoJet const& jet,
                          MatTwo const& jet_params,
                          double jet_weight,
                          int m_type);

    void Qjetmass(vecPseudoJet particles,
                  vector<vector<double> > weights,
                  int which,
                  std::string out);

    double SoftpT(vecPseudoJet const& particles,
                  vector<vector<double> > const& weights,
                  int jet_index,
                  int m_type);

};

#endif
