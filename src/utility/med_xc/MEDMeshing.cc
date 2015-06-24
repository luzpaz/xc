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
//MEDMeshing

#include "MEDMeshing.h"
#include "MEDDblFieldInfo.h"
#include "MEDIntFieldInfo.h"
#include "MEDGaussModel.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/nucleo/InterpreteRPN.h"
#include "xc_utils/src/nucleo/MatrizAny.h"
#include "xc_utils/src/base/utils_any.h"
#include "modelador/MapSet.h"
#include "post_process/MapFields.h"
#include "modelador/set_mgmt/Set.h"
#include "domain/domain/Domain.h"
#include "domain/mesh/element/Element.h"
#include "domain/mesh/element/gauss_models/GaussModel.h"
#include "domain/mesh/node/Node.h"
#include "ProblemaEF.h"

const std::string XC::MEDMeshing::str_grupo_nodos= "_nodos";
const std::string XC::MEDMeshing::str_grupo_elementos= "_elementos";

// @brief Returns med mesh to export.
XC::MEDMeshing::MEDMeshing(const ProblemaEF &prb) 
  : sets(prb.getModelador().get_sets()), fields(prb.getFields()), vertices(prb.getDomain()->getMesh()), cells(prb.getDomain()->getMesh(),vertices.getMapIndices())
  {}

//! Constructor.
XC::MEDMeshing::MEDMeshing(const Mesh &mesh,const MapSet &s, const MapFields &f)
  : sets(s), fields(f), vertices(mesh), cells(mesh,vertices.getMapIndices()) {} 

//! Devuelve una referencia a la malla de MEDMEM.
MEDMEM::MESHING &XC::MEDMeshing::getMallaMED(void) const
  { return malla; }

//! Borra la malla.
void XC::MEDMeshing::clear(void)
  {
    vertices.clear();
    cells.clear();
    med_groups.clear();
    for(std::deque<MEDFieldInfo *>::iterator i= med_fields.begin();i!=med_fields.end();i++)
      delete *i;
    med_fields.clear();
    //malla= MEDMEM::MESHING(); //Da problemas 2012/10/03
  }
//! Constructor.
XC::MEDMeshing::~MEDMeshing(void)
  { clear(); }

const XC::MEDMapIndices &XC::MEDMeshing::getMapIndicesVertices(void) const
  { return vertices.getMapIndices(); }

const XC::MEDMapIndices &XC::MEDMeshing::getMapIndicesCeldas(void) const
  { return cells.getMapIndices(); }

//! @brief Define los grupos de MEDMEM correspondientes a los conjuntos
//! de XC.
void XC::MEDMeshing::defineMEDGroups(void)
  {
    for(MapSet::const_iterator i= sets.begin();i!=sets.end();i++)
      {
        const Set *set= dynamic_cast<const Set *>(i->second);
        if(set)
          {
            const std::string nmb= set->GetNombre();
            const size_t numNodos= set->NumNodos();
            const size_t numElementos= set->NumElementos();
            if((numElementos==0) && (numNodos==0))
              continue;
            else if((numElementos==0) || (numNodos==0))
              med_groups.push_back(MEDGroupInfo(this,*set));
            else
              {
                Set set_nodos(*set);
                set_nodos.clearElementos();
                set_nodos.Nombre()= nmb+str_grupo_nodos;
                med_groups.push_back(MEDGroupInfo(this,set_nodos));
                Set set_elementos(*set);
                set_elementos.clearNodos();
                set_elementos.Nombre()= nmb+str_grupo_elementos;
                med_groups.push_back(MEDGroupInfo(this,set_elementos));
              }
          }
      }
  }

XC::MEDGroupInfo *XC::MEDMeshing::getGroupInfo(const std::string &nmb) const
  {
    MEDGroupInfo *retval= nullptr;
    std::deque<MEDGroupInfo>::iterator i= med_groups.begin();
    for(;i!=med_groups.end();i++)
      if(i->getNombre()==nmb)
        break;
    if(i!=med_groups.end())
      retval= &(*i);
    return retval;
  }

XC::MEDGroupInfo *XC::MEDMeshing::getGroupInfo(const Set &set,const FieldInfo &field) const
  {
    MEDGroupInfo *retval= nullptr;
    std::string nmb_grupo= "";
    if(field.isDefinedOnNodes())
      nmb_grupo= set.GetNombre()+str_grupo_nodos;
    else if(field.isDefinedOnElements())
      nmb_grupo= set.GetNombre()+str_grupo_elementos;
    retval= getGroupInfo(nmb_grupo);
    if(!retval)
      std::cerr << "MEDMeshing::getGroupInfo; no se encotró el grupo: "
                << nmb_grupo << std::endl;
    return retval;  
  }

//! @brief Define los modelos de integración de Gauss de los elementos.
void XC::MEDMeshing::defineMEDGaussModels(const Set &set,MEDFieldInfo &med_fi) const
  {
    med_fi.defineGaussModels(set);
  }

//! @brief Define un campo sobre un conjunto.
void XC::MEDMeshing::defineMEDDblField(const Set &set,const FieldInfo &fi,MEDGroupInfo *grupo) const
  {
    MEDDblFieldInfo *med_fi= new MEDDblFieldInfo(fi,grupo);
    med_fields.push_back(med_fi);
    med_fi->to_med();
    if(fi.isDefinedOnNodes())
      { med_fi->populateOnNodes(set,fi); }
    else if(fi.isDefinedOnElements())
      {
        if(fi.isDefinedOnGaussPoints())
          {
            defineMEDGaussModels(set,*med_fi);
            med_fi->populateOnGaussPoints(set,fi);
          }
        else
          med_fi->populateOnElements(set,fi);
      }
  }

//! @brief Define un campo sobre un conjunto.
void XC::MEDMeshing::defineMEDIntField(const Set &set,const FieldInfo &fi,MEDGroupInfo *grupo) const
  {
    MEDIntFieldInfo *med_fi= new MEDIntFieldInfo(fi,grupo);
    med_fields.push_back(med_fi);
    med_fi->to_med();
    if(fi.isDefinedOnNodes())
      { med_fi->populateOnNodes(set,fi); }
    else if(fi.isDefinedOnElements())
      {
        if(fi.isDefinedOnGaussPoints())
          {
            defineMEDGaussModels(set,*med_fi);
            med_fi->populateOnGaussPoints(set,fi);
          }
        else
          med_fi->populateOnElements(set,fi);
      }
  }

//! @brief Define los campos de MEDMEM correspondientes a los de XC.
void XC::MEDMeshing::defineMEDFields(void) const
  {
    for(MapFields::const_iterator fieldIter= fields.begin();fieldIter!=fields.end();fieldIter++)
      {
        const FieldInfo &fi= *fieldIter;
        const Set *set= dynamic_cast<const Set *>(sets.busca_set(fi.getSetName()));
        if(set)
          {
            MEDGroupInfo *grupo= getGroupInfo(*set,fi);
            if(grupo)
              {
                const std::string tipo_componentes= fi.getComponentsType();
                if(tipo_componentes=="double")
                  { defineMEDDblField(*set,fi,grupo); }
                else if(tipo_componentes=="int")
                  { defineMEDIntField(*set,fi,grupo); }
                else
                  std::cerr << "MEDMeshing::defineMEDFields; el tipo: " << tipo_componentes
                            << " es desconocido." << std::endl;
	      }
          }
        else
	  std::cerr << "MEDMeshing::defineMEDFields; no se encontró el conjunto: " 
                    << fi.getSetName() << " en el que se define el campo: "
                    << fi.GetNombre() << std::endl;
      }
  }

//! @brief Vuelca la definición de vértices y celdas en la
//! malla MED
void XC::MEDMeshing::to_med(void)
  {
    vertices.to_med(malla);
    cells.to_med(malla);
    defineMEDGroups();
    for(std::deque<MEDGroupInfo>::const_iterator i=med_groups.begin();i!=med_groups.end();i++)
      i->to_med();
    defineMEDFields();
  }

void XC::MEDMeshing::write(const std::string &fileName)
  {
    to_med();
    const int id= malla.addDriver(MEDMEM::MED_DRIVER,fileName,malla.getName());
    malla.write(id);
    for(std::deque<MEDFieldInfo *>::iterator i= med_fields.begin();i!=med_fields.end();i++)
      (*i)->write(fileName);
  }

//! @brief Lectura del objeto desde archivo.
bool XC::MEDMeshing::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(MEDMeshing) Procesando comando: " << cmd << std::endl;

    if(cmd == "setName") //Asigna nombre a la malla.
      {
        const std::string nmb= interpretaString(status.GetBloque());
        malla.setName(nmb);
        return true;
      }
    else if(cmd == "vertices")
      {
        vertices.LeeCmd(status);
        return true;
      }
    else if(cmd == "cells")
      {
        cells.LeeCmd(status);
        return true;
      }
    else if(cmd == "clear")
      {
        status.GetString(); //Ignoramos argumentos.
        clear();
        return true;
      }
    else if(cmd == "write")
      {
        const std::string fileName= interpretaString(status.GetString());
        write(fileName);
        return true;
      }
    else    
      return MEDObject::procesa_comando(status);
  }


//! Devuelve la propiedad del objeto cuyo código se pasa 
//! como parámetro. 
any_const_ptr XC::MEDMeshing::GetProp(const std::string &cod) const 
  { return MEDObject::GetProp(cod); }