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
//# PROJECT:           Object Oriented Finite Element Program
//# PURPOSE:           Finite Deformation Hyper-Elastic classes
//# CLASS:
//#
//# VERSION:           0.6_(1803398874989) (golden section)
//# LANGUAGE:          C++
//# TARGET OS:         all...
//# DESIGN:            Zhao Cheng, Boris Jeremic (jeremic@ucdavis.edu)
//# PROGRAMMER(S):     Zhao Cheng, Boris Jeremic
//#
//#
//# DATE:              July 2004
//# UPDATE HISTORY:
//#
//===============================================================================

#ifndef fdYieldDP_H
#define fdYieldDP_H

#include "material/nD/FiniteDeformation/fdYield/fdYield.h"

namespace XC {
//! @ingroup FDYieldNDMat
//
//! @brief ??.
class fdYieldDP : public fdYield
{
  private:
    double FricAngle;
    double Cohesion;
    int ConeIndex;
    double k1;
    double k2; 
  public:
    fdYieldDP(double FricAngle_in, double Cohesion_in, int ConeIndex_in = 0);
    // virtual ~fdYieldDP() {}; 
    
    fdYield *newObj();   

    int getNumRank();
    double getTolerance();
    
    double Yd(const stresstensor &sts, const FDEPState &fdepstate ) const;	

    stresstensor dYods(const stresstensor &sts, const FDEPState &fdepstate ) const; 
    double dYodq(const stresstensor &sts, const FDEPState &fdepstate ) const;	    
    stresstensor dYoda(const stresstensor &sts, const FDEPState &fdepstate ) const;

    void print(void);

    friend std::ostream& operator<< (std::ostream& os, const fdYieldDP & fdydDP);
};
} // fin namespace XC


#endif