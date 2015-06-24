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
//MovableID.cc

#include "MovableID.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "utility/actor/channel/Channel.h"
#include "utility/actor/objectBroker/FEM_ObjectBroker.h"
#include "utility/matrix/ID.h"
#include "CommMetaData.h"

//! @brief Constructor.
XC::MovableID::MovableID(const size_t &sz)
  :ID(sz),MovableObject(0) {}

//! @brief Constructor.
XC::MovableID::MovableID(const ID &v)
  :ID(v),MovableObject(0) {}

//! @brief Lee un objeto XC::Node desde archivo
//!
//! Soporta los comandos:
//! -coo: Lee las coordenadas del nodo.
bool XC::MovableID::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(MovableID) Procesando comando: " << cmd << std::endl;

      return ID::procesa_comando(status);
  }

//! @brief Asigna el ID.
void XC::MovableID::setID(const ID &v)
  { ID::operator=(v); }

//! @brief Devuelve un vector para almacenar los dbTags
//! de los miembros de la clase.
XC::DbTagData &XC::MovableID::getDbTagData(void) const
  {
    static DbTagData retval(2);
    return retval;
  }

//! @brief Envia el ID con los parámetros (canal,...) que se pasan como argumento.
int XC::MovableID::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dbTag= getDbTag();
    inicComm(2);
    const int dataTag= cp.getDbTag();
    const int sz= Size();
    setDbTagDataPos(0,sz);
    setDbTagDataPos(1,dataTag);

    int res= cp.sendIdData(getDbTagData(),dbTag);
    if(res < 0)
      std::cerr << "MovableID::sendSelf() - failed to send ID data\n";

    if(sz>0)
      {
        res+= cp.sendID(*this,dataTag);
        if(res<0)
          std::cerr << "MovableID::sendSelf() - failed to send Disp data\n";
      }
    return res;
  }

//! @brief Recibe el ID con los parámetros (canal,...) que se pasan como argumento.
int XC::MovableID::recvSelf(const CommParameters &cp)
  {
    inicComm(2);
    const int dbTag= getDbTag();
    int res = cp.receiveIdData(getDbTagData(),dbTag);
    if(res < 0)
      std::cerr << "MovableID::recvSelf() - failed to receive ID data\n";
    else
      {
        const int sz= getDbTagDataPos(0);
        resize(sz);
        if(sz>0)
          {
            const int dataTag= getDbTagDataPos(1);
            if(sz>0)
              res+= cp.receiveID(*this,dataTag);
            if(res<0)
              std::cerr << "MovableID::recvSelf - failed to receive data\n";
          }
      }
    return 0;
  }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
//!
//! Soporta los códigos:
//! nnod: Devuelve el número de nodos del dominio.
any_const_ptr XC::MovableID::GetProp(const std::string &cod) const
  {
    return ID::GetProp(cod);
  }