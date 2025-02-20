/*
 * data.h
 * 
 * Defines class for stellar data.
 * 
 * This file is part of bayestar.
 * Copyright 2012 Gregory Green
 * 
 * Bayestar is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 */

#ifndef _STELLAR_DATA_H__
#define _STELLAR_DATA_H__

#include "model.h"

#include <vector>
#include <string>
#include <math.h>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <algorithm>

//#define __STDC_LIMIT_MACROS
#include <stdint.h>

#include <unistd.h>

#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>

#include "h5utils.h"
#include "cpp_utils.h"


// Auxiliary functions
void seed_gsl_rng(gsl_rng **r);


struct TStellarData {
	struct TFileData {
		uint64_t obj_id;
		double l, b;
	        double pi, pierr;
		float mag[NBANDS];
		float err[NBANDS];
		float maglimit[NBANDS];
		uint32_t N_det[NBANDS];
		float EBV;
	};
	
	struct TMagnitudes {
		uint64_t obj_id;
		double l, b;
	        double pi, pierr;
		double m[NBANDS];
		double err[NBANDS];
		double maglimit[NBANDS];
		double maglim_width[NBANDS];
		unsigned int N_det[NBANDS];
		double EBV;
		double lnL_norm;
		
		TMagnitudes() {}
		
		TMagnitudes(double (&_m)[NBANDS], double (&_err)[NBANDS]) {
			lnL_norm = 0.;
			for(unsigned int i=0; i<NBANDS; i++) {
				m[i] = _m[i];
				err[i] = _err[i];
				maglimit[i] = 23.;
				maglim_width[i] = 0.20;
				if(err[i] < 9.e9) {	// Ignore missing bands (otherwise, they affect evidence)
					lnL_norm += 0.9189385332 + log(err[i]);
				}
			}
			EBV = 1.;
		}
		
		TMagnitudes& operator=(const TMagnitudes& rhs) {
			obj_id = rhs.obj_id;
			l = rhs.l;
			b = rhs.b;
			pi = rhs.pi;
			pierr=rhs.pierr;
			for(unsigned int i=0; i<NBANDS; i++) {
				m[i] = rhs.m[i];
				err[i] = rhs.err[i];
				maglimit[i] = rhs.maglimit[i];
				maglim_width[i] = rhs.maglim_width[i];
				N_det[i] = rhs.N_det[i];
			}
			lnL_norm = rhs.lnL_norm;
			EBV = rhs.EBV;
			return *this;
		}
		
		void set(const TStellarData::TFileData& dat, double err_floor = 0.02);
	};
	
	// Pixel metadata
	std::string pix_name;
	uint64_t healpix_index;
	uint32_t nside;
	bool nested;
	double l, b, EBV;
	std::vector<TMagnitudes> star;
	
	TStellarData(const std::string& infile, std::string _pix_name, double err_floor = 0.02);
	TStellarData(uint64_t _healpix_index, uint32_t _nside, bool _nested, double _l, double _b);
	TStellarData() {}
	
	TMagnitudes& operator[](unsigned int index) { return star.at(index); }
	
	void clear() { star.clear(); }
	
	// Read/write stellar photometry from/to HDF5 files
	bool save(const std::string& fname, const std::string& group, const std::string& dset, int compression=9);
	bool load(const std::string& fname, const std::string& group, const std::string& dset,
		  double err_floor=0.02, double default_EBV=5.);
};


class TDraw1D {
public:
	typedef double (*func_ptr_t)(double x, void* params);
	
	TDraw1D(func_ptr_t func, double _x_min, double _x_max, void* _params, unsigned int samples, bool is_log = false);
	~TDraw1D();
	
	double operator()();
	
private:
	double x_min, x_max;
	TMultiLinearInterp<double>* x_of_P;
	gsl_rng *r;
	void *params;
};

// Generate mock photometry from the given stellar and Galactic model, and magnitude limits
void draw_from_synth_model(size_t nstars, double RV, TGalacticLOSModel& gal_model, TSyntheticStellarModel& stellar_model,
                           TStellarData& stellar_data, TExtinctionModel& ext_model, double (&mag_limit)[NBANDS]);
void draw_from_emp_model(size_t nstars, double RV, TGalacticLOSModel& gal_model, TStellarModel& stellar_model,
                           TStellarData& stellar_data, TExtinctionModel& ext_model, double (&mag_limit)[NBANDS]);

// Return healpix indices of pixels in input file
void get_input_pixels(std::string fname, std::vector<std::string> &pix_name);


#endif // _STELLAR_DATA_H__
