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
//EqualDOF.h

// Este objeto construye un objeto MP_Constraint que impone
// igualdad de movimientos en los grados de libertad que
// se le indiquen.

#ifndef EqualDOF_h
#define EqualDOF_h

#include "MP_Constraint.h"

namespace XC {
class Domain;
class ID;

//! @ingroup CContMP
//
//! @brief Impone la igualdad de desplazamientos a varios nodos.
class EqualDOF: public MP_Constraint
  {
    void setup_matrix(void);
  public:
    EqualDOF(int tag);
    EqualDOF(int tag,const int &,const int &,const ID &);

    void setDofs(const ID &);
    void setup(Domain *theDomain);
  };
} // fin namespace XC

#endif