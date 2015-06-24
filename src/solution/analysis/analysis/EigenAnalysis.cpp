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
// $Date: 2005/08/31 17:39:34 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/analysis/EigenAnalysis.cpp,v $
                                                                        
                                                                        
// File: ~/analysis/analysis/eigenAnalysis/EigenAnalysis.C
//
// Written: Jun Peng
// Created: Wed Jan 27, 1999
// Revision: A
//
// Description: This file contains the class definition of XC::EigenAnalysis.
// EigenAnalysis is a subclass of XC::Analysis, it is used to perform the 
// eigen value analysis on the FE_Model.
//
// This class is inheritanted from the base class of XC::Analysis
// which was created by fmk (Frank).


#include <solution/analysis/analysis/EigenAnalysis.h>
#include <solution/analysis/algorithm/eigenAlgo/EigenAlgorithm.h>
#include <solution/analysis/model/AnalysisModel.h>
#include <solution/system_of_eqn/eigenSOE/EigenSOE.h>
#include <solution/analysis/numberer/DOF_Numberer.h>
#include <solution/analysis/handler/ConstraintHandler.h>
#include <solution/analysis/integrator/EigenIntegrator.h>
#include <domain/domain/Domain.h>
#include "solution/analysis/ModelWrapper.h"
#include "solution/SoluMethod.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include "xc_utils/src/nucleo/InterpreteRPN.h"
#include "utility/matrix/Matrix.h"

//! @brief Constructor.
XC::EigenAnalysis::EigenAnalysis(SoluMethod *metodo)
  :Analysis(metodo), domainStamp(0) {}

//! @brief Constructor virtual.
XC::Analysis *XC::EigenAnalysis::getCopy(void) const
  { return new EigenAnalysis(*this); }

//! @brief Lee un objeto XC::EigenAnalysis desde archivo
bool XC::EigenAnalysis::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(EigenAnalysis) Procesando comando: " << cmd << std::endl;
    if(cmd == "analyze") //Lanza el análisis del problema.
      {
        int numModes= interpretaInt(status.GetString());
        analysis_result= analyze(numModes);
        return true;
      }
    else
      return Analysis::procesa_comando(status);
  }

//! @brief Borra todos los miembros del objeto (Manejador coacciones, modelo de análisis,...).
void XC::EigenAnalysis::clearAll(void)
  {
    // invoke the destructor on all the objects in the aggregation
    Analysis::clearAll();
    delete getEigenSolutionAlgorithmPtr();
  }

//! @brief Ejecuta el análisis.
int XC::EigenAnalysis::analyze(int numModes)
  {
    int result= 0;
    assert(metodo_solu);
    EntCmd *old= metodo_solu->Owner();
    metodo_solu->set_owner(this);

    // check for change in Domain since last step. As a change can
    // occur in a commit() in a domaindecomp with load balancing
    // this must now be inside the loop
    int stamp = getDomainPtr()->hasDomainChanged();
    if(stamp != domainStamp)
      {
	domainStamp = stamp;
	result = domainChanged();
	if(result < 0)
          {
	    std::cerr << "EigenAnalysis::analyze() - domainChanged failed\n";
	    return -1;
	  }
      }

    result = getEigenIntegratorPtr()->newStep();
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::analyze() - integrator failed\n";
	return -2;
      }
    result = getEigenSolutionAlgorithmPtr()->solveCurrentStep(numModes);
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::analyze() - algorithm failed\n";
	return -3;
      }
    metodo_solu->set_owner(old);
    return 0;
  }

//! @brief Hace los cambios necesarios como consecuencia de un cambio en el dominio.
int XC::EigenAnalysis::domainChanged(void)
  {
    getAnalysisModelPtr()->clearAll();    
    getConstraintHandlerPtr()->clearAll();      
    int result= getConstraintHandlerPtr()->handle();
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::domainChanged() - ";
        std::cerr << "ConstraintHandler::handle() failed." << std::endl;
        return -1;
      }

    //Asignamos números de ecuación.
    result= getDOF_NumbererPtr()->numberDOF();
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::domainChanged() - ";
        std::cerr << "fallo al numerar las ecuaciones." << std::endl;
        return -2;
      }
    result= getConstraintHandlerPtr()->doneNumberingDOF();
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::domainChanged() - ";
        std::cerr << "fallo en doneNumberingDOF()." << std::endl;
        return -3;
      }

    Graph &theGraph = getAnalysisModelPtr()->getDOFGraph();
    result= getEigenSOEPtr()->setSize(theGraph);
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::domainChanged() - ";
        std::cerr << "fallo al asingnar el tamaño del sistema de ecuaciones." << std::endl;
        return -4;
      }

    result= getEigenIntegratorPtr()->domainChanged();
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::domainChanged() - ";
        std::cerr << "fallo en Integrator::domainChanged()." << std::endl;
        return -5;
      }
    result= getEigenSolutionAlgorithmPtr()->domainChanged();
    if(result < 0)
      {
        std::cerr << "EigenAnalysis::domainChanged() - ";
        std::cerr << "fallo en Algorithm::domainChanged()." << std::endl;
        return -5;
      }

    return 0;
  }

//! @brief Asigna el algoritmo a emplear en el análisis.
int XC::EigenAnalysis::setAlgorithm(EigenAlgorithm &theAlgo)
  {
    Analysis::setAlgorithm(theAlgo);
    std::cerr << "EigenAnalysis::setAlgorithm() - does nothing yet\n";
    return 0;
  }

//! @brief Asigna el integrador a emplear en el análisis.
int XC::EigenAnalysis::setIntegrator(EigenIntegrator &theIntegrator)
  {
    Analysis::setIntegrator(theIntegrator);
    std::cerr << "EigenAnalysis::setIntegrator() - does nothing yet\n";    
    return 0;
  }

//! @brief Asigna el sistema de autovalores a emplear en el análisis.
int XC::EigenAnalysis::setEigenSOE(EigenSOE &theSOE)
  {
    Analysis::setEigenSOE(theSOE);
    return 0;
  }

//! @brief Devuelve el autovector que corresponde al modo que se pasa como parámetro.
const XC::Vector &XC::EigenAnalysis::getEigenvector(int mode) const
  {
    static Vector retval(1);
    retval.Zero();
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEigenvector(mode);
    return retval;
  }

//! @brief Devuelve el autovector que corresponde al modo que se pasa como parámetro
//! normalizado de modo que la componente máxima valga 1 (norma_infinito).
XC::Vector XC::EigenAnalysis::getNormalizedEigenvector(int mode) const
  {
    static Vector retval(1);
    retval.Zero();
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getNormalizedEigenvector(mode);
    return retval;
  }

//! @brief Devuelve una matriz con los autovectores calculados colocados
//! por columnas.
XC::Matrix XC::EigenAnalysis::getEigenvectors(void) const
  {
    Matrix retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEigenvectors();
    return retval;
  }

//! @brief Devuelve una matriz con los autovectores normalizados colocados
//! por columnas (norma_infinito).
XC::Matrix XC::EigenAnalysis::getNormalizedEigenvectors(void) const
  {
    Matrix retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getNormalizedEigenvectors();
    return retval;
  }

//! @brief Devuelve el autovalor que corresponde al modo que se pasa como parámetro.
const double &XC::EigenAnalysis::getEigenvalue(int mode) const
  {
    static double retval= 0.0;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEigenvalue(mode);
    return retval;
  }

//! @brief Devuelve la pulsación correspondiente al modo
//! que se pasa como parámetro.
double XC::EigenAnalysis::getPulsacion(int mode) const
  { return sqrt(getEigenvalue(mode)); }

//! @brief Devuelve el período correspondiente al modo
//! que se pasa como parámetro.
double XC::EigenAnalysis::getPeriodo(int mode) const
  { return 2.0*M_PI/getPulsacion(mode); }

//! @brief Devuelve la frecuencia correspondiente al modo
//! que se pasa como parámetro.
double XC::EigenAnalysis::getFrecuencia(int mode) const
  { return 1./getPeriodo(mode); }

//! @brief Devuelve un vector con los autovalores calculados.
XC::Vector XC::EigenAnalysis::getEigenvalues(void) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEigenvalues();
    return retval;
  }

//! @brief Devuelve un vector con las pulsaciones calculadas.
XC::Vector XC::EigenAnalysis::getPulsaciones(void) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getPulsaciones();
    return retval;
  }

//! @brief Devuelve un vector con las periodos calculados.
XC::Vector XC::EigenAnalysis::getPeriodos(void) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getPeriodos();
    return retval;
  }

//! @brief Devuelve un vector con las frecuencias calculadas.
XC::Vector XC::EigenAnalysis::getFrecuencias(void) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getFrecuencias();
    return retval;
  }

//! @brief Devuelve el número de autovalores que se han calculado.
int XC::EigenAnalysis::getNumModes(void) const
  {
    int retval= 0;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getNumModes();
    return retval;
  }

//! @brief Devuelve el factor de participación modal
//! correspondiente al modo i.
double XC::EigenAnalysis::getModalParticipationFactor(int mode) const
  {
    double retval= 0.0;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getModalParticipationFactor(mode);
    else
      std::cerr << "EigenAnalysis::getModalParticipationFactor( " << mode
                << "); el puntero al sistema de ecuaciones es nulo." << std::endl;
    return retval;
  }

//! @brief Devuelve los factores de participación modal.
XC::Vector XC::EigenAnalysis::getModalParticipationFactors(void) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getModalParticipationFactors();
    else
      std::cerr << "EigenAnalysis::getModalParticipationFactors; "
                << "el puntero al sistema de ecuaciones es nulo." << std::endl;
    return retval;
  }
//! @brief Devuelve el factor de distribución correspondiente al modo
//! que se pasa como parámetro.
XC::Vector XC::EigenAnalysis::getDistributionFactor(int mode) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getDistributionFactor(mode);
    return retval;
  }

//! @brief Devuelve una matriz con los factores de distribución
//! calculados colocados por columnas.
XC::Matrix XC::EigenAnalysis::getDistributionFactors(void) const
  {
    Matrix retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getDistributionFactors();
    return retval;
  }

//! @brief Devuelve la masa modal efectiva 
//! correspondiente al modo i.
double XC::EigenAnalysis::getEffectiveModalMass(int mode) const
  {
    double retval= 0.0;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEffectiveModalMass(mode);
    return retval;
  }

//! @brief Devuelve las masas modales efectivas.
XC::Vector XC::EigenAnalysis::getEffectiveModalMasses(void) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEffectiveModalMasses();
    return retval;
  }

//! @brief Devuelve la masa total del modelo.
double XC::EigenAnalysis::getTotalMass(void) const
  {
    double retval= 0.0;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getTotalMass();
    return retval;
  }

//! @brief Devuelve la fuerza estática equivalente para el modo
//! que se pasa como parámetro.
XC::Vector XC::EigenAnalysis::getEquivalentStaticLoad(int mode,const double &accel_mode) const
  {
    Vector retval;
    EigenSOE *ptr_soe= getEigenSOEPtr();
    if(ptr_soe)
      retval= ptr_soe->getEquivalentStaticLoad(mode,accel_mode);
    return retval;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::EigenAnalysis::GetProp(const std::string &cod) const
  {
    if(verborrea>4)
      std::clog << "EigenAnalysis::GetProp ("
                << nombre_clase() 
                << "::GetProp) Buscando propiedad: "
                << cod << std::endl;


    if(cod=="eigen_value")
      {
        const int modo= popInt(cod);
        tmp_gp_dbl= getEigenvalue(modo);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getPulsacion")
      {
        const int modo= popInt(cod);
        tmp_gp_dbl= getPulsacion(modo);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getPeriodo")
      {
        const int modo= popInt(cod);
        tmp_gp_dbl= getPeriodo(modo);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getFrecuencia")
      {
        const int modo= popInt(cod);
        tmp_gp_dbl= getFrecuencia(modo);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if((cod=="getEigenvector"))
      {
        const int modo= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double(getEigenvector(modo));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getNormalizedEigenvector")
      {
        const int modo= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double(getNormalizedEigenvector(modo));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getEigenvectors")
      {
        //tmp_gp_mdbl= getEigenvectors();
 	std::cerr << "DEPRECATED; use Python" << std::endl;
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getNumModes")
      {
        tmp_gp_int= getNumModes();
        return any_const_ptr(tmp_gp_int);
      }
    else if(cod=="getEigenvalues")
      {
        tmp_gp_mdbl= vector_to_m_double(getEigenvalues());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getPulsaciones")
      {
        tmp_gp_mdbl= vector_to_m_double(getPulsaciones());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getPeriodos")
      {
        tmp_gp_mdbl= vector_to_m_double(getPeriodos());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getFrecuencias")
      {
        tmp_gp_mdbl= vector_to_m_double(getFrecuencias());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getEigenvectors")
      {
        //tmp_gp_mdbl= getEigenvectors();
 	std::cerr << "DEPRECATED; use Python" << std::endl;
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getNormalizedEigenvectors")
      {
        //tmp_gp_mdbl= getNormalizedEigenvectors();
 	std::cerr << "DEPRECATED; use Python" << std::endl;
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getModalParticipationFactor")
      {
        const int modo= popInt(cod);
        tmp_gp_dbl= getModalParticipationFactor(modo);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getModalParticipationFactors")
      {
        tmp_gp_mdbl= vector_to_m_double(getModalParticipationFactors());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getEffectiveModalMass")
      {
        const int modo= popInt(cod);
        tmp_gp_dbl= getEffectiveModalMass(modo);
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getEffectiveModalMasses")
      {
        tmp_gp_mdbl= vector_to_m_double(getEffectiveModalMasses());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getTotalMass")
      {
        tmp_gp_dbl= getTotalMass();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getDistributionFactor")
      {
        const int modo= popInt(cod);
        tmp_gp_mdbl= vector_to_m_double(getDistributionFactor(modo));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getDistributionFactors")
      {
        //tmp_gp_mdbl= getDistributionFactors();
 	std::cerr << "DEPRECATED; use Python" << std::endl;
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getEquivalentStaticLoad")
      {
        int modo= 0;
        double accel= 0.0;
        if(InterpreteRPN::Pila().size()>1)
          {
            accel= convert_to_double(InterpreteRPN::Pila().Pop());
            modo= convert_to_int(InterpreteRPN::Pila().Pop());
          }
        else
          err_num_argumentos(std::cerr,2,"GetProp",cod);
        tmp_gp_mdbl= vector_to_m_double(getEquivalentStaticLoad(modo,accel));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else
      return Analysis::GetProp(cod);
  }