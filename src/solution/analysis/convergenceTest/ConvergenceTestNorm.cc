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
//ConvergenceTestNorm.cc

#include <solution/analysis/convergenceTest/ConvergenceTestNorm.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"


XC::ConvergenceTestNorm::ConvergenceTestNorm(EntCmd *owr,int classTag)	    	
  : ConvergenceTestTol(owr,classTag), norm0(0.0) {}


XC::ConvergenceTestNorm::ConvergenceTestNorm(EntCmd *owr,int classTag,double theTol, int maxIter, int printIt, int normType, int sz)
  : ConvergenceTestTol(owr,classTag,theTol,maxIter,printIt,normType,sz), norm0(0.0) {}

//! @brief Lee un objeto ConvergenceTestNorm desde archivo
bool XC::ConvergenceTestNorm::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(ConvergenceTestNorm) Procesando comando: " << cmd << std::endl;
    if(cmd=="norm0")
      {
        norm0= interpretaDouble(status.GetString());
        return true;
      }
    else
      return ConvergenceTestTol::procesa_comando(status);
  }

int XC::ConvergenceTestNorm::start(void)
  {
    ConvergenceTest::start();
    norm0 = 0.0;
    return 0;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
//!
//! Soporta los códigos:
any_const_ptr XC::ConvergenceTestNorm::GetProp(const std::string &cod) const
  {
    if(cod=="norm0")
      return any_const_ptr(norm0);
    else
      return ConvergenceTestTol::GetProp(cod);
  }
