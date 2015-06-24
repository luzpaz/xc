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
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in XC::main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */

// $Revision: 1.9 $
// $Date: 2004/06/07 23:21:19 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/forceBeamColumn/HingeRadauTwoBeamIntegration2d.cpp,v $

#include <domain/mesh/element/truss_beam_column/forceBeamColumn/beam_integration/HingeRadauTwoBeamIntegration2d.h>

#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include <utility/actor/objectBroker/FEM_ObjectBroker.h>
#include <domain/mesh/element/Information.h>
#include "domain/component/Parameter.h"

XC::HingeRadauTwoBeamIntegration2d::HingeRadauTwoBeamIntegration2d(double e,
                                                               double a,
                                                               double i,
                                                               double lpi,
                                                               double lpj)
  : HingeBeamIntegration2d(BEAM_INTEGRATION_TAG_HingeRadauTwo2d,e,a,i,lpi,lpj) {}

XC::HingeRadauTwoBeamIntegration2d::HingeRadauTwoBeamIntegration2d(void)
  : HingeBeamIntegration2d(BEAM_INTEGRATION_TAG_HingeRadauTwo2d){}

void XC::HingeRadauTwoBeamIntegration2d::getSectionLocations(int numSections, double L,double *xi) const
{
  double two3oneOverL = (2.0/3.0)/L;

  xi[0] = 0.0;
  xi[1] = lpI*two3oneOverL;
  xi[2] = 1.0-lpJ*two3oneOverL;
  xi[3] = 1.0;
  for (int i = 4; i < numSections; i++)
    xi[i] = 0.0;
}

void XC::HingeRadauTwoBeamIntegration2d::getSectionWeights(int numSections, double L,double *wt) const
{
  double oneOverL = 1.0/L;

  wt[0] = 0.25*lpI*oneOverL;
  wt[1] = 3.0*wt[0];
  wt[3] = 0.25*lpJ*oneOverL;
  wt[2] = 3.0*wt[3];
  for (int i = 4; i < numSections; i++)
    wt[i] = 1.0;
}

int
XC::HingeRadauTwoBeamIntegration2d::addElasticFlexibility(double L, Matrix &fElastic)
{
  double oneOverL = 1.0/L;

  // Length of elastic interior
  double Le = L-lpI-lpJ;
  double LoverEA  = Le/(E*A);
  double Lover3EI = Le/(3*E*I);
  double Lover6EI = 0.5*Lover3EI;
  
  // Elastic flexibility of element interior
  static XC::Matrix fe(2,2);
  fe(0,0) = fe(1,1) =  Lover3EI;
  fe(0,1) = fe(1,0) = -Lover6EI;
  
  // Equilibrium transformation matrix
  static XC::Matrix B(2,2);
  double betaI = lpI*oneOverL;
  double betaJ = lpJ*oneOverL;
  B(0,0) = 1.0 - betaI;
  B(1,1) = 1.0 - betaJ;
  B(0,1) = -betaI;
  B(1,0) = -betaJ;
  
  // Transform the elastic flexibility of the element
  // interior to the basic system
  static XC::Matrix ftmp(2,2);
  ftmp.addMatrixTripleProduct(0.0, B, fe, 1.0);

  fElastic(0,0) += LoverEA;
  fElastic(1,1) += ftmp(0,0);
  fElastic(1,2) += ftmp(0,1);
  fElastic(2,1) += ftmp(1,0);
  fElastic(2,2) += ftmp(1,1);

  return -1;
}

double XC::HingeRadauTwoBeamIntegration2d::getTangentDriftI(double L, double LI,double q2, double q3)
{
  double oneOverL = 1.0/L;

  double betaI = lpI*oneOverL;

  double qq2 = (1-betaI)*q2 - betaI*q3;

  if (LI < lpI)
    return 0.0;
  else
    return (LI-lpI)/3*(LI-lpI)*qq2/(E*I);
}

double
XC::HingeRadauTwoBeamIntegration2d::getTangentDriftJ(double L, double LI,
                                                 double q2, double q3)
{
  double oneOverL = 1.0/L;

  double betaJ = lpJ*oneOverL;

  double qq3 = (1-betaJ)*q3 - betaJ*q2;

  if (LI > L-lpJ)
    return 0.0;
  else
    return (L-LI-lpJ)/3*(L-LI-lpJ)*qq3/(E*I);
}

//! @brief Constructor virtual.
XC::BeamIntegration *XC::HingeRadauTwoBeamIntegration2d::getCopy(void) const
  { return new HingeRadauTwoBeamIntegration2d(*this); }

//! @brief Envía el objeto a través del canal que se pasa como parámetro.
int XC::HingeRadauTwoBeamIntegration2d::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(3);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << nombre_clase() << "sendSelf() - failed to send data\n";
    return res;
  }

//! @brief Recibe el objeto a través del canal que se pasa como parámetro.
int XC::HingeRadauTwoBeamIntegration2d::recvSelf(const CommParameters &cp)
  {
    inicComm(3);
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


int XC::HingeRadauTwoBeamIntegration2d::setParameter(const std::vector<std::string> &argv, Parameter &param)
  {
    // E of the beam interior
    if(argv[0] == "E")
      return param.addObject(1, this);
    // A of the beam interior
    if(argv[0] == "A")
      return param.addObject(2, this);
    // I of the beam interior
    if(argv[0] == "I")
      return param.addObject(3, this);
    if(argv[0] == "lpI")
      return param.addObject(4, this);
    else if(argv[0] == "lpJ")
      return param.addObject(5, this);
    else 
      return -1;
  }

int
XC::HingeRadauTwoBeamIntegration2d::updateParameter(int parameterID,
                                                Information &info)
{
  switch (parameterID) {
  case 1:
    E = info.theDouble;
    return 0;
  case 2:
    A = info.theDouble;
    return 0;
  case 3:
    I = info.theDouble;
    return 0;
  case 4:
    lpI = info.theDouble;
    return 0;
  case 5:
    lpJ = info.theDouble;
    return 0;
  default:
    return -1;
  }
}

int
XC::HingeRadauTwoBeamIntegration2d::activateParameter(int parameterID)
{
  // For Terje to do
  return 0;
}

void XC::HingeRadauTwoBeamIntegration2d::Print(std::ostream &s, int flag)
  {
    s << "HingeRadauTwo2d" << std::endl;
    HingeBeamIntegration2d::Print(s);
    return;
  }