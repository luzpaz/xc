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

// $Revision: 1.1 $
// $Date: 2005/12/15 00:13:51 $
// $Source: /usr/local/cvs/OpenSees/SRC/convergenceTest/CTestFixedNumIter.cpp,v $

// Written: Andreas Schellenberg (andreas.schellenberg@gmx.net)
// Created: 09/05
// Revision: A
//
// Purpose: This file contains the class implementation of XC::CTestFixedNumIter.
//
// What: "@(#) CTestFixedNumIter.cpp, revA"


#include <solution/analysis/convergenceTest/CTestFixedNumIter.h>
#include <solution/analysis/algorithm/equiSolnAlgo/EquiSolnAlgo.h>
#include <solution/system_of_eqn/linearSOE/LinearSOE.h>


XC::CTestFixedNumIter::CTestFixedNumIter(EntCmd *owr)	    	
  : ConvergenceTest(owr,CONVERGENCE_TEST_CTestFixedNumIter)
  {}


XC::CTestFixedNumIter::CTestFixedNumIter(EntCmd *owr,int maxIter, int printIt, int normType)
  : ConvergenceTest(owr,CONVERGENCE_TEST_CTestFixedNumIter,maxIter,printIt,normType,maxIter)
  {}

XC::ConvergenceTest* XC::CTestFixedNumIter::getCopy(void) const
  { return new CTestFixedNumIter(*this); }

//! @brief Comprueba si se ha producido la convergencia.
int XC::CTestFixedNumIter::test(void)
  {
    // check to ensure the SOE has been set - this should not happen if the 
    // return from start() is checked
    if(!hasLinearSOE()) return -2;
    
    // check to ensure the algo does invoke start() - this is needed otherwise
    // may never get convergence later on in analysis!
    if(currentIter == 0)
      {
        std::cerr << "WARNING: XC::CTestFixedNumIter::test() - start() was never invoked.\n";	
        return -2;
      }
        
    // determine the energy & save value in norms vector
    const Vector &b= getB();
    const Vector &x= getX();    
    double product = x ^ b;
    if(product < 0.0)
      product *= -0.5;
    else
      product *= 0.5;
    
    if(currentIter <= maxNumIter) 
      norms(currentIter-1)= product;

    // print the data if required
    if(printFlag == 1)
      {
        std::clog << "XC::CTestFixedNumIter::test() - iteration: " << currentIter;
        std::clog << " current EnergyIncr: " << product;
        std::clog << " (Norm deltaX: " << x.pNorm(nType) << ", Norm deltaR: " << b.pNorm(nType) << ")\n";
      } 
    if(printFlag == 4)
      {
        std::clog << "XC::CTestFixedNumIter::test() - iteration: " << currentIter;
        std::clog << " current EnergyIncr: " << product;
        std::clog << " (Norm deltaX: " << x.pNorm(nType) << ", Norm deltaR: " << b.pNorm(nType) << ")\n";
        std::clog << "\tdeltaX: " << x << "\tdeltaR: " << b;
      } 
    
    //
    // check if the algorithm converged
    //
    
    // if converged - print & return ok
    if(currentIter == maxNumIter)
      {   
        // do some printing first
        if(printFlag != 0)
          {
            if(printFlag == 1 || printFlag == 4) 
              std::clog << std::endl;
            else if(printFlag == 2 || printFlag == 6)
              {
                std::clog << "XC::CTestFixedNumIter::test() - iteration: " << currentIter;
                std::clog << " last EnergyIncr: " << product;
                std::clog << " (Norm deltaX: " << x.pNorm(nType) << ", Norm deltaR: " << b.pNorm(nType) << ")\n";
              }
          }
        // return the number of times test has been called
        return currentIter;
      }  
    // algorithm not yet converged - increment counter and return -1
    else
      {
        currentIter++;    
        return -1;
      }
  }

