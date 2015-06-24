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
** Reliability module developed by:                                   **
**   Terje Haukaas (haukaas@ce.berkeley.edu)                          **
**   Armen Der Kiureghian (adk@ce.berkeley.edu)                       **
**                                                                    **
** ****************************************************************** */
                                                                        
// $Revision: 1.2 $
// $Date: 2004/08/27 17:55:37 $
// $Source: /usr/local/cvs/OpenSees/SRC/reliability/analysis/analysis/ParametricReliabilityAnalysis.cpp,v $


//
// Written by Terje Haukaas (haukaas@ce.berkeley.edu)
//

#include <reliability/analysis/analysis/ParametricReliabilityAnalysis.h>
#include <reliability/analysis/analysis/ReliabilityAnalysis.h>
#include <reliability/domain/components/ReliabilityDomain.h>
#include <reliability/analysis/designPoint/FindDesignPointAlgorithm.h>
#include <reliability/analysis/sensitivity/GradGEvaluator.h>
#include <reliability/domain/components/ParameterPositioner.h>
#include <reliability/domain/distributions/NormalRV.h>
#include <tcl.h>

#include <fstream>
#include <iomanip>
#include <iostream>
using std::ifstream;
using std::ios;
using std::setw;
using std::setprecision;
using std::setiosflags;

XC::ParametricReliabilityAnalysis::ParametricReliabilityAnalysis(ReliabilityDomain *passedReliabilityDomain,
                                                                         FindDesignPointAlgorithm *passedFindDesignPointAlgorithm,
                                                                         GradGEvaluator *passedGradGEvaluator,
                                                                         int pParameterNumber,
                                                                         double pFirst,
                                                                         double pLast,
                                                                         int pNumIntervals,
                                                                 const std::string &passedFileName,
                                                                         Tcl_Interp *passedTclInterp)
:ReliabilityAnalysis()
{
        parameterNumber = pParameterNumber;
        first = pFirst;
        last = pLast;
        numIntervals = pNumIntervals;

        theReliabilityDomain = passedReliabilityDomain;
        theFindDesignPointAlgorithm = passedFindDesignPointAlgorithm;
        theGradGEvaluator = passedGradGEvaluator;

        fileName= passedFileName;

        theTclInterp = passedTclInterp;
}


int 
XC::ParametricReliabilityAnalysis::analyze(void)
{
        
        // Alert the user that the FORM analysis has started
        std::cerr << "Fragility XC::Analysis is running ... " << std::endl;


        // The relevant commands are now, for instance, given like this
        // performanceFunction 1 "{par_1}-{u_7_1}"
        // runParametricReliabilityAnalysis output.out -par 1 -range 14.0 16.0 -num 10


        // Open output file
        ofstream outputFile(fileName.c_str(), ios::out );


        // Get number of limit-state functions
        int numLsf = theReliabilityDomain->getNumberOfLimitStateFunctions();

        // Initial declarations
        Vector pf(numIntervals+1);
        Vector pdf(numIntervals+1);
        Vector uStar, alpha;
        double beta;
        NormalRV aStdNormRV(1,0.0,1.0,0.0);
        Matrix dGdPar;
        int numPars;
        Vector gradient;
        double currentValue;
        Vector currentValues(numIntervals+1);
        int numPos;
        ParameterPositioner *theParameterPositioner = 0;


        // Loop over number of limit-state functions and perform FORM analysis
        for (int lsf=1; lsf<=numLsf; lsf++ ) {


                // Inform the user which limit-state function is being evaluated
                std::cerr << "Limit-state function number: " << lsf << std::endl;


                // Set tag of "active" limit-state function
                theReliabilityDomain->setTagOfActiveLimitStateFunction(lsf);


                // "Download" limit-state function from reliability domain
                // fmk to Terje: you just set it so why do you need the tag again
                //     also you can't do a redef inside a loop with the same def as loop variable!!!!
                // => changing int lst to int newLsf in line below
                const int newLsf= theReliabilityDomain->getTagOfActiveLimitStateFunction();
                LimitStateFunction *theLimitStateFunction = theReliabilityDomain->getLimitStateFunctionPtr(lsf);
                if(theLimitStateFunction == 0)
                  {
                    std::cerr << "ParametricReliabilityAnalysis::analyze() - could not find" << std::endl
                              << " limit-state function with tag #" << lsf << "." << std::endl;
                    return -1;
                }


                // Print results to output file
                outputFile << "#######################################################################" << std::endl;
                outputFile << "#  FORM ANALYSIS RESULTS, LIMIT-STATE FUNCTION NUMBER "
                        <<setiosflags(ios::left)<<setprecision(1)<<setw(4)<<lsf <<"            #" << std::endl;
                outputFile << "#                                                                     #" << std::endl;
                outputFile << "#                                                                     #" << std::endl;
                outputFile << "#                    Failure probability     Estimated probability    #" << std::endl;
                outputFile << "#    Parameter       estimate (fragility)     densitity function      #" << std::endl;
                outputFile << "#     value               (CDF)                    (PDF)              #" << std::endl;
                outputFile.setf(ios::scientific, ios::floatfield);
                outputFile.flush();


                // Range over parameter values
                currentValue = first;
                for (int counter=0; counter<(numIntervals+1); counter++) {

                        currentValues(counter) = currentValue;

                        // Detect parameter and set value, first the 
                        // case where the parameter is represented by a 
                        // parameterPositioner
                        numPos = theReliabilityDomain->getNumberOfParameterPositioners();
                        if (numPos != 0) {
                                theParameterPositioner = theReliabilityDomain->getParameterPositionerPtr(parameterNumber);
                                if (theParameterPositioner == 0) {
                                        std::cerr << "XC::ParametricReliabilityAnalysis::analyze() -- The parameter number in the " << std::endl
                                                << " fragility analysis object does not match the parameter number " << std::endl
                                                << " being set by the parameter positioner." << std::endl;
                                }
                        }
                        else {
                                char separators[5] = "_}{";
                                const std::string theExpression = theLimitStateFunction->getExpression();
                                char *lsf_forTokenizing = new char[1000];
                                strcpy(lsf_forTokenizing,theExpression.c_str());
                                char *tokenPtr = strtok( lsf_forTokenizing, separators);
                                while ( tokenPtr != nullptr ) {
                                        if ( strcmp(tokenPtr, "par") == 0) {
                                                tokenPtr = strtok( nullptr, separators);
                                                int par = atoi( tokenPtr );
                                                if (par==parameterNumber) {
                                                        char tclAssignment[100];
                                                        sprintf(tclAssignment , "set par_%d  %15.5f", parameterNumber, currentValue);
                                                        Tcl_Eval( theTclInterp, tclAssignment);
                                                }
                                                else {
                                                        std::cerr << "XC::ParametricReliabilityAnalysis::analyze() -- The parameter number " << std::endl
                                                                << " in the limit-state function does not match the parameter " << std::endl
                                                                << " number in the fragility analysis object." << std::endl;
                                                }
                                        }
                                        tokenPtr = strtok( nullptr, separators);
                                }
                        }


                        // Possibly set the parameter value in the FE domain
                        if (theParameterPositioner != 0) {
                                theParameterPositioner->update(currentValue);
                        }


                        // Find the design point
                        if (theFindDesignPointAlgorithm->findDesignPoint(theReliabilityDomain) < 0){

                                // Set detectable 'crazy' values when the design point wasn't found
                                pf(counter) = -1.0;
                                pdf(counter) = -1.0;
                        }
                        else
                          {

                            // Store probability of failure
                            uStar= theFindDesignPointAlgorithm->get_u();
                            alpha= theFindDesignPointAlgorithm->get_alpha();
                            beta = alpha ^ uStar;
                            pf(counter) = 1.0 - aStdNormRV.getCDFvalue(beta);

                            // Compute PDF, first; derivative of lsf wrt. parameter
                            dGdPar = theGradGEvaluator->getDgDpar();

                            double thedGdPar= 0.0;
                            // Find the right element
                            numPars = dGdPar.noRows();
                            for(int i=0; i<numPars; i++)
                              {
                                if(((int)(dGdPar(i,0))) == parameterNumber)
                                  { thedGdPar = dGdPar(i,1); }
                              }
                            // Gradient of limit-state function
                            gradient = theFindDesignPointAlgorithm->getGradientInStandardNormalSpace();
                            // Compute PDF value
                            pdf(counter) = fabs( aStdNormRV.getPDFvalue(-beta) / gradient.Norm() * thedGdPar );
                          }

                        currentValue = currentValue + (last-first)/numIntervals;


                        // Print results to output file
                        outputFile << "#   " << setprecision(3)<<setw(11)<<currentValues(counter)<<"         ";
                        if (pf(counter)==-1.0) {
                                outputFile << "--failed--              ";
                                outputFile << "--failed--            #" << std::endl;
                        }
                        else {
                                outputFile <<setprecision(3)<<setw(11)<<pf(counter)<<"             ";
                                outputFile <<setprecision(3)<<setw(11)<<pdf(counter)<<"           #" << std::endl;
                        }
                        outputFile.flush();

                } // Done looping over parameter range


                outputFile << "#                                                                     #" << std::endl;
                outputFile << "#######################################################################" << std::endl << std::endl << std::endl;

        } // Done looping over limit-state functions
                

        // Print summary of results to screen (more here!!!)
        std::cerr << "Fragility XC::Analysis completed." << std::endl;


        // Clean up
        outputFile.close();

        return 0;
}
