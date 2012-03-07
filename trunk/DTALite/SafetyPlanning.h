
class DTASafetyPredictionModel
{
public:

	double time_dependent_flow_ratio[MAX_TIME_INTERVAL_SIZE];

	int safety_crash_model_id;

	double input_length_in_miles;
	double input_upstream_ADT;
	double input_on_ramp_ADT;
	double input_off_ramp_ADT;
	double input_spacing_in_feet;
	double input_aux_lane;
	double input_num_lanes;


	double coeff_length_in_miles;
	double coeff_upstream_ADT;
	double coeff_on_ramp_ADT;
	double coeff_off_ramp_ADT;
	double coeff_freeway_constant;
	double coeff_inverse_spacing_in_feet;
	double coeff_aux_lane;
	double coeff_num_lanes;

	double num_fi_crashes_per_year;
	double avg_additional_delay_per_vehicle_per_crash_in_min;
	double avg_additional_delay_per_peak_period;
	double peak_hour_time_of_day_ratio;


	double num_pto_crashes_per_year;
	double add_delay_per_period;
	double AADT;
	double minor_leg_AADT;
	double two_way_AADT;
	double on_ramp_AADT;
	double off_ramp_AADT;
	double upstream_AADT;
	double num_driveway;
	double intersection_3sg;
	double intersection_4sg;
	double intersection_3st;
	double intersection_4st;

	DTASafetyPredictionModel()
	{
		peak_hour_time_of_day_ratio = 0.15;
		num_fi_crashes_per_year = 0;
		input_length_in_miles = 0.7f;
		input_num_lanes = 3;
		input_upstream_ADT = 80000;
		input_on_ramp_ADT = 2500;
		input_off_ramp_ADT = 3500;
		input_spacing_in_feet = 2000;
		input_aux_lane = 1;

		//  	1/3 * [length_in_miles^1.0*upstream_ADT^0.9212*on_ramp_ADT^0.1209*off_ramp_ADT^0.0445*exp(-8.492 + (513.59/spacing_in_feet) – 300.89*(aux_lane/spacing_in_feet) + 0.1638*input_aux_lane)] 

		coeff_length_in_miles = 1.0f;
		coeff_upstream_ADT = 0.9212f;
		coeff_on_ramp_ADT = 0.1209f;
		coeff_off_ramp_ADT = 0.0445f;
		coeff_freeway_constant = -8.492f;
		coeff_inverse_spacing_in_feet = 513.59f;
		coeff_aux_lane = -300.89f;
		coeff_num_lanes = 0.1638f;
		avg_additional_delay_per_vehicle_per_crash_in_min = 10.15;

		time_dependent_flow_ratio[	0	]=	0.000695	;
		time_dependent_flow_ratio[	1	]=	0.000867	;
		time_dependent_flow_ratio[	2	]=	0.001196	;
		time_dependent_flow_ratio[	3	]=	0.001535	;
		time_dependent_flow_ratio[	4	]=	0.002054	;
		time_dependent_flow_ratio[	5	]=	0.002536	;
		time_dependent_flow_ratio[	6	]=	0.003225	;
		time_dependent_flow_ratio[	7	]=	0.003795	;
		time_dependent_flow_ratio[	8	]=	0.004558	;
		time_dependent_flow_ratio[	9	]=	0.005041	;
		time_dependent_flow_ratio[	10	]=	0.005714	;
		time_dependent_flow_ratio[	11	]=	0.006124	;
		time_dependent_flow_ratio[	12	]=	0.006723	;
		time_dependent_flow_ratio[	13	]=	0.00716	;
		time_dependent_flow_ratio[	14	]=	0.007686	;
		time_dependent_flow_ratio[	15	]=	0.008	;
		time_dependent_flow_ratio[	16	]=	0.008303	;
		time_dependent_flow_ratio[	17	]=	0.008344	;
		time_dependent_flow_ratio[	18	]=	0.008435	;
		time_dependent_flow_ratio[	19	]=	0.008488	;
		time_dependent_flow_ratio[	20	]=	0.008584	;
		time_dependent_flow_ratio[	21	]=	0.008741	;
		time_dependent_flow_ratio[	22	]=	0.009036	;
		time_dependent_flow_ratio[	23	]=	0.009369	;
		time_dependent_flow_ratio[	24	]=	0.009902	;
		time_dependent_flow_ratio[	25	]=	0.010305	;
		time_dependent_flow_ratio[	26	]=	0.010767	;
		time_dependent_flow_ratio[	27	]=	0.010989	;
		time_dependent_flow_ratio[	28	]=	0.010938	;
		time_dependent_flow_ratio[	29	]=	0.010977	;
		time_dependent_flow_ratio[	30	]=	0.010999	;
		time_dependent_flow_ratio[	31	]=	0.011179	;
		time_dependent_flow_ratio[	32	]=	0.011605	;
		time_dependent_flow_ratio[	33	]=	0.011984	;
		time_dependent_flow_ratio[	34	]=	0.01249	;
		time_dependent_flow_ratio[	35	]=	0.012843	;
		time_dependent_flow_ratio[	36	]=	0.013251	;
		time_dependent_flow_ratio[	37	]=	0.013603	;
		time_dependent_flow_ratio[	38	]=	0.01416	;
		time_dependent_flow_ratio[	39	]=	0.014783	;
		time_dependent_flow_ratio[	40	]=	0.015821	;
		time_dependent_flow_ratio[	41	]=	0.016768	;
		time_dependent_flow_ratio[	42	]=	0.017986	;
		time_dependent_flow_ratio[	43	]=	0.019031	;
		time_dependent_flow_ratio[	44	]=	0.020315	;
		time_dependent_flow_ratio[	45	]=	0.021086	;
		time_dependent_flow_ratio[	46	]=	0.021838	;
		time_dependent_flow_ratio[	47	]=	0.022187	;
		time_dependent_flow_ratio[	48	]=	0.022447	;
		time_dependent_flow_ratio[	49	]=	0.022543	;
		time_dependent_flow_ratio[	50	]=	0.02257	;
		time_dependent_flow_ratio[	51	]=	0.022601	;
		time_dependent_flow_ratio[	52	]=	0.022533	;
		time_dependent_flow_ratio[	53	]=	0.022417	;
		time_dependent_flow_ratio[	54	]=	0.022135	;
		time_dependent_flow_ratio[	55	]=	0.021827	;
		time_dependent_flow_ratio[	56	]=	0.021109	;
		time_dependent_flow_ratio[	57	]=	0.020648	;
		time_dependent_flow_ratio[	58	]=	0.019933	;
		time_dependent_flow_ratio[	59	]=	0.019668	;
		time_dependent_flow_ratio[	60	]=	0.019556	;
		time_dependent_flow_ratio[	61	]=	0.019589	;
		time_dependent_flow_ratio[	62	]=	0.019629	;
		time_dependent_flow_ratio[	63	]=	0.019728	;
		time_dependent_flow_ratio[	64	]=	0.0198	;
		time_dependent_flow_ratio[	65	]=	0.019489	;
		time_dependent_flow_ratio[	66	]=	0.018791	;
		time_dependent_flow_ratio[	67	]=	0.017854	;
		time_dependent_flow_ratio[	68	]=	0.016271	;
		time_dependent_flow_ratio[	69	]=	0.014659	;
		time_dependent_flow_ratio[	70	]=	0.012347	;
		time_dependent_flow_ratio[	71	]=	0.010438	;
		time_dependent_flow_ratio[	72	]=	0.008134	;
		time_dependent_flow_ratio[	73	]=	0.006477	;
		time_dependent_flow_ratio[	74	]=	0.004709	;
		time_dependent_flow_ratio[	75	]=	0.004023	;
		time_dependent_flow_ratio[	76	]=	0.001517	;
		time_dependent_flow_ratio[	77	]=	0.002038	;
		time_dependent_flow_ratio[	78	]=	0.001918	;
		time_dependent_flow_ratio[	79	]=	0.001847	;
		time_dependent_flow_ratio[	80	]=	0.001826	;
		time_dependent_flow_ratio[	81	]=	0.001778	;
		time_dependent_flow_ratio[	82	]=	0.001665	;
		time_dependent_flow_ratio[	83	]=	0.001564	;
		time_dependent_flow_ratio[	84	]=	0.001427	;
		time_dependent_flow_ratio[	85	]=	0.001337	;
		time_dependent_flow_ratio[	86	]=	0.001249	;
		time_dependent_flow_ratio[	87	]=	0.001194	;
		time_dependent_flow_ratio[	88	]=	0.00114	;
		time_dependent_flow_ratio[	89	]=	0.001082	;
		time_dependent_flow_ratio[	90	]=	0.000987	;
		time_dependent_flow_ratio[	91	]=	0.000893	;
		time_dependent_flow_ratio[	92	]=	0.000754	;
		time_dependent_flow_ratio[	93	]=	0.000665	;
		time_dependent_flow_ratio[	94	]=	0.000586	;
		time_dependent_flow_ratio[	95	]=	0.000593	;

	}

	void EstimateDelayPerPeakPeriod()
	{
		avg_additional_delay_per_peak_period  = num_fi_crashes_per_year/260 * peak_hour_time_of_day_ratio  * avg_additional_delay_per_vehicle_per_crash_in_min;		 
		TRACE("additional_delay: %f\b",avg_additional_delay_per_peak_period);
	}


	double EstimateFreewayCrashRatePerYear(double input_upstream_ADT, double input_length_in_miles)
	{
		//  	1/3 * [length_in_miles^1.0*upstream_ADT^0.9212*on_ramp_ADT^0.1209*off_ramp_ADT^0.0445*exp(-8.492 + (513.59/spacing_in_feet) – 300.89*(aux_lane/spacing_in_feet) + 0.1638*input_aux_lane)] 
		num_fi_crashes_per_year  = pow(input_length_in_miles,coeff_length_in_miles)*
			pow(input_upstream_ADT,coeff_upstream_ADT)*
			pow(input_on_ramp_ADT,coeff_on_ramp_ADT)*
			pow(input_off_ramp_ADT,coeff_off_ramp_ADT) /3.0f * 
			exp( coeff_freeway_constant + coeff_inverse_spacing_in_feet/input_spacing_in_feet +
			coeff_aux_lane * input_aux_lane / input_spacing_in_feet + 
			coeff_num_lanes* input_num_lanes);

		TRACE("crash %f\b",num_fi_crashes_per_year);

		return num_fi_crashes_per_year;

	}

	// data for aterial crashes
	double Nmvnd_total, Nmvnd_fatal_injury,Nmvnd_PDO;
	double Nsv_total, Nsv_fatal_injury,Nsv_PDO;
	double Nmvd_total, Nmvd_fatal_injury,Nmvd_PDO;
	double Nmv_total_3SG,Nmv_fatal_injury_3SG,Nmv_PDO_3SG;
	double Nmv_total_3ST,Nmv_fatal_injury_3ST,Nmv_PDO_3ST;
	double Nmv_total_4SG, Nmv_fatal_injury_4SG,Nmv_PDO_4SG;
	double Nmv_total_4ST, Nmv_fatal_injury_4ST,Nmv_PDO_4ST;
	double Nsv_total_3SG, Nsv_fatal_injury_3SG,Nsv_PDO_3SG;
	double Nsv_total_3ST, Nsv_fatal_injury_3ST, NSv_PDO_3ST;
	double Nsv_total_4SG, Nsv_fatal_injury_4SG,Nsv_PDO_4SG;
	double Nsv_total_4ST,Nsv_fatal_injury_4ST,NSv_PDO_4ST;


	double EstimateArterialCrashRatePerYear(double AADT = 26000,
		double Length  = 0.89, 
		double Num_Driveways_Per_Mile = 20,
		double volume_proportion_on_minor_leg = 0.1,
		double Num_3SG_Intersections = 1,
		double Num_3ST_Intersections = 1,
		double Num_4SG_Intersections = 3,
		double Num_4ST_Intersections = 0)
	{
		Nmvnd_total = exp(-11.63 + 1.33*log(AADT) + log(Length));
		Nmvnd_fatal_injury = Nmvnd_total*(exp(-12.08 + 1.25*log(AADT) 
			+ log(Length))/(exp(-12.08 + 1.25*log(AADT) + log(Length)) + exp(-12.53 + 1.38*log(AADT) + log(Length))));
		Nmvnd_PDO = Nmvnd_total - Nmvnd_fatal_injury;

		Nsv_total = exp(-7.99 + 0.81*log(AADT) + log(Length));
		Nsv_fatal_injury = Nsv_total*(exp(-7.37 + 0.61*log(AADT) + log(Length))/(exp(-7.37 + 0.61*log(AADT) + log(Length)) + exp(-8.5 + 0.84*log(AADT) + log(Length))));
		Nsv_PDO = Nsv_total - Nsv_fatal_injury;

		Nmvd_total = pow(0.083*Num_Driveways_Per_Mile*Length*(AADT/15000),1);
		Nmvd_fatal_injury = Nmvd_total*0.323;
		Nmvd_PDO = Nmvd_total*0.677;

		Nmv_total_3SG = Num_3SG_Intersections*exp(-12.13 + 1.11*log(AADT) + 0.26*log(volume_proportion_on_minor_leg*AADT));
		Nmv_fatal_injury_3SG = Nmv_total_3SG*
			exp(-11.58 + 1.02*log(AADT) + 0.17*log(volume_proportion_on_minor_leg*AADT))/
			( exp(-11.58 + 1.02*log(AADT) + 0.17*log(volume_proportion_on_minor_leg*AADT))+ 
			exp(-13.24 + 1.14*log(AADT) + 0.3*log(volume_proportion_on_minor_leg*AADT)));
		Nmv_PDO_3SG = Nmv_total_3SG - Nmv_fatal_injury_3SG;


		Nmv_total_3ST = Num_3ST_Intersections*exp(-13.36 + 1.11*log(AADT) + 0.41*log(volume_proportion_on_minor_leg*AADT));
		Nmv_fatal_injury_3ST = Nmv_total_3ST* exp(-14.01 + 1.16*log(AADT) + 0.3*log(volume_proportion_on_minor_leg*AADT))
			/( exp(-14.01 + 1.16*log(AADT) + 0.3*log(volume_proportion_on_minor_leg*AADT))+ 
			exp(-15.38 + 1.2*log(AADT) + 0.51*log(volume_proportion_on_minor_leg*AADT)) );
		Nmv_PDO_3ST = Nmv_total_3ST - Nmv_fatal_injury_3ST ;


		Nmv_total_4SG = Num_4SG_Intersections*exp(-10.99 + 1.07*log(AADT) + 0.23*log(volume_proportion_on_minor_leg*AADT));
		Nmv_fatal_injury_4SG = Nmv_total_4SG
			*(exp(-13.14 + 1.18*log(AADT) + 0.22*log(volume_proportion_on_minor_leg*AADT))
			/(exp(-13.14 + 1.18*log(AADT) + 0.22*log(volume_proportion_on_minor_leg*AADT))
			+exp(-11.02 + 1.02*log(AADT) + 0.24*log(volume_proportion_on_minor_leg*AADT))));
		Nmv_PDO_4SG = Nmv_total_4SG - Nmv_fatal_injury_4SG;

		Nmv_total_4ST = Num_4ST_Intersections*exp(-8.9 + 0.82*log(AADT) + 0.25*log(volume_proportion_on_minor_leg*AADT));
		Nmv_fatal_injury_4ST = Nmv_total_4ST
			*(exp(-11.13 + 0.93*log(AADT) + 0.28*log(volume_proportion_on_minor_leg*AADT))
			/( exp(-11.13 + 0.93*log(AADT) + 0.28*log(volume_proportion_on_minor_leg*AADT))
			+exp(-8.74 + 0.77*log(AADT) + 0.23*log(volume_proportion_on_minor_leg*AADT))));
		Nmv_PDO_4ST = Nmv_total_4ST - Nmv_fatal_injury_4ST;

		Nsv_total_3SG = Num_3SG_Intersections*exp(-9.02 + 0.42*log(AADT) + 0.4*log(volume_proportion_on_minor_leg*AADT));
		Nsv_fatal_injury_3SG =Nsv_total_3SG
			*(exp(-9.75 + 0.27*log(AADT) + 0.51*log(volume_proportion_on_minor_leg*AADT))
			/( exp(-9.75 + 0.27*log(AADT) + 0.51*log(volume_proportion_on_minor_leg*AADT))
			+exp(-9.08 + 0.45*log(AADT) + 0.33*log(volume_proportion_on_minor_leg*AADT))));
		Nsv_PDO_3SG = Nsv_total_3SG - Nsv_fatal_injury_3SG;

		Nsv_total_3ST = Num_3ST_Intersections*exp(-6.81 + 0.16*log(AADT) + 0.51*log(volume_proportion_on_minor_leg*AADT));
		Nsv_fatal_injury_3ST = Nsv_total_3ST
			*(0.31*exp(-6.81 + 0.16*log(AADT) + 0.51*log(volume_proportion_on_minor_leg*AADT)))
			/((0.31*exp(-6.81 + 0.16*log(AADT) + 0.51*log(volume_proportion_on_minor_leg*AADT)))
			+ exp(-8.36 + 0.25*log(AADT) + 0.55*log(volume_proportion_on_minor_leg*AADT) ) );
		NSv_PDO_3ST = Nsv_total_3ST - Nsv_fatal_injury_3ST;

		Nsv_total_4SG = Num_4SG_Intersections*exp(-10.21 + 0.68*log(AADT) + 0.27*log(volume_proportion_on_minor_leg*AADT));
		Nsv_fatal_injury_4SG = Nsv_total_4SG*
			exp(-9.25 + 0.43*log(AADT) + 0.29*log(volume_proportion_on_minor_leg*AADT))
			/ (exp(-9.25 + 0.43*log(AADT) + 0.29*log(volume_proportion_on_minor_leg*AADT))
			+ exp(-11.34 + 0.78*log(AADT) + 0.25*log(volume_proportion_on_minor_leg*AADT)));
		Nsv_PDO_4SG = Nsv_total_4SG - Nsv_fatal_injury_4SG;

		Nsv_total_4ST = Num_4ST_Intersections*exp(-5.33 + 0.33*log(AADT) + 0.12*log(volume_proportion_on_minor_leg*AADT));
		Nsv_fatal_injury_4ST = Nsv_total_4ST*
			(0.28*exp(-5.33 + 0.33*log(AADT) + 0.12*log(volume_proportion_on_minor_leg*AADT))
			/(0.28*exp(-5.33 + 0.33*log(AADT) + 0.12*log(volume_proportion_on_minor_leg*AADT))
			+exp(-7.04 + 0.36*log(AADT) + 0.25*log(volume_proportion_on_minor_leg*AADT))));
		NSv_PDO_4ST = Nsv_total_4ST - Nsv_fatal_injury_4ST ;

		TRACE("%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n%f\n",
			Nmvnd_total, Nmvnd_fatal_injury,Nmvnd_PDO,
			Nsv_total, Nsv_fatal_injury,Nsv_PDO,
			Nmvd_total, Nmvd_fatal_injury,Nmvd_PDO,
			Nmv_total_3SG,Nmv_fatal_injury_3SG,Nmv_PDO_3SG,
			Nmv_total_3ST,Nmv_fatal_injury_3ST,Nmv_PDO_3ST,
			Nmv_total_4SG, Nmv_fatal_injury_4SG,Nmv_PDO_4SG,
			Nmv_total_4ST, Nmv_fatal_injury_4ST,Nmv_PDO_4ST,
			Nsv_total_3SG, Nsv_fatal_injury_3SG,Nsv_PDO_3SG,
			Nsv_total_3ST, Nsv_fatal_injury_3ST, NSv_PDO_3ST,
			Nsv_total_4SG, Nsv_fatal_injury_4SG,Nsv_PDO_4SG,
			Nsv_total_4ST,Nsv_fatal_injury_4ST,NSv_PDO_4ST);

		double total_crash_per_year = 
			Nmvnd_total +
			Nsv_total +
			Nmvd_total + 
			Nmv_total_3SG +
			Nmv_total_3ST +
			Nmv_total_4SG +
			Nmv_total_4ST +
			Nsv_total_3SG +
			Nsv_total_3ST + 
			Nsv_total_4SG +
			Nsv_total_4ST;
		return total_crash_per_year;
	}

	double m_AADTConversionFactorForStudyHorizion;

	void UpdateAADTConversionFactor();
	void UpdateCrashRateForAllLinks();
};