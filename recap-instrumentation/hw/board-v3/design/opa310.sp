* OPA2310 - Rev. B
* Created by Jerry Madalvanos 6/24/2022
* Tweaked by Jacob Nogaj
* Created with Green-Williams-Lis Op Amp Macro-model Architecture
* Copyright 2022 by Texas Instruments Corporation
******************************************************
* MACRO-MODEL SIMULATED PARAMETERS:
******************************************************
* OPEN-LOOP GAIN AND PHASE VS. FREQUENCY  WITH RL, CL EFFECTS (Aol)
* UNITY GAIN BANDWIDTH (GBW)
* INPUT COMMON-MODE REJECTION RATIO VS. FREQUENCY (CMRR)
* POWER SUPPLY REJECTION RATIO VS. FREQUENCY (PSRR)
* DIFFERENTIAL INPUT IMPEDANCE (Zid)
* COMMON-MODE INPUT IMPEDANCE (Zic)
* OPEN-LOOP OUTPUT IMPEDANCE VS. FREQUENCY (Zo)
* OUTPUT CURRENT THROUGH THE SUPPLY (Iout)
* INPUT VOLTAGE NOISE DENSITY VS. FREQUENCY (en)
* OUTPUT VOLTAGE SWING vs. OUTPUT CURRENT (Vo)
* SHORT-CIRCUIT OUTPUT CURRENT (Isc)
* QUIESCENT CURRENT (Iq)
* SETTLING TIME VS. CAPACITIVE LOAD (ts)
* SLEW RATE (SR)
* SMALL SIGNAL OVERSHOOT VS. CAPACITIVE LOAD
* LARGE SIGNAL RESPONSE
* OVERLOAD RECOVERY TIME (tor)
* INPUT BIAS CURRENT (Ib)
* INPUT OFFSET CURRENT (Ios)
* INPUT OFFSET VOLTAGE (Vos)
* INPUT COMMON-MODE VOLTAGE RANGE (Vcm)
* INPUT/OUTPUT ESD CELLS (ESDin, ESDout)
************************************************************************************************************************
.SUBCKT OPA2310 OUT VCC IN+ IN- VEE
****************************************
* Models
.MODEL R_RES_1 RES (TCE=0 T_ABS=-273.15)
.MODEL S_VSWITCH_1 VSWITCH (RON=50 ROFF=1T VON=500M VOFF=450M)
.MODEL S_VSWITCH_2 VSWITCH (RON=10M ROFF=1G VON=10M VOFF=0)
.MODEL S_VSWITCH_3 VSWITCH (RON=1M ROFF=1G VON=900M VOFF=800M)
****************************************
* Components
C_CMN ESDN MID 500F 
C_CMP MID ESDP 500F 
C_DIFF ESDP ESDN 1.4P 
C_VCLP VCLP MID 100P 
C_VIMON VIMON MID 1N 
C_VOUT_S VOUT_S MID 1N 
C1 43 44 454.7F 
C1B 46 47 6.92P 
C2 31 MID 10F 
C4 30 29 15.92P 
C5 27 28 15.92P 
C6 39 41 40P 
C7 33 MID 1F 
C8 MID 26 1F 
C9 25 MID 1F 
C10 73 MID 1F 
C11 MID 74 1F 
C12 69 MID 1F 
C13 MID 70 1F 
C14 53 MID 1F 
C15 MID 54 1F 
C16 50 MID 1F 
C17 57 MID 1P 
C18 55 MID 1P 
C19 SW_OL MID 1P 
C20 CLAMP MID 80N 
C21 40 38 200N 
C22 36 37 1.3P 
C33 85 0 1F 
C34 83 0 100N 
C35 84 0 1F 
E2 78 MID CL_CLAMP MID 1
E3 77 MID OUT MID 1
E6 MID 0 83 0 1
G_1 29 MID VEE_B MID -500M
G_2 45 MID 46 MID -2.174
G_ADJUST 44 MID ESDP MID -7.621M
G2 87 22 45 MID -1M
G6 82 86 27 30 -1M
G8 VCC_CLP MID 26 MID -1M
G9 VEE_CLP MID 25 MID -1M
G10 61 MID 62 MID -1
G11 59 MID 60 MID -1
G12 VSENSE MID CLAMP MID -1M
G15 CLAW_CLAMP MID 31 MID -1M
G16 CL_CLAMP MID CLAW_CLAMP MID -1M
G36 VCC_B 0 VCC 0 -1
G37 VEE_B 0 VEE 0 -1
GVCCS1 47 MID 43 MID -1
GVCCS2 28 MID VCC_B MID -500M
GVCCS3 31 MID VSENSE MID -1U
GVCCS5 36 MID 41 MID -1.3
GVCCS6 40 MID CL_CLAMP 35 -90.91
GVCCS7 39 MID 38 MID -9.5
H2 58 MID V11 -1
H3 56 MID V12 1
H4 76 MID V4 1K
I_B 21 MID -3.5P
I_OS ESDN MID -4P
I_Q VCC VEE 165U
R_VCLP 78 VCLP R_RES_1 100 
R_VIMON 76 VIMON R_RES_1 100 
R_VOUT_S 77 VOUT_S R_RES_1 100 
R1 IN+ ESDP R_RES_1 10M 
R1B 47 46 R_RES_1 10K 
R2 IN- ESDN R_RES_1 10M 
R2B MID 46 R_RES_1 8.519K 
R3 MID ESDP R_RES_1 1T 
R4 ESDN MID R_RES_1 1T 
R5 MID 47 R_RES_1 1 
R6 MID 45 R_RES_1 1 
R7 35 42 R_RES_1 800K 
R8 22 87 R_RES_1 1K 
R9 86 87 R_RES_1 1M 
R10 34 ESDN R_RES_1 1M 
R11 42 MID 1 
R12 44 43 R_RES_1 100MEG 
R13 MID 43 R_RES_1 233.9K 
R14 MID 29 R_RES_1 1 
R15 29 30 R_RES_1 100MEG 
R16 30 MID R_RES_1 2K 
R17 MID 28 R_RES_1 1 
R18 28 27 R_RES_1 100MEG 
R19 MID 27 R_RES_1 2K 
R20 40 MID R_RES_1 1 
R21 86 82 R_RES_1 1K 
R22 39 MID R_RES_1 1 
R23 41 39 R_RES_1 10K 
R24 41 MID R_RES_1 25K 
R25 MID 80 R_RES_1 1G 
R26 81 MID R_RES_1 1G 
R27 MID 79 R_RES_1 1 
R28 33 79 R_RES_1 1M 
R29 23 VCC_B R_RES_1 1K 
R30 23 26 R_RES_1 1M 
R31 VCC_CLP MID R_RES_1 1K 
R32 VEE_B 24 R_RES_1 1K 
R33 25 24 R_RES_1 1M 
R34 MID VEE_CLP R_RES_1 1K 
R35 VCC_CLP MID R_RES_1 1G 
R36 71 MID R_RES_1 1 
R37 71 73 R_RES_1 1M 
R38 72 74 R_RES_1 1M 
R39 MID 72 R_RES_1 1 
R40 VEE_CLP MID R_RES_1 1G 
R41 63 MID R_RES_1 1G 
R42 65 MID R_RES_1 1 
R43 65 69 R_RES_1 1M 
R44 66 70 R_RES_1 1M 
R45 MID 66 R_RES_1 1 
R46 MID 64 R_RES_1 1G 
R47 61 MID R_RES_1 1 
R48 MID 59 R_RES_1 1 
R49 48 MID R_RES_1 1G 
R50 51 MID R_RES_1 1 
R51 51 53 R_RES_1 1M 
R52 52 54 R_RES_1 1M 
R53 MID 52 R_RES_1 1 
R54 49 MID R_RES_1 1G 
R55 VSENSE 50 R_RES_1 1M 
R56 58 57 R_RES_1 100 
R57 56 55 R_RES_1 100 
R58 75 MID R_RES_1 1 
R59 75 SW_OL R_RES_1 100 
R60 MID 32 R_RES_1 1MEG 
R61 MID CLAMP R_RES_1 1MEG 
R62 MID VSENSE R_RES_1 1K 
R63 36 MID R_RES_1 1 
R64 38 40 R_RES_1 10K 
R65 MID CLAW_CLAMP R_RES_1 1K 
R66 MID CL_CLAMP R_RES_1 1K 
R67 38 MID R_RES_1 1K 
R68 37 36 R_RES_1 10K 
R69 37 MID R_RES_1 1K 
R70 MID 31 R_RES_1 1MEG 
R81 MID 76 R_RES_1 1G 
R83 MID 77 R_RES_1 1G 
R107 VCC_B 0 R_RES_1 1 
R108 83 0 R_RES_1 1T 
R109 VEE_B 0 R_RES_1 1 
R110 VCC_B 85 R_RES_1 1M 
R111 85 83 R_RES_1 1MEG 
R112 83 84 R_RES_1 1MEG 
R113 84 VEE_B R_RES_1 1M 
RDUMMY MID 35 R_RES_1 80K 
RSRC MID 44 R_RES_1 1 
S1 40 38 SW_OL MID S_VSWITCH_3
S4 VEE ESDN VEE ESDN S_VSWITCH_1
S5 VEE ESDP VEE ESDP S_VSWITCH_1
S6 OUT VCC OUT VCC S_VSWITCH_1
S7 VEE OUT VEE OUT S_VSWITCH_1
SOR_SWN 68 CLAMP 68 CLAMP S_VSWITCH_2
SOR_SWP CLAMP 67 CLAMP 67 S_VSWITCH_2
V_GRN 49 MID -1.49K
V_GRP 48 MID 1.51K
V_ISCN 64 MID -200
V_ISCP 63 MID 150
V_ORN 62 VCLP -2.25
V_ORP 60 VCLP 2.25
V4 35 OUT 0
V11 68 61 0
V12 67 59 0
VCM_MAX 81 VCC_B 100M
VCM_MIN 80 VEE_B -100M
VS1 21 22 -242U
XCL_AMP 63 64 VIMON MID 65 66 CLAMP_AMP_LO_0_FEEL_OPA2310
XCL_SRC 69 70 CL_CLAMP MID VCCS_LIM_4_0_FEEL_OPA2310
XCLAW_AMP VCC_CLP VEE_CLP VOUT_S MID 71 72 CLAMP_AMP_LO_0_FEEL_OPA2310
XCLAW_SRC 73 74 CLAW_CLAMP MID VCCS_LIM_3_0_FEEL_OPA2310
XCLAWN MID VIMON VEE_B 24 VCCS_LIM_CLAW-_0_FEEL_OPA2310
XCLAWP VIMON MID 23 VCC_B VCCS_LIM_CLAW+_0_FEEL_OPA2310
XE_N ESDP 21 VNSE_0_FEEL_OPA2310
XGR_AMP 48 49 50 MID 51 52 CLAMP_AMP_HI_0_FEEL_OPA2310
XGR_SRC 53 54 CLAMP MID VCCS_LIM_GR_0_FEEL_OPA2310
XI_NN ESDN MID FEMT_0_FEEL_OPA2310
XI_NP MID 21 FEMT_0_FEEL_OPA2310
XIQN MID VIMON VEE MID VCCS_LIMIT_IQ_0_FEEL_OPA2310
XIQP VIMON MID MID VCC VCCS_LIMIT_IQ_0_FEEL_OPA2310
XOL_SENSE MID 75 57 55 OL_SENSE_0_FEEL_OPA2310
XU1 37 MID MID 42 VCCS_LIM_ZO_0_FEEL_OPA2310
XVCCS_LIM_1 33 34 MID 32 VCCS_LIM_1_0_FEEL_OPA2310
XVCCS_LIM_2 32 MID MID CLAMP VCCS_LIM_2_0_FEEL_OPA2310
XVCM_CLAMP 82 MID 79 MID 81 80 VCCS_EXT_LIM_0_FEEL_OPA2310
.ENDS OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT CLAMP_AMP_HI_0_FEEL_OPA2310 VC+ VC- VIN COM VO+ VO-
****************************************
* Parameters
.PARAM G=10
****************************************
* Components
GVO- COM VO- VALUE = {IF(V(VIN,COM)<V(VC-,COM),((V(VC-,COM)-V(VIN,COM))*G),0)}
GVO+ COM VO+ VALUE = {IF(V(VIN,COM)>V(VC+,COM),((V(VIN,COM)-V(VC+,COM))*G),0)}
.ENDS CLAMP_AMP_HI_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT CLAMP_AMP_LO_0_FEEL_OPA2310 VC+ VC- VIN COM VO+ VO-
****************************************
* Parameters
.PARAM G=1
****************************************
* Components
GVO- COM VO- VALUE = {IF(V(VIN,COM)<V(VC-,COM),((V(VC-,COM)-V(VIN,COM))*G),0)}
GVO+ COM VO+ VALUE = {IF(V(VIN,COM)>V(VC+,COM),((V(VIN,COM)-V(VC+,COM))*G),0)}
.ENDS CLAMP_AMP_LO_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT FEMT_0_FEEL_OPA2310 1 2
****************************************
* Parameters
.PARAM NVRF=10
.PARAM RNVF={1.184*PWR(NVRF,2)}
****************************************
* Components
E1 3 0 5 0 10
G1 1 2 3 0 1E-6
R1 5 0 {RNVF} 
R2 5 0 {RNVF} 
.ENDS FEMT_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT OL_SENSE_0_FEEL_OPA2310 COM SW+ OLN OLP
****************************************
* Components
GSW+ COM SW+ VALUE = {IF((V(OLN,COM)>10E-3|V(OLP,COM)>10E-3),1,0)}
.ENDS OL_SENSE_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_EXT_LIM_0_FEEL_OPA2310 VIN+ VIN- IOUT- IOUT+ VP+ VP-
****************************************
* Parameters
.PARAM GAIN=1
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VIN+,VIN-),V(VP-,VIN-),V(VP+,VIN-))}
.ENDS VCCS_EXT_LIM_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_1_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=1E-4
.PARAM INEG=-.5
.PARAM IPOS=.5
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VC+,VC-),INEG,IPOS)}
.ENDS VCCS_LIM_1_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_2_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=3.201E-02
.PARAM INEG=-.24
.PARAM IPOS=.24
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VC+,VC-),INEG,IPOS)}
.ENDS VCCS_LIM_2_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_3_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=1
.PARAM INEG=-3.5
.PARAM IPOS=3.5
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VC+,VC-),INEG,IPOS)}
.ENDS VCCS_LIM_3_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_4_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=1
.PARAM INEG=-7.5
.PARAM IPOS=7.5
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VC+,VC-),INEG,IPOS)}
.ENDS VCCS_LIM_4_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_CLAW-_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Components
G1 IOUT+ IOUT- TABLE {(V(VC+,VC-))} =(0, 9.34E-08)(140, 3.24E-04)(170, 4.19E-04)(180, 5.08E-04)(190, 8.97E-04)(200,
+ 1.99E-03)(210, 4.52E-03)
.ENDS VCCS_LIM_CLAW-_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_CLAW+_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Components
G1 IOUT+ IOUT- TABLE {ABS(V(VC+,VC-))} =(0, 4.35E-08)(120, 2.84E-04)(130, 4.34E-04)(140, 6.70E-04)(150, 1.87E-03)(160,
+ 3.00E-03)(170, 4.61E-03)
.ENDS VCCS_LIM_CLAW+_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_GR_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=1
.PARAM INEG=-1
.PARAM IPOS=1
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VC+,VC-),INEG,IPOS)}
.ENDS VCCS_LIM_GR_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIM_ZO_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=11
.PARAM INEG=-240E6
.PARAM IPOS=240E6
****************************************
* Components
G1 IOUT+ IOUT- VALUE={LIMIT(GAIN*V(VC+,VC-),INEG,IPOS)}
.ENDS VCCS_LIM_ZO_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VCCS_LIMIT_IQ_0_FEEL_OPA2310 VC+ VC- IOUT+ IOUT-
****************************************
* Parameters
.PARAM GAIN=1E-3
****************************************
* Components
G1 IOUT- IOUT+ VALUE={IF((V(VC+,VC-)<=0),0,GAIN*V(VC+,VC-))}
.ENDS VCCS_LIMIT_IQ_0_FEEL_OPA2310
************************************************************************************************************************

************************************************************************************************************************
.SUBCKT VNSE_0_FEEL_OPA2310 1 2
****************************************
* Parameters
.PARAM FLW=100
.PARAM GLF={PWR(FLW,0.25)*NLF/1164}
.PARAM NLF=32
.PARAM NVR=6
.PARAM RNV={1.184*PWR(NVR,2)}
****************************************
* Models
.MODEL DVN D (KF={PWR(FLW,0.5)/1E11} IS=1.0E-16)
****************************************
* Components
D1 7 0 DVN 
D2 8 0 DVN 
E1 3 6 7 8 {GLF}
E2 6 4 5 0 10
E3 1 2 3 4 1
I1 0 7 10E-3
I2 0 8 10E-3
R1 3 0 1E9 
R2 3 0 1E9 
R3 3 6 1E9 
R4 5 0 {RNV} 
R5 5 0 {RNV} 
R6 3 4 1E9 
R7 4 0 1E9 
.ENDS VNSE_0_FEEL_OPA2310
************************************************************************************************************************

