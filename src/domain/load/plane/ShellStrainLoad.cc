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


#include "ShellStrainLoad.h"
#include "utility/matrix/Vector.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "utility/matrix/ID.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"


//! @brief Constructor.
XC::ShellStrainLoad::ShellStrainLoad(int tag, const ID &theElementTags)
  :BidimStrainLoad(tag,4,Vector(8), theElementTags) {}

//! @brief Constructor.
XC::ShellStrainLoad::ShellStrainLoad(int tag)
  :BidimStrainLoad(tag,4,Vector(8)) {}

//! @brief Lee un objeto ShellStrainLoad desde archivo
bool XC::ShellStrainLoad::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(ShellStrainLoad) Procesando comando: " << cmd << std::endl;

    return BidimStrainLoad::procesa_comando(status);
  }

//! Devuelve la propiedad del objeto cuyo código se pasa
//! como parámetro.
any_const_ptr XC::ShellStrainLoad::GetProp(const std::string &cod) const
  {
    return BidimStrainLoad::GetProp(cod);
  }