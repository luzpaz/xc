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
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 2001, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** Reliability module developed by:                                   **
**   Terje Haukaas (haukaas@ce.berkeley.edu)                          **
**   Armen Der Kiureghian (adk@ce.berkeley.edu)                       **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.2 $
// $Date: 2003/10/27 23:45:45 $
// $Source: /usr/local/cvs/OpenSees/SRC/reliability/analysis/transformation/NatafProbabilityTransformation.h,v $


//
// Written by Terje Haukaas (haukaas@ce.berkeley.edu)
//

#ifndef NatafProbabilityTransformation_h
#define NatafProbabilityTransformation_h

#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>
#include <reliability/domain/components/ReliabilityDomain.h>
#include <reliability/analysis/misc/MatrixOperations.h>

namespace XC {
class NatafProbabilityTransformation : public ProbabilityTransformation
{

public:
	NatafProbabilityTransformation(ReliabilityDomain *passedReliabilityDomain,
						  int printFlag);
	~NatafProbabilityTransformation();

	int set_x(Vector x);
	int set_u(Vector u);

	int transform_x_to_u();
	int transform_u_to_x();
	int transform_u_to_x_andComputeJacobian();

	Vector get_x();
	Vector get_u();
	Matrix getJacobian_x_u();
	Matrix getJacobian_u_x();

	Vector meanSensitivityOf_x_to_u(Vector &x, int gradNumber);
	Vector stdvSensitivityOf_x_to_u(Vector &x, int gradNumber);

protected:

private:

	// Private data members
	Vector *x;
	Vector *u;
	Matrix *jacobian_x_u;
	Matrix *jacobian_u_x;
	ReliabilityDomain *theReliabilityDomain;
	MatrixOperations *theMatrixOperations;
	int nrv;
	Matrix *correlationMatrix;
	Matrix *lowerCholesky;
	Matrix *inverseLowerCholesky;
	int printFlag;

	// Private member functions
	void setCorrelationMatrix(int pertMeanOfThisRV, int pertStdvOfThisRV, double h);
	Matrix getJacobian_z_x(Vector x, Vector z);
	Vector z_to_x(Vector z);
	Vector x_to_z(Vector x);

	// Auxiliary member functions for manual evaluation of 
	// the integral equation to find Nataf correlation
	double phi2(double z_i, 
				double z_j, 
				double rho);
	double integrand(int rv_i,
				     double z_i, 
				     double mean_i,
				     double stdv_i, 
				     int rv_j,
				     double z_j,
				     double mean_j, 
				     double stdv_j,
				     double rho);
	double doubleIntegral(int rv_i,
						  double mean_i,
						  double stdv_i, 
						  int rv_j,
						  double mean_j, 
						  double stdv_j,
						  double rho);
	double residualFunction(double rho_original, 
						    double rho,
						    double rv_i, 
						    double mean_i, 
						    double stdv_i, 
						    double rv_j, 
						    double mean_j, 
						    double stdv_j);
	double solveForCorrelation(int rv_i, int rv_j, double rho_original);
};
} // fin namespace XC

#endif