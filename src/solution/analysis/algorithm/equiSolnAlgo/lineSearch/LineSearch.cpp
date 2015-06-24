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
** file 'COPYRIGHT'  in XC::main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.1 $
// $Date: 2001/12/07 01:02:09 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/algorithm/equiSolnAlgo/LineSearch.cpp,v $
                                                                        
// Written: fmk 
// Created: 11/01

// Description: This file contains the class implementation for 
// LineSearch. 
// 
// What: "@(#)LineSearch.C, revA"

#include <solution/analysis/algorithm/equiSolnAlgo/lineSearch/LineSearch.h>
#include <utility/matrix/Vector.h>
#include <solution/system_of_eqn/linearSOE/LinearSOE.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"

XC::LineSearch::LineSearch(int clasTag,const double &tol, const int &mi,const double &mneta,const double &mxeta,const int &flag)
  :MovableObject(clasTag), tolerance(tol), maxIter(mi), minEta(mneta), maxEta(mxeta),printFlag(flag) {}

int XC::LineSearch::newStep(LinearSOE &theSOE)
  {
    const Vector &dU = theSOE.getX();

    if(x.Size() != dU.Size())
      x= dU;
    return 0;
  }

//! @brief Lee un objeto XC::LineSearch desde archivo
bool XC::LineSearch::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(LineSearch) Procesando comando: " << cmd << std::endl;
    if(cmd == "tol")
      {
        tolerance= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "maxIter")
      {
        maxIter= interpretaInt(status.GetString());
        return true;
      }
    else if(cmd == "minEta")
      {
        minEta= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "maxEta")
      {
        maxEta= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "print_flag")
      {
        printFlag= interpretaInt(status.GetString());
        return true;
      }
    else
      return EntCmd::procesa_comando(status);
  }

int XC::LineSearch::sendSelf(CommParameters &cp)
  { return 0; }

int XC::LineSearch::recvSelf(const CommParameters &cp)
  { return 0; }

void XC::LineSearch::Print(std::ostream &s, int flag)
  {
    if(flag == 0)
      {
        s << nombre_clase() << " :: Line Search Tolerance = " << tolerance << std::endl; 
        s << "                         max num Iterations = " << maxIter << std::endl;
        s << "                         min value on eta = " << minEta << std::endl;
        s << "                         max value on eta = " << maxEta << std::endl;
      }
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::LineSearch::GetProp(const std::string &cod) const
  {
    if(cod=="getTol")
      return any_const_ptr(tolerance);
    else if(cod=="getMaxIter")
      return any_const_ptr(maxIter);
    else if(cod=="getMinEta")
      return any_const_ptr(minEta);
    else if(cod=="getMaxEta")
      return any_const_ptr(maxEta);
    else
      return EntCmd::GetProp(cod);
  }