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
//===============================================================================
//# COPYRIGHT (C): Woody's license (by BJ):
//                 ``This    source  code is Copyrighted in
//                 U.S.,  for  an  indefinite  period,  and anybody
//                 caught  using it without our permission, will be
//                 mighty good friends of ourn, cause we don't give
//                 a  darn.  Hack it. Compile it. Debug it. Run it.
//                 Yodel  it.  Enjoy it. We wrote it, that's all we
//                 wanted to do.''
//
//# PROJECT:           Object Oriented Finite XC::Element Program
//# PURPOSE:           Hyper-spherical Constraint
//# CLASS:             HSConstraint
//#
//# VERSION:           0.61803398874989 (golden section)
//# LANGUAGE:          C++
//# TARGET OS:         all...
//# DESIGN:            Ritu Jain, Boris Jeremic
//# PROGRAMMER(S):     Ritu, Boris Jeremic
//#
//#
//# DATE:              14Mar2003
//# UPDATE HISTORY:
//#
//#
//===============================================================================



#include <solution/analysis/integrator/static/HSConstraint.h>
#include <solution/analysis/model/AnalysisModel.h>
#include <solution/system_of_eqn/linearSOE/LinearSOE.h>
#include <cmath>
#include "utility/actor/actor/MatrixCommMetaData.h"

//! @brief Constructor.
XC::HSConstraint::HSConstraint(SoluMethod *owr,double arcLength, double psi_u, double psi_f, double u_ref)
  :ProtoArcLength(owr,INTEGRATOR_TAGS_HSConstraint,arcLength),
   psi_u2(psi_u*psi_u), psi_f2(psi_f*psi_f), u_ref2(u_ref*u_ref) {}

//! @brief Devuelve el valor de dLambda para el método newStep.
double XC::HSConstraint::getDLambdaNewStep(void) const
  {
    const Vector &dUhat= vectores.getDeltaUhat();
    const Vector &f_ext= vectores.getPhat();

    // determine delta lambda(1) == dlambda
//    double retval = sqrt(arcLength2/((dUhat^dUhat)+alpha2));
// out temp BJ 
//    double retval = sqrt(arcLength2/((psi_u2/u_ref2*fabs(dUhat^dUhat))+psi_f2));
// old version with fext
    double retval = sqrt(arcLength2/( (psi_u2/u_ref2*fabs(dUhat^dUhat) ) + psi_f2*(f_ext^f_ext)  ));
    retval *= signLastDeltaLambdaStep; // base sign of load change
                                        // on what was happening last step
    return retval;
  }

//! @brief Devuelve el valor de dLambda para el método update.
double XC::HSConstraint::getDLambdaUpdate(void) const
  {
    const double &dLStep= vectores.getDeltaLambdaStep();
    const Vector &dUhat= vectores.getDeltaUhat();
    const Vector &dUstep= vectores.getDeltaUstep();
    const Vector &dUbar= vectores.getDeltaUbar();
    const Vector &f_ext= vectores.getPhat();
    const double prod_f_ext= (f_ext^f_ext);

    // determine the coeeficients of our quadratic equation
    const double a1= psi_u2/u_ref2*(dUhat^dUhat) + psi_f2 * prod_f_ext;
    
    const double a2= 2.0 *(psi_u2/u_ref2*((dUhat^dUbar)+(dUhat^dUstep))
                      + psi_f2*dLStep * prod_f_ext);
    
    const double a3= psi_u2/u_ref2 * ((dUstep+dUbar)^(dUstep+dUbar)) - arcLength2 
                      + (dLStep*dLStep)*psi_f2 * prod_f_ext ;

    // check for a solution to quadratic
    const double b24ac = a2*a2 - a1*a3;
    if(b24ac < 0)
      {
        std::cerr << "XC::HSConstraint::update() - imaginary roots due to multiple instability";
        std::cerr << " directions - initial load increment was too large\n";
        std::cerr << "a1: " << a1 << " a2: " << a2 << " a3: " << a3 << " b24ac: " << b24ac << std::endl;
        return -1;
      }
    double retval;
    if(a1 == 0.0)
      {
        // std::cerr << "XC::HSConstraint::update() - zero denominator";
        // std::cerr << "\n";
        // return -2;
	retval= -a3/(2.0*a2);
      }
    else
      {
    	// determine the roots of the quadratic
    	const double sqrtb24ac= sqrt(b24ac);
    	const double dlambda1= (-a2 + sqrtb24ac)/a1;
    	const double dlambda2= (-a2 - sqrtb24ac)/a1;

	//Vector deltaU1 = dUbar;
	//deltaU1->addVector(1.0, dUhat,dlambda1);
	//double costheta1 = dUstep^(dUstep+deltaU1);

	//Vector deltaU2 = dUbar;
	//deltaU2->addVector(1.0, dUhat,dlambda2);
	//double costheta2 = dUstep)^(dUstep+deltaU2);

        const double val= dUhat^dUstep;
    	double costheta1= (dUstep^dUstep) + (dUbar^dUstep);
    	const double costheta2= costheta1 + dlambda2*val;

    	costheta1+= dlambda1*val;

    	// choose retval based on angle between incremental displacement before
    	// and after this step -- want positive
    	if(costheta1 > costheta2)
     	  retval= dlambda1;
    	else
      	  retval= dlambda2;
      }
    return retval;
  }


//! @brief Envía los miembros del objeto a través del canal que se pasa como parámetro.
int XC::HSConstraint::sendData(CommParameters &cp)
  {
    int res= ProtoArcLength::sendData(cp);
    res+= cp.sendDoubles(psi_u2,psi_f2,u_ref2,getDbTagData(),CommMetaData(18));
    res+= cp.sendMatrix(scalingMatrix,getDbTagData(),CommMetaData(19));
    return res;
  }

//! @brief Recibe los miembros del objeto a través del canal que se pasa como parámetro.
int XC::HSConstraint::recvData(const CommParameters &cp)
  {
    int res= ProtoArcLength::recvData(cp);
    res+= cp.receiveDoubles(psi_u2,psi_f2,u_ref2,getDbTagData(),CommMetaData(18));
    res+= cp.receiveMatrix(scalingMatrix,getDbTagData(),CommMetaData(19));
    return res;
  }

int XC::HSConstraint::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(23);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << nombre_clase() << "sendSelf() - failed to send data\n";
    return res;
  }


int XC::HSConstraint::recvSelf(const CommParameters &cp)
  {
    inicComm(23);
    const int dataTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dataTag);

    if(res<0)
      std::cerr << nombre_clase() << "::recvSelf - failed to receive ids.\n";
    else
      {
        //setTag(getDbTagDataPos(0));
        res+= recvData(cp);
        if(res<0)
          std::cerr << nombre_clase() << "::recvSelf - failed to receive data.\n";
      }
    return res;
  }

void XC::HSConstraint::Print(std::ostream &s, int flag)
  {
    ProtoArcLength::Print(s,flag);
    s << "  HSConstraint: " << sqrt(arcLength2) /*<<  "  alpha: ";
    s << sqrt(alpha2) */ << std::endl;
  }