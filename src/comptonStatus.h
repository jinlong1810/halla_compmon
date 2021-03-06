/*
//	comptonStatus Class
//      keeps track of helicity, mps number, etc. 
//     usses EPICS and auxiliary VME data
//    working with delayed helicity info, etc. should go here
*/

#include <stdlib.h>
#include <TROOT.h>
#include "TTree.h"
#include "TMath.h"
#include "TBranch.h"
#include "TDirectory.h"
#include "TFile.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "THaEpics.h"
#include "THaCodaFile.h"
#include "textParams.h"
#include "helicityTracker.h"
#include "bankstructure.h"
#include "fadcdata.h"
#include "vmeauxdata.h"

#ifndef comptonStatus_h
#define comptonStatus_h

#define HELPLUS 1
#define HELMINUS 0

enum laserStateFlag_t {LASER_RIGHT, LASER_LEFT, LASER_RIGHTOFF, LASER_LEFTOFF,
		       LASER_UNKNOWN};
enum combinedSpinFlag_t {SPIN_LN,SPIN_LP,SPIN_RN,SPIN_RP,
			 SPIN_LNOFF,SPIN_LPOFF,SPIN_RNOFF,SPIN_RPOFF,SPIN_UNKNOWN};
enum beamStateFlag_t {BEAM_OFF, BEAM_ON, BEAM_UNKNOWN};

class comptonStatus {
 public:
  comptonStatus();
  comptonStatus(textParams* theParamsIn);
  int DefineStatusBranches(TTree *mytree);  //add-ons to other trees
  int DefineTree();
  int newRun();  //called at start of run to initizlize status variables
  bool newMPS(int codaEventNumber,fadcdata* theFADCData, vmeauxdata* theVMEauxdata); //called at new MPS
  bool SetLaserState();     //uses laserOn bit, laser power reading, and history to determine state
  int UnpackEpics(THaEpics *epics, uint32_t* codadata);
  int EpicsDebugDump();
  //
  // data extraction functions
  void DebugDump(int mode);
  int GetCountMPS(){ return countMPS;};
  int GetLaserState(){ return currentLaserState;};
  TString DecodeLaserState(int laserState);
  //
  // Stuff from IP Scaler  (integrated over helicity period scaler)
  float GetCalibratedBCM(){ return calbcm;};
  float GetRawBCM(){ return rawBCMFloat;};   //raw BCM IP scaler
  float GetCalibratedCavityPower(){return cavPowerCalibrated;};
  float GetRawcavityPower(){return rawCavPowFloat;};
  float GetBPMSummed(){ return bpmsum;};  //sum of positon monitor info
  float GetEpicsBCMAverage(){ return epics_hacbmf;};
  int GetBeamState(){return beamState;};
  TString DecodeBeamState(int beamState);
  float ComputeBPMPosition(float x_pos, float x_neg, float x_pos_ped,
      float x_neg_ped, float alpha, float sensitivity, float freq_conversion);
  void ComputeBPMPositionLab(float rotX, float rotY, float sintheta,
      float costheta, float xoff, float yoff, float &xlab, float &ylab);

  //helicity stuff
  int GetHelicityState();  //returns -1 if unknown, otherwise 0 or 1 
  int GetCurrentHelicityBit(){return statusHW->currentHelicityBit;};
  int GetCombinedSpinState() {return currentSpinState;};  //encoded helicitdy and LaserState info
  int GetCountQuad(){return statusHW->countQuartets;}; //return quad count number
  int GetIndexQuartet() {return statusHW->indexQuartet;}; //return number (0 to 3, -1 for undefined);
  bool IsHelicityValid(){return statusHW->helicityValid;};
  bool IsHelicityPredictionValid(){return statusHW->helicityPredictionValid;};
  bool IsHelicityBitInAgreement(){return statusHW->helicityBitInAgreement;};
  bool IsHelicitySynchValid(){return statusHW->helicitySynchValid;};

  //******************************
 private:
  //
  bool getEpicsValue(THaEpics *epics, const char* tag, float* pReturn, int vergose=0);
  void SetEpicsDefaults();
  //
  helicityTracker* theHelicityTracker;
  helicityStatus* statusHW;  //pointer to helicity window status info.
  TTree* runWiseTree; //info like run number
  bool runWiseTreeFilled; //use to fill just once after first event
  int helicityState;
  int currentHelicityBit;  //actual helicity Bit for current MPS (not helicitdyi if running delayed)
  textParams* theParams;
  int runnum;
  int countMPS;      
  int countMPSsinceEPICS;

  int countEpics; //count number of EPICS events encoutnered
  
  //
  //PARAMETERS updated at start of Run (from textParams* theParams)
  //helcity tracking and predicting
  //helicity parameters filled in at newRun
  int helicityStructure; //#of MPS in each set  (4 for "quads", 8 for Octs)
  int helicityShiftBits;  //# bits in helicity generating shift register
  int helicityDelay;  //filled in at start of run using comptomParams
  int helicityPredictorBitFlip;
  int helicityRegisterType; 
  int helicityBitForPositive; //==1 if bit=1 for positive helicity,etc.
  int run_number;
  float ped_value;
  float cavPowerCalibration;
  float cavPowerPedestal;
  float BCMCalibration;
  float BCMPedestal;
  float BPMSumCalibration;
  float BPMSumPedestal;
  // BPM calibrations and the like
  float BPM2Axm_pedestal;
  float BPM2Axp_pedestal;
  float BPM2Aym_pedestal;
  float BPM2Ayp_pedestal;
  float BPM2Bxm_pedestal;
  float BPM2Bxp_pedestal;
  float BPM2Bym_pedestal;
  float BPM2Byp_pedestal;
  float BPM2A_alphax;
  float BPM2A_alphay;
  float BPM2B_alphax;
  float BPM2B_alphay;
  float BPM2A_xoff;
  float BPM2A_yoff;
  float BPM2B_xoff;
  float BPM2B_yoff;
  float BPM2A_sensitivity;
  float BPM2B_sensitivity;
  float BPM2A_angle;
  float BPM2B_angle;
  //
  int useBPMSumCuts;  //==1 to use sumed BPMs instead of BCM for beam state
  float BPMSum_OnMin;
  float BPMSum_OffMax;
  float BCM_OnMin;
  float BCM_OffMax;
  float cavityPowerOnMin;
  float cavityPowerOffMax;
  float clockRateIP;   //clock rate used for IP scaler normalization
  //End of parameters
  int countLaserCycles;  //count number complete laser cycles
  int subcountMPSLaserCycles; //count MPS within current laser cycle


  bool laserStateValid;
  laserStateFlag_t laserStateFlag;  //enum labels of laser modes
  int laserStateFound[6];  //tracks which laser states encountered within laser cycle
  laserStateFlag_t previousLaserState;
  laserStateFlag_t currentLaserState;
  combinedSpinFlag_t currentSpinState; //combined helicity and laser polarization 
  laserStateFlag_t knownLaserState;
  laserStateFlag_t lastKnownLaserState;
  int laser_good;
  int laser_on;
  int laser_count;
  int laser_triplet;
  int first_laser_cycle;
  bool good_laser_triplet;
  bool last_good_triplet;
  int first_laser_cycle_arr[3];
  unsigned int laserstate_clock_start;
  float laserstate_clock;  //time into laser state
  int rtcavpow;
  float cavPowerCalibrated;

  // additional status variables for Root Tree
  int mpsCoda;    //mps counter from CODA file
  int mpsSignal;  //state of mpsSignal from TIR
  int mpsScaler;  //mps scaler readout
  int numTriggers; //number of calorimeter triggers in mps

  //Beam and Laser Status Info

  int old_clockscaler;  //last MPS RUN scaler clock
  int clock_diff;  //RUN scaler since lst MPS
  //stuff from IP scaler
  int clockscaler;  //from RUN scaler
  int clockIP;    //from IP scaler

  float rawCavPowFloat;
  float bpmsum;   //backup beam current info (sum BPM info)
  float rawBCMFloat;

  float calbcm;
  float bcmsum; //backup beam current info (sum BPM info)
  float ip_s1power;
  float ip_s2power;
  float ip_bpmAx;
  float ip_bpmAy;
  float ip_bpmBx;
  float ip_bpmBy;
  float bpmAx;
  float bpmAy;
  float bpmBx;
  float bpmBy;

  //Accumulator Info
  int ithr_near;
  int ithr_far;
    unsigned int mps;

    unsigned int clockscalerBCMFloat;

    int n4before;
    int n4after;
    int n5before;
    int n5after;
    int dithering;
    int dac;
    int rampdelay;
    int inttime;


    //unkown leftovers
    float epbcmu3;
    int buflen;
    unsigned int l1a;

    int beam_trip;
    int beam_burp;
    int eppol_burp;
    int HV_trip;
    int rate_fluct;
    int epics_dead;
    int read_helicity;
    int cavpow_burp;
    int trip_mps;
    float future_bcm;
    int HVtrip_mps;
    float future_HV;
    int rate_burp;
    int rate_trip;
    int rate_cut;
    int rate_fluct_mps;
    int rate_trip_mps;
    int epics_mps;
    float future_finger;
    float future_rate;
    int helicity;
    int next_helicity;
    Bool_t helvalid;

    int bad_rtcavpow;
    int rtbcmu3;
    int lastcavpol;
    float epcavpow;
    float eppoldir;
    float eppolpct;

    unsigned int bcmscaler;
    beamStateFlag_t beamState;
    beamStateFlag_t beamStatePrev;   //beamOn boolean for previous MPS
    float lasteppol;
    //EPICS data  (not everything implemented)
    int epics_evnum;
    float epics_PMTRate;
    float epics_PMTRateHigh;
    float epics_crystalHV;
    float epics_crystalCurrent;
    float epics_horizontalFingerHV;
    float epics_horizontalFingerCurrent;
    float epics_verticalFingerHV;
    float epics_verticalFingerCurrent;
    float epics_tablePosX;
    float epics_tablePosY;
    float epics_cavpow;
    float epics_hacbmf;
    float epics_cavpolpercent;
    float epics_s1;
    float epics_s2;
    float epics_cavpoldir;
    int epics_ihwp_in;
    float epics_aPosX;
    float epics_aPosY;
    float epics_bPosX;
    float epics_bPosY;

    float epics_coda_deadtime;	// ratio of mps to l1a
    int epics_wein_right;
    float rtcavpol;
    float epbeameng;
    float eptransmit;


    // test0 thru test 11?
    int test0;
    int test1;
    int test2;
    int test3;
    int test4;
    int test5;
    int test6;
    int test7;
    int test8;
    int test9;
    int test10;
    int test11;
    int test12;
    int test13;
    int test14;
    int test15;


    /* //UNKNOWN from original fadcanal.h */

    /* unsigned int triggerscaler; */
    /* unsigned int verticalfingerscaler; */
    /* unsigned int horizontalfingerscaler; */
    /* unsigned int old_bcmscaler; */
    /* unsigned int old_clockscaler; */
    /* unsigned int old_triggerscaler; */
    /* unsigned int old_l1a; */
    /* int old_mps;   //allow -1 for initial value */

};


#endif


