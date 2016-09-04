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
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */

// $Revision: 1.4 $
// $Date: 2005/12/15 00:19:28 $
// $Source: /usr/local/cvs/OpenSees/SRC/convergenceTest/CTestRelativeNormUnbalance.cpp,v $


#include <solution/analysis/convergenceTest/CTestRelativeNormUnbalance.h>
#include <solution/analysis/algorithm/equiSolnAlgo/EquiSolnAlgo.h>
#include <solution/system_of_eqn/linearSOE/LinearSOE.h>


XC::CTestRelativeNormUnbalance::CTestRelativeNormUnbalance(EntCmd *owr)	    	
  : ConvergenceTestNorm(owr,CONVERGENCE_TEST_CTestRelativeNormUnbalance) {}


XC::CTestRelativeNormUnbalance::CTestRelativeNormUnbalance(EntCmd *owr,double theTol, int maxIter, int printIt, int normType)
  : ConvergenceTestNorm(owr,CONVERGENCE_TEST_CTestRelativeNormUnbalance,theTol,maxIter,printIt,normType,maxIter+1) {}

XC::ConvergenceTest* XC::CTestRelativeNormUnbalance::getCopy(void) const
  { return new CTestRelativeNormUnbalance(*this); }

//! @brief Returns a message showing the values of the principal parameters.
std::string XC::CTestRelativeNormUnbalance::getStatusMsg(const int &flag) const
  {
    std::string retval= getTestIterationMessage();
    retval+= getRatioMessage("(|dR|/|dR0|)");
    if(flag>=4)
      {
	retval+= getDeltaXRNormsMessage();
        retval+= getDeltaXRMessage();
      }
    return retval;
  }

//! @brief Comprueba si se ha producido la convergencia.
int XC::CTestRelativeNormUnbalance::test(void)
  {
    // check to ensure the SOE has been set - this should not happen if the 
    // return from start() is checked
    if(!hasLinearSOE()) return -2;
    
    // check to ensure the algo does invoke start() - this is needed otherwise
    // may never get convergence later on in analysis!
    if(currentIter == 0)
      {
        std::cerr << "WARNING: XC::CTestRelativeNormUnbalance::test() - start() was never invoked.\n";	
        return -2;
      }
    
    // get the B vector & determine it's norm & save the value in norms vector
    calculatedNormX= getNormX();
    calculatedNormB = getNormB();
    lastRatio= calculatedNormB;
    if(currentIter <= maxNumIter) 
      norms(currentIter)= calculatedNormB;
    
    // determine the ratio
    if(norm0 != 0.0)
      lastRatio/= norm0;
    
    // print the data if required
    if(printFlag)
      std::clog << getStatusMsg(printFlag);
    
    //
    // check if the algorithm converged
    //
    
    // if converged - print & return ok
    
    if(lastRatio <= tol)
      { // the algorithm converged  
        // do some printing first
        if(printFlag != 0)
          {
            if(printFlag == 1 || printFlag == 4) 
              std::cerr << std::endl;
            else if(printFlag == 2 || printFlag == 6)
              {
                std::cerr << getTestIterationMessage();
                std::cerr << getRatioMessage("(|dR|/|dR0|)");
              }
          }
        // return the number of times test has been called
        return currentIter;
      } 
    // algo failed to converged after specified number of iterations - but RETURN OK
    else if((printFlag == 5 || printFlag == 6) && currentIter >= maxNumIter)
      {
        std::cerr << "WARNING: XC::CTestRelativeNormUnbalance::test() - failed to converge but going on -";
        std::cerr << getRatioMessage("(dR/dR0)");
        return currentIter;
      }
    
    // algo failed to converged after specified number of iterations - return FAILURE -2
    else if(currentIter >= maxNumIter)
      { // the algorithm failed to converge
        std::cerr << getFailedToConvergeMessage();
        currentIter++;  // we increment in case analysis does not check for convergence
        return -2;
      } 
    // algorithm not yet converged - increment counter and return -1
    else
      {
        currentIter++;    
        return -1;
      }
  }


int XC::CTestRelativeNormUnbalance::start(void)
  {
    ConvergenceTestNorm::start();
    
    // determine the initial norm .. the the norm of the initial unbalance
    calculatedNormB= getNormB();
    if(currentIter <= maxNumIter) 
      norms(0)= calculatedNormB;
    norm0= calculatedNormB;
    return 0;
  }
