//----------------------------------------------------------------------------
//  programa XC; cálculo mediante el método de los elementos finitos orientado
//  a la solución de problemas estructurales.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  El programa deriva del denominado OpenSees <http://opensees.berkeley.edu>
//  desarrollado por el «Pacific earthquake engineering research center».
//
//  Salvo las restricciones que puedan derivarse del copyright del
//  programa original (ver archivo copyright_opensees.txt) este
//  software es libre: usted puede redistribuirlo y/o modificarlo 
//  bajo los términos de la Licencia Pública General GNU publicada 
//  por la Fundación para el Software Libre, ya sea la versión 3 
//  de la Licencia, o (a su elección) cualquier versión posterior.
//
//  Este software se distribuye con la esperanza de que sea útil, pero 
//  SIN GARANTÍA ALGUNA; ni siquiera la garantía implícita
//  MERCANTIL o de APTITUD PARA UN PROPÓSITO DETERMINADO. 
//  Consulte los detalles de la Licencia Pública General GNU para obtener 
//  una información más detallada. 
//
// Debería haber recibido una copia de la Licencia Pública General GNU 
// junto a este programa. 
// En caso contrario, consulte <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
// Inelastic2DYS03.cpp
//////////////////////////////////////////////////////////////////////

#include "domain/mesh/element/truss_beam_column/updatedLagrangianBeamColumn/Inelastic2DYS03.h"
#include <cmath>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

XC::Inelastic2DYS03::Inelastic2DYS03(int tag, double a_ten, double a_com,
                 double e, double iz_pos, double iz_neg,
                 int Nd1, int Nd2,
				 YieldSurface_BC *ysEnd1,  YieldSurface_BC *ysEnd2,
				 int rf_algo, bool islinear, double rho)

			:InelasticYS2DGNL(tag, Nd1, Nd2,
			ysEnd1, ysEnd2, rf_algo, islinear, rho),
			Atens(a_ten), Acomp(a_com), E(e), IzPos(iz_pos), IzNeg(iz_neg), ndisp(6), ndisp_hist(6)
{
	ndisp_hist.Zero();
	ndisp.Zero();
}

//! @brief Constructor virtual.
XC::Element* XC::Inelastic2DYS03::getCopy(void) const
  { return new XC::Inelastic2DYS03(*this); }

void XC::Inelastic2DYS03::getLocalStiff(Matrix &K) const
  {
     double L1,L2,I1,I2,A;
     Vector ndisp_inc(6);

     getIncrNaturalDisp(ndisp_inc);
     ndisp = ndisp_hist + ndisp_inc;

	// getTrialnaturalDisp(ndisp);
	
    std::cerr << ndisp;
    // std::cerr << ndisp_hist;
    // std::cerr << ndisp_inc;
    std::cerr << "\a";

	if(ndisp(2)*ndisp(5) < 0  || fabs(ndisp(2)*ndisp(5)) < 1e-10) {	//if single curvature
		L1 = L;
		L2 = 0;
		if(ndisp(2) > 0 || ndisp(5) < 0)
			I1 = I2 = IzNeg;
		else I1 = I2 = IzPos;
	} else {		//double curvature
		
		if((fabs(ndisp(2)) + fabs(ndisp(5)) < 1e-10))
			L1 = 0;
		else
			L1 = (fabs(ndisp(2))*L) / (fabs(ndisp(2)) + fabs(ndisp(5)));
			L2 = L - L1;
		if(ndisp(2) > 0) {
			I1 = IzNeg;
			I2 = IzPos;
		} else {
			I1 = IzPos;
			I2 = IzNeg;
		}
	}

	std::cerr << L1 << "  " << L2 << "\n";
	
	if(ndisp(3) < 0) //element is in compression
		A = Acomp;
	else					 //element is in tension
		A = Atens;

	//some factors used in stiffness matrix
	double X1 = 
I2*I2*L1*L1*L1*L1+4*I2*L1*L1*L1*L2*I1+6*I2*L1*L1*L2*L2*I1+4*I2*L1*L2*L2*L2*I1+L2*L2*L2*L2*I1*I1;
	double X2 = (I2*I1*(I2*L1*L1+2*L2*I1*L1+L2*L2*I1))/(X1);
	double X3 = ((L1*I2+L2*I1)*I2*I1)/(X1);
	double X4 = ((I2*L1*L1+2*I2*L1*L2+L2*L2*I1)*I2*I1)/(X1);

	//zeros in stiffness matrix
    K(0,1) = K(0,2) = K(0,4) = K(0,5)=0;
    K(1,0) = K(1,3) = 0;
    K(2,0) = K(2,3) = 0;
    K(3,1) = K(3,2) = K(3,4) = K(3,5)=0;
    K(4,0) = K(4,3) = 0;
    K(5,0) = K(5,3) = 0;

	//axial terms
  	K(0,0) = K(3,3) = (A*E)/(L); 
  	K(0,3) = K(3,0) = (-A*E)/(L);

	//shear and moment terms
  	K(1,1) = K(4,4) = 12*E*X3;
  	K(1,4) = K(4,1) = -12*E*X3;
  	K(1,2) = K(2,1) = 6*E*X2;
	K(1,5) = K(5,1) = 6*E*X4;
  	K(2,4) = K(4,2) = -6*E*X2;
	K(4,5) = K(5,4) = -6*E*X4;
  	K(2,2) = (4*E*I2*I1*(I2*L1*L1*L1 + 3*L2*I1*L1*L1 + 3*L2*L2*I1*L1 + 
              L2*L2*L2*I1))/X1;
	K(5,5) = (4*E*I2*I1*(I2*L1*L1*L1 + 3*I2*L1*L1*L2 + 3*I2*L1*L2*L2 + 
              L2*L2*L2*I1))/X1;
  	K(2,5) = K(5,2) = 2*E*I2*I1*(I2*L1*L1*L1 + 3*I2*L1*L1*L2 + 3*L2*L2*I1*L1 
             + L2*L2*L2*I1)/X1;

   std::cerr << "\nInelastic2DYS03::getLocalStiff(..) = \n" << K;
  }//getLocalStiff


int XC::Inelastic2DYS03::commitState(void)
  {
    // first let the super classes do their stuff
    this->XC::InelasticYS2DGNL::commitState();
    // now set the commit natural disps
    ndisp_hist = ndisp;
    return 0;
  }
