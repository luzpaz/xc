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
//ProfileSPDLinDirectBase.h

#ifndef ProfileSPDLinDirectBase_h
#define ProfileSPDLinDirectBase_h

#include <solution/system_of_eqn/linearSOE/profileSPD/ProfileSPDLinSolver.h>
#include "utility/matrix/Vector.h"

namespace XC {

//! @ingroup Solver
//
//! @brief Objetos que se encargan de la solución
//! de sistemas de ecuaciones con matriz en perfil.
class ProfileSPDLinDirectBase: public ProfileSPDLinSolver
  {
  protected:
    double minDiagTol;
    int size;
    ID RowTop;
    std::vector<double *> topRowPtr;
    Vector invD;
  public:
    ProfileSPDLinDirectBase(int classTag, double tol);    

  };
} // fin namespace XC


#endif
