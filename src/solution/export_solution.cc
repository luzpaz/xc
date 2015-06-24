//----------------------------------------------------------------------------
//  programa XC; cálculo mediante el método de los elementos finitos orientado
//  a la solución de problemas estructurales.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  Este software es libre: usted puede redistribuirlo y/o modificarlo 
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
//utils_python_interface.cxx

#include "python_interface.h"
#include "ProblemaEF.h"

void export_solution(void)
  {
    using namespace boost::python;
    docstring_options doc_options;

#include "analysis/python_interface.tcc"
#include "system_of_eqn/python_interface.tcc"

class_<XC::ConvergenceTest, bases<XC::MovableObject,EntCmd>, boost::noncopyable >("ConvergenceTest", no_init);

class_<XC::SoluMethod, bases<EntCmd>, boost::noncopyable >("SoluMethod", no_init)
    .def("newSolutionAlgorithm", &XC::SoluMethod::newSolutionAlgorithm,return_internal_reference<>(),"Define el algoritmo de solución a emplear.")
    .def("newIntegrator", &XC::SoluMethod::newIntegrator,return_internal_reference<>(),"Define el integrador a emplear.")
    .def("newSystemOfEqn", &XC::SoluMethod::newSystemOfEqn,return_internal_reference<>(),"Define el sistema de ecuaciones a emplear.")
    .def("newConvergenceTest", &XC::SoluMethod::newConvergenceTest,return_internal_reference<>(),"Define el criterio de convergencia a emplear.")
    ;

class_<XC::MapSoluMethod, bases<EntCmd>, boost::noncopyable >("MapSoluMethod", no_init)
    .add_property("existeSoluMethod", &XC::MapSoluMethod::existeSoluMethod)
    .def("newSoluMethod", &XC::MapSoluMethod::newSoluMethod,return_internal_reference<>(),"Crea un nuevo procedimiento de solución.")
    ;

XC::ModelWrapper *(XC::ProcSoluControl::*getModelWrapperPtr)(const std::string &)= &XC::ProcSoluControl::getModelWrapper;
class_<XC::ProcSoluControl, bases<EntCmd>, boost::noncopyable >("SoluControl", no_init)
    .add_property("getModelWrapper", make_function( getModelWrapperPtr, return_internal_reference<>() ))
    .add_property("getModelWrapperContainer",  make_function(&XC::ProcSoluControl::getModelWrapperContainer, return_internal_reference<>()) )
    .add_property("getSoluMethodContainer",  make_function(&XC::ProcSoluControl::getSoluMethodContainer, return_internal_reference<>()) )
    ;

XC::ProcSoluControl &(XC::ProcSolu::*getSoluControlRef)(void)= &XC::ProcSolu::getSoluControl;
class_<XC::ProcSolu, bases<EntCmd>, boost::noncopyable >("ProcSolu", no_init)
    .add_property("getSoluControl", make_function( getSoluControlRef, return_internal_reference<>() ))
    .add_property("getAnalysis", make_function( &XC::ProcSolu::getAnalysis, return_internal_reference<>() ))
    .def("newAnalysis", &XC::ProcSolu::newAnalysis,return_internal_reference<>(),"Crea un nuevo análisis.")
    ;

  }
