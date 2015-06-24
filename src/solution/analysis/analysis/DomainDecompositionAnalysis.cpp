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
                                                                        
// $Revision: 1.6 $
// $Date: 2005/11/29 23:36:47 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/analysis/DomainDecompositionAnalysis.cpp,v $
                                                                        
                                                                        
// File: ~/analysis/Analysis/DomainDecompositionAnalysis.C
// 
// Written: fmk 
// Created: Tue Sept 17 16:34:47: 1996
// Revision: A
//
// Description: This file contains the class definition for 
// DomainDecompositionAnalysis. DomainDecompositionAnalysis is a subclass 
// of AnalysisAnalysis, it is used to perform the static condensation process
// on a subdomain.
//
// What: "@(#) DomainDecompositionAnalysis.C, revA"


#include <solution/analysis/analysis/DomainDecompositionAnalysis.h>
#include "solution/analysis/ModelWrapper.h"
#include "solution/SoluMethod.h"
#include <solution/analysis/handler/ConstraintHandler.h>
#include <solution/analysis/numberer/DOF_Numberer.h>
#include <solution/analysis/model/AnalysisModel.h>
#include <solution/system_of_eqn/linearSOE/LinearSOE.h>
#include <solution/analysis/algorithm/domainDecompAlgo/DomainDecompAlgo.h>
#include <solution/analysis/algorithm/equiSolnAlgo/EquiSolnAlgo.h>
#include <solution/system_of_eqn/linearSOE/DomainSolver.h>
#include <solution/analysis/convergenceTest/ConvergenceTest.h>
#include <solution/analysis/integrator/IncrementalIntegrator.h>
#include "domain/domain/subdomain/Subdomain.h"

#include <solution/analysis/model/dof_grp/DOF_Group.h>
#include <utility/matrix/ID.h>
#include <domain/mesh/node/Node.h>

//! @brief Asigna los punteros al DomainDecompSolutionAlgorithm y al subdominio.
void XC::DomainDecompositionAnalysis::set_all_links(void)
  {
    theSubdomain->setDomainDecompAnalysis(*this);
  }

//! @brief Constructor.
XC::DomainDecompositionAnalysis::DomainDecompositionAnalysis(Subdomain &the_Domain,SoluMethod *s)
  : Analysis(s),
    MovableObject(DomDecompANALYSIS_TAGS_DomainDecompositionAnalysis),
    theSubdomain(&the_Domain),
    theSolver(nullptr),
    numEqn(0),numExtEqn(0),tangFormed(false),tangFormedCount(0),
    domainStamp(0)
  {
    theSubdomain->setDomainDecompAnalysis(*this);
  }

//! @brief Constructor.
XC::DomainDecompositionAnalysis::DomainDecompositionAnalysis(Subdomain &the_Domain,DomainSolver &theSlvr,SoluMethod *s)
  : Analysis(s),
    MovableObject(DomDecompANALYSIS_TAGS_DomainDecompositionAnalysis),
    theSubdomain(&the_Domain),
    theSolver(&theSlvr),
    numEqn(0),numExtEqn(0),tangFormed(false),tangFormedCount(0),
    domainStamp(0)
  {
    theSubdomain->setDomainDecompAnalysis(*this);
  }

//! @brief Constructor.
XC::DomainDecompositionAnalysis::DomainDecompositionAnalysis(int clsTag, Subdomain &the_Domain,SoluMethod *s)
  : Analysis(s),
    MovableObject(clsTag),
    theSubdomain(&the_Domain),
    theSolver(nullptr), numEqn(0),numExtEqn(0),tangFormed(false),tangFormedCount(0),
    domainStamp(0) {}

//! @brief Constructor.
XC::DomainDecompositionAnalysis::DomainDecompositionAnalysis(int clsTag, Subdomain &theDomain,DomainSolver &theSolver,SoluMethod *s)
  : Analysis(s),
    MovableObject(clsTag),
    theSubdomain(&theDomain),
    theSolver(&theSolver), numEqn(0),numExtEqn(0),tangFormed(false),tangFormedCount(0),
    domainStamp(0) {}


//! @brief Borra todos los miembros del objeto (Manejador coacciones, modelo de análisis,...).
void XC::DomainDecompositionAnalysis::clearAll(void)
  {
    // invoke the destructor on all the objects in the aggregation
    Analysis::clearAll();
  }

//! @brief Ejecuta el análisis.
int XC::DomainDecompositionAnalysis::analyze(double dT)
  { return 0; }

int XC::DomainDecompositionAnalysis::initialize(void)
  { return 0; }

//! @brief Devuelve un puntero al DomainSolver.
const XC::DomainSolver *XC::DomainDecompositionAnalysis::getDomainSolver(void) const
  { return theSolver; }

//! @brief Devuelve un puntero al DomainSolver.
XC::DomainSolver *XC::DomainDecompositionAnalysis::getDomainSolver(void)
  { return theSolver; }

//! @brief Devuelve un puntero al subdominio.
const XC::Subdomain *XC::DomainDecompositionAnalysis::getSubdomain(void) const
  { return theSubdomain; }

//! @brief Devuelve un puntero al subdominio.
XC::Subdomain *XC::DomainDecompositionAnalysis::getSubdomain(void)
  { return theSubdomain; }

bool XC::DomainDecompositionAnalysis::doesIndependentAnalysis(void)
  { return false; }

//! @brief Ejecuta los cambios que implica un cambio en el dominio del problema.
int XC::DomainDecompositionAnalysis::domainChanged(void)
  {
    // remove existing FE_elements and DOF_Groups from the Analysis
    getAnalysisModelPtr()->clearAll();
    getConstraintHandlerPtr()->clearAll();

    // now we invoke handle() on the constraint handler which
    // causes the creation of XC::FE_Element and XC::DOF_Group objects
    // and their addition to the AnalysisModel.

    numExtEqn = getConstraintHandlerPtr()->handle(&(theSubdomain->getExternalNodes()));

    // we now get a node to number last

    const XC::ID &theExtNodes = theSubdomain->getExternalNodes();
    int idSize = theExtNodes.Size();
    //    int theLastDOF = -1;

    ID theLastDOFs(1);
    int cnt = 0;

    // create an XC::ID containing the tags of the DOF_Groups that are to
    // be numbered last
    for (int i=0; i<idSize; i++) {
	int nodeTag = theExtNodes(i);
	Node *nodePtr = theSubdomain->getNode(nodeTag);
	DOF_Group *dofGrpPtr = nodePtr->getDOF_GroupPtr();
	if (dofGrpPtr != 0) {
	    const XC::ID theID = dofGrpPtr->getID();
	    int size = theID.Size();
	    for (int j=0; j<size; j++)
		if (theID(j) == -3) {
		    theLastDOFs[cnt]  = dofGrpPtr->getTag();
		    cnt++;
		    j = size;
		}
	}
    }

    // we now invoke number() on the numberer which causes
    // equation numbers to be assigned to all the DOFs in the
    // AnalysisModel.    

    getDOF_NumbererPtr()->numberDOF(theLastDOFs);

    // we invoke setSize() on the XC::LinearSOE which
    // causes that object to determine its size    
    
    getLinearSOEPtr()->setSize(getAnalysisModelPtr()->getDOFGraph());    
    numEqn = getLinearSOEPtr()->getNumEqn();

    // we invoke domainChange() on the integrator and algorithm

    getIncrementalIntegratorPtr()->domainChanged();
    getDomainDecompSolutionAlgorithmPtr()->domainChanged();        

    // now set the variables to indicate that tangent has not been formed

    tangFormed = false;
    tangFormedCount = 0;
    
    return 0;
  }

//! @brief Devuelve el número de ecuaciones externas.
int XC::DomainDecompositionAnalysis::getNumExternalEqn(void)
  { return numExtEqn; }

//! @brief Devuelve el númer de ecuaciones internas.
int XC::DomainDecompositionAnalysis::getNumInternalEqn(void)
  { return numEqn-numExtEqn; }


int XC::DomainDecompositionAnalysis::newStep(double dT)
  { return getIncrementalIntegratorPtr()->newStep(dT); }



int XC::DomainDecompositionAnalysis::computeInternalResponse(void)
  {  return getDomainDecompSolutionAlgorithmPtr()->solveCurrentStep(); }



//! @brief Forma la matriz de rigidez tangente.
int XC::DomainDecompositionAnalysis::formTangent(void)
  {
    int result =0;

    Domain *the_Domain = this->getDomainPtr();

    // we check to see if the domain has changed 
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	this->domainChanged();
      }
    
    // if tangFormed == -1 then formTangent has already been
    // called for this state by formResidual() or formTangVectProduct()
    // so we won't be doing it again.

    if(tangFormedCount != -1)
      {
	result = getIncrementalIntegratorPtr()->formTangent();
	if(result < 0)
	  return result;
	result = theSolver->condenseA(numEqn-numExtEqn);
	if(result < 0)
	  return result;
      }
	
    tangFormed = true;
    tangFormedCount++;
    
    return result;
  }


//! @brief Forma el vector residuo.
int XC::DomainDecompositionAnalysis::formResidual(void)
  {
    int result =0;
    Domain *the_Domain = this->getDomainPtr();    
    
    // we check to see if the domain has changed 
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	this->domainChanged();
      }
    
    if(tangFormed == false)
      {
	result = this->formTangent();
	if(result < 0)
	  return result;
	tangFormedCount = -1; // set to minus number so tangent 
	                      // is not formed twice at same state
      }

    result = getIncrementalIntegratorPtr()->formUnbalance();

    if(result < 0)
      return result;
    return theSolver->condenseRHS(numEqn-numExtEqn);
  }


//! @brief ??
int XC::DomainDecompositionAnalysis::formTangVectProduct(Vector &u)
  {
    int result = 0;

    Domain *the_Domain = this->getDomainPtr();
    
    // we check to see if the domain has changed 
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	this->domainChanged();
      }
    
    if(tangFormed == false)
      {
	result = this->formTangent();
	if (result < 0)
	    return result;
	tangFormedCount = -1; // set to minus number so tangent 
	                      // is not formed twice at same state
      }    
    return theSolver->computeCondensedMatVect(numEqn-numExtEqn,u);
  }


//! @brief Devuelve la matriz de rigidez tangente.
const XC::Matrix &XC::DomainDecompositionAnalysis::getTangent(void)
  {
    Domain *the_Domain = this->getDomainPtr();
    
    // we check to see if the domain has changed 
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	this->domainChanged();
      }

    if(tangFormed == false)
      {	this->formTangent(); }
    return (theSolver->getCondensedA());
  }


//! @brief Devuelve el vector residuo.
const XC::Vector &XC::DomainDecompositionAnalysis::getResidual(void)
  {

    Domain *the_Domain = this->getDomainPtr();
    
    // we check to see if the domain has changed 
    // we check to see if the domain has changed 
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	this->domainChanged();
	this->formResidual();	
      }    
    theResidual= theSolver->getCondensedRHS();
    return theResidual;
  }


const XC::Vector &XC::DomainDecompositionAnalysis::getTangVectProduct()
  {
    Domain *the_Domain = this->getDomainPtr();
    
    // we check to see if the domain has changed 
    int stamp = the_Domain->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	this->domainChanged();
      }
    return theSolver->getCondensedMatVect();
  }

int XC::DomainDecompositionAnalysis::sendSelf(CommParameters &cp)
  {
    // determine the type of each object in the aggregation,
    // store it in an XC::ID and send the info off.
    getDbTag(cp);
    ID data(14);
    data(0)= getConstraintHandlerPtr()->getClassTag();
    data(1)= getDOF_NumbererPtr()->getClassTag();
    data(2)= getAnalysisModelPtr()->getClassTag();
    data(3)= getDomainDecompSolutionAlgorithmPtr()->getClassTag();
    data(4)= getIncrementalIntegratorPtr()->getClassTag();    
    data(5)= getLinearSOEPtr()->getClassTag();    
    data(6)= theSolver->getClassTag();        

    data(7)= getConstraintHandlerPtr()->getDbTag();
    data(8)= getDOF_NumbererPtr()->getDbTag();
    data(9)= getAnalysisModelPtr()->getDbTag();
    data(10)= getDomainDecompSolutionAlgorithmPtr()->getDbTag();
    data(11)= getIncrementalIntegratorPtr()->getDbTag();    
    data(12)= getLinearSOEPtr()->getDbTag();    
    data(13)= theSolver->getDbTag();        

    cp.sendID(data,getDbTagData(),CommMetaData(0));

    // invoke sendSelf on each object in the aggregation
    
    getConstraintHandlerPtr()->sendSelf(cp);
    getDOF_NumbererPtr()->sendSelf(cp);
    getAnalysisModelPtr()->sendSelf(cp);
    getDomainDecompSolutionAlgorithmPtr()->sendSelf(cp);
    getIncrementalIntegratorPtr()->sendSelf(cp);    
    getLinearSOEPtr()->sendSelf(cp);    
    theSolver->sendSelf(cp);            
    return 0;
  }

int XC::DomainDecompositionAnalysis::recvSelf(const CommParameters &cp)
  {
    // receive the data identifyng the objects in the aggregation
    ID data(14);
    getDbTag();
    cp.receiveID(data,getDbTagData(),CommMetaData(0));

    //
    // now ask the object broker an object of each type
    // and invoke recvSelf() on the object to init it.
    //

    brokeConstraintHandler(cp,ID(data(0)));
    if(getConstraintHandlerPtr())
      {
	getConstraintHandlerPtr()->setDbTag(data(7));
	getConstraintHandlerPtr()->recvSelf(cp);
      }
    else
      {
	std::cerr << "DomainDecompositionAnalysis::recvSelf";
	std::cerr << " - failed to get the XC::ConstraintHandler\n";
	return -1;
      }

    brokeNumberer(cp,data);
    if(getDOF_NumbererPtr())
      {
	getDOF_NumbererPtr()->setDbTag(data(8));	
	getDOF_NumbererPtr()->recvSelf(cp);
      }
    else
      {
	std::cerr << "DomainDecompositionAnalysis::recvSelf";
	std::cerr << " - failed to get the DOF Numberer\n";
	return -1;
      }


    brokeAnalysisModel(cp,ID(data(2)));
    if(getAnalysisModelPtr())
      {
	getAnalysisModelPtr()->setDbTag(data(9));
	getAnalysisModelPtr()->recvSelf(cp);
      }
    else {
	std::cerr << "DomainDecompositionAnalysis::recvSelf";
	std::cerr << " - failed to get the AnalysisModel\n";
	return -1;
    }        


    brokeDomainDecompAlgo(cp,data);
    if(getDomainDecompSolutionAlgorithmPtr())
      {
	getDomainDecompSolutionAlgorithmPtr()->setDbTag(data(10));
	getDomainDecompSolutionAlgorithmPtr()->recvSelf(cp);
      }
    else
      {
	std::cerr << "DomainDecompositionAnalysis::recvSelf";
	std::cerr << " - failed to get the Domain Decomp Algo\n";
	return -1;
      }            

    brokeIncrementalIntegrator(cp,data);
    if(getIncrementalIntegratorPtr())
      {
	getIncrementalIntegratorPtr()->setDbTag(data(11));
	getIncrementalIntegratorPtr()->recvSelf(cp);
      }
    else
      {
	std::cerr << "DomainDecompositionAnalysis::recvSelf";
	std::cerr << " - failed to get the XC::IncrementalIntegrator\n";
	return -1;
      }        	

    brokeDDLinearSOE(cp,ID(data(5)));
    theSolver = cp.getNewDomainSolver();

    if(getLinearSOEPtr() == 0 || theSolver == 0)
      {
	std::cerr << "DomainDecompositionAnalysis::recvSelf";
	std::cerr << " - failed to get the LinearSOE and the DomainSolver \n";
	return -1;
      }
    else
      {
	getLinearSOEPtr()->setDbTag(data(12));
	theSolver->setDbTag(data(13));
	getLinearSOEPtr()->recvSelf(cp);
	theSolver->recvSelf(cp);
      }

    // set the links in all the objects
    set_all_links();
    return 0;
  }

//! @brief Asigna el algoritmo de solución a emplear en el análisis.
int XC::DomainDecompositionAnalysis::setAlgorithm(EquiSolnAlgo &theAlgorithm)
  {
    Analysis::setAlgorithm(theAlgorithm);
    std::cerr << "DomainDecompositionAnalysis::setAlgorithm() - not implemented\n";
    return -1;
  }

//! @brief Asigna el integrador a emplear en el análisis.
int XC::DomainDecompositionAnalysis::setIntegrator(IncrementalIntegrator &theIntegrator) 
  {
    Analysis::setIntegrator(theIntegrator);
    std::cerr << "DomainDecompositionAnalysis::setIntegrator() - not implemented\n";
    return -1;
  }


//! @brief Asigna el sistema de ecuaciones a emplear en el análisis.
int XC::DomainDecompositionAnalysis::setLinearSOE(LinearSOE &theSOE)
  {
    Analysis::setLinearSOE(theSOE);
    std::cerr << "DomainDecompositionAnalysis::setLinearSOE() - not implemented\n";
    return -1;
  }

//! @brief Asigna el test de convergencia a emplear en el análisis.
int XC::DomainDecompositionAnalysis::setConvergenceTest(ConvergenceTest &theTest)
  { return metodo_solu->setConvergenceTest(theTest); }

