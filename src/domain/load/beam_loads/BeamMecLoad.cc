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
//BeamMecLoad.cpp

#include <domain/load/beam_loads/BeamMecLoad.h>
#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "utility/matrix/ID.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "domain/mesh/element/Element1D.h"
#include "domain/mesh/element/coordTransformation/CrdTransf.h"
#include "utility/actor/actor/MovableID.h"
#include "utility/actor/actor/MovableVector.h"

XC::BeamMecLoad::BeamMecLoad(int tag,int classTag,const double &wt,const double &wa,const XC::ID &theElementTags)
  :BeamLoad(tag, classTag, theElementTags), Trans(wt), Axial(wa) {}

XC::BeamMecLoad::BeamMecLoad(int tag,int classTag)
  :BeamLoad(tag, classTag), Trans(0.0), Axial(0.0) {}

//! @brief Lee un objeto BeamMecLoad desde archivo
bool XC::BeamMecLoad::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(BeamMecLoad) Procesando comando: " << cmd << std::endl;
    if(cmd == "trans")
      {
        Trans= interpretaDouble(status.GetString());
        return true;
      }
    else if(cmd == "axial")
      {
        Axial= interpretaDouble(status.GetString());
        return true;
      }
    else
      return BeamLoad::procesa_comando(status);
  }

const XC::Matrix &XC::BeamMecLoad::getAppliedSectionForces(const double &L,const XC::Matrix &xi,const double &loadFactor)
  {
    static Matrix retval;
    std::cerr << "getAppliedSectionForces no definida." << std::endl;
    return retval;
  }

//! @brief Añade la carga al vector de cargas consistentes (ver página 108 libro Eugenio Oñate).
//! @param L Longitud del elemento.
//! @param loadFactor Ponderación de la carga.
//! @param p0 Vector de cargas del elemento.
void XC::BeamMecLoad::addReactionsInBasicSystem(const double &,const double &,FVector &)
  {
    std::cerr << "addReactionsInBasicSystem no definida." << std::endl;
  }

//! @brief ??
//! @param L Longitud del elemento.
//! @param loadFactor Ponderación de la carga.
//! @param q0 ??
void XC::BeamMecLoad::addFixedEndForcesInBasicSystem(const double &L,const double &loadFactor,FVector &)
  {
    std::cerr << "addFixedEndForcesInBasicSystem no definida." << std::endl;
  }

void XC::BeamMecLoad::addElasticDeformations(const double &L,const ConstantesSecc2d &ctes_scc,const double &lpI,const double &lpJ,const double &loadFactor,FVector &v0)
  {
    std::cerr << "addElasticDeformations no definida para secciones 2d." << std::endl;
  }

void XC::BeamMecLoad::addElasticDeformations(const double &L,const ConstantesSecc3d &ctes_scc,const double &lpI,const double &lpJ,const double &loadFactor,FVector &v0)
  {
    std::cerr << "addElasticDeformations no definida para secciones 3d." << std::endl;
  }

//! @brief Devuelve la dimension del vector fuerza.
size_t XC::BeamMecLoad::getDimVectorFuerza(void) const
  { return 2; }

//! @brief Devuelve la dimension del vector momento.
size_t XC::BeamMecLoad::getDimVectorMomento(void) const
  { return 1; }

//! @brief Devuelve las componentes de los vectores fuerza.
const XC::Matrix &XC::BeamMecLoad::getLocalForces(void) const
  {
    static Matrix retval;
    const size_t sz= numElements();
    retval= Matrix(sz,2);
    for(size_t i=0; i<sz; i++)
      {
        retval(i,0)= Trans;
        retval(i,1)= Axial;
      }
    return retval;
  }

//! @brief Devuelve las componentes de los vectores momento.
const XC::Matrix &XC::BeamMecLoad::getLocalMoments(void) const
  {
    static Matrix retval;
    const size_t sz= numElements();
    retval= Matrix(sz,1);
    for(size_t i=0; i<sz; i++)
      retval(i,0)= 0.0;
    return retval;
  }

const XC::Matrix &XC::BeamMecLoad::getGlobalVectors(const Matrix &localVectors) const
  {
    static Matrix retval;
    retval= localVectors;
    const Domain *ptrDom= getDomain();
    if(ptrDom)
      {
        const size_t sz= localVectors.noRows();
        for(size_t i=0; i<sz; i++)
          {
            const size_t elemTag= getElementTags()(i);
            const Element1D *ptrElem= dynamic_cast<const Element1D *>(ptrDom->getElement(elemTag));
            if(ptrElem)
              {
                const CrdTransf *ptrTransf= ptrElem->getCoordTransf();
                if(ptrTransf)
                  retval= ptrTransf->getVectorGlobalCoordFromLocal(localVectors);
                else
		  std::cerr << "BeamMecLoad::getGlobalVectors; el elemento: "
                        << elemTag << " no tiene transformación de coordenadas." << std::endl;
              }
            else
	      std::cerr << "BeamMecLoad::getGlobalVectors; el elemento: "
                        << elemTag << " no es unidimensional." << std::endl;
          }
      }
    else
      std::cerr << "BeamMecLoad::getGlobalVectors; no existe apuntador al dominio." << std::endl;
    return retval;
  }

//! @brief Devuelve la fuerza expresada en coordenadas globales.
const XC::Matrix &XC::BeamMecLoad::getGlobalForces(void) const
  { return getGlobalVectors(getLocalForces()); }

//! @brief Devuelve el momento expresado en coordenadas globales.
const XC::Matrix &XC::BeamMecLoad::getGlobalMoments(void) const
  { return getGlobalVectors(getLocalMoments()); }

void XC::BeamMecLoad::Print(std::ostream &s, int flag) const
  {
    s << "BeamMecLoad - Reference load" << std::endl;
    s << "  Transverse: " << Trans << std::endl;
    s << "  Axial:      " << Axial << std::endl;
    BeamLoad::Print(s,flag);
  }

//! @brief Envía los datos a través del canal que se pasa como parámetro.
int XC::BeamMecLoad::sendData(CommParameters &cp)
  {
    int res= BeamLoad::sendData(cp);
    res+= cp.sendDoubles(Trans,Axial,getDbTagData(),CommMetaData(5));
    return res;
  }

//! @brief Recibe los datos a través del canal que se pasa como parámetro.
int XC::BeamMecLoad::recvData(const CommParameters &cp)
  {
    int res= BeamLoad::recvData(cp);
    res+= cp.receiveDoubles(Trans,Axial,getDbTagData(),CommMetaData(5));
    return res;
  }

//! Devuelve la propiedad del objeto cuyo código se pasa
//! como parámetro.
any_const_ptr XC::BeamMecLoad::GetProp(const std::string &cod) const
  {
    if(cod == "axial")
      return any_const_ptr(Axial);
    else if(cod == "trans")
      return any_const_ptr(Trans);
    else if(cod == "getLocalForces")
      {
        static m_double retval;
        retval= matrix_to_m_double(getLocalForces());
        return any_const_ptr(retval);
      }
    else if(cod == "getLocalMoments")
      {
        static m_double retval;
        retval= matrix_to_m_double(getLocalMoments());
        return any_const_ptr(retval);
      }
    else if(cod == "getGlobalForces")
      {
        static m_double retval;
        retval= matrix_to_m_double(getGlobalForces());
        return any_const_ptr(retval);
      }
    else
      return BeamLoad::GetProp(cod);
  }