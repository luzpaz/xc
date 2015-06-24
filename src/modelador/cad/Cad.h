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
//Cad.h

#ifndef CAD_H
#define CAD_H

#include "xc_utils/src/nucleo/EntCmd.h"
#include <map>
#include "boost/lexical_cast.hpp"
#include "modelador/cad/entidades/MapPuntos.h"
#include "modelador/cad/entidades/MapLineas.h"
#include "modelador/cad/entidades/MapSuperficies.h"
#include "modelador/cad/entidades/MapCuerpos.h"
#include "modelador/cad/entidades/MapUniformGrids.h"
#include "modelador/cad/trf/MapTrfGeom.h"
#include "modelador/cad/MapSisRef.h"
#include "modelador/cad/matrices/MapEsquemas2d.h"
#include "modelador/cad/matrices/MapEsquemas3d.h"

class Pos3d;
class Recta3d;
class Plano3d;
class Vector3d;

namespace XC {

class Modelador;
class Pnt;
class UniformGrid;
class SetEstruct;
class SisRef;

//! \ingroup Mdlr
//
//! \defgroup Cad Generación de la geometría del modelo.
//
//! @ingroup Cad
//
//! @brief Objeto encargado de generar la geometría del
//! modelo y gestionar las entidades de tipo CAD:
//! puntos, lineas, superficies, sólidos, etc.
//!
//! Mantiene las listas de estas entidades y crea, para
//! cada una de ellas un conjunto destinado a alojar un puntero a la
//! propia entidad, así como punteros a los nodos y elementos contenidos
//! en la entidad.
class Cad: public EntCmd
  {
  private:
    Modelador *mdlr; //!< Modelador.

    template <class L>
    static void numera_lista(L &l);
    void numera(void);
  private:
    MapSisRef sistemas_referencia; //!< Lista de sistemas de referencia.
    MapTrfGeom transformaciones_geometricas; //!< Lista de transformaciones geométricas.

    MapPuntos puntos; //!< Lista de puntos.
    MapLineas edges; //!< Lista de edges.
    MapSuperficies faces; //!< Lista de superficies.
    MapCuerpos cuerpos; //!< Lista de cuerpos.
    MapUniformGrids unif_grid; //!< Lista de mallas uniformes.

    MapEsquemas2d esquemas2d; //!< Lista de esquemas bidimensionales.

    MapEsquemas3d esquemas3d; //!< Lista de esquemas tridimensionales.

  protected:
    virtual bool procesa_comando(CmdStatus &status);
    friend class Modelador;
    SetEstruct *busca_set_estruct(const MapUniformGrids::Indice &nmb);
  public:
    //! @brief Constructor.
    Cad(Modelador *mod= NULL);
    inline Modelador *getModelador(void)
      { return mdlr; } 
    inline const Modelador *getModelador(void) const
      { return mdlr; } 

    Edge *busca_edge_extremos(const MapPuntos::Indice &,const MapPuntos::Indice &);
    const Edge *busca_edge_extremos(const MapPuntos::Indice &,const MapPuntos::Indice &) const;
    
    void conciliaNDivs(void);

    void clearAll(void);
    //! @brief Destructor.
    virtual ~Cad(void);

    inline const MapPuntos &getPuntos(void) const
      { return puntos; }
    inline MapPuntos &getPuntos(void)
      { return puntos; }
    inline const MapLineas &getLineas(void) const
      { return edges; }
    inline MapLineas &getLineas(void)
      { return edges; }
    inline const MapSuperficies &getSuperficies(void) const
      { return faces; }
    inline MapSuperficies &getSuperficies(void)
      { return faces; }
    inline const MapTrfGeom &getTransformacionesGeometricas(void) const
      { return transformaciones_geometricas; }
    inline MapTrfGeom &getTransformacionesGeometricas(void)
      { return transformaciones_geometricas; }
    inline const MapSisRef &getSistemasReferencia(void) const
      { return sistemas_referencia; }
    inline MapSisRef &getSistemasReferencia(void)
      { return sistemas_referencia; }
    inline const MapEsquemas2d &getEsquemas2d(void) const
      { return esquemas2d; }
    inline MapEsquemas2d &getEsquemas2d(void)
      { return esquemas2d; }
    inline const MapEsquemas3d &getEsquemas3d(void) const
      { return esquemas3d; }
    inline MapEsquemas3d &getEsquemas3d(void)
      { return esquemas3d; }
    inline const MapUniformGrids &getUniformGrids(void) const
      { return unif_grid; }
    inline MapUniformGrids &getUniformGrids(void)
      { return unif_grid; }

    any_const_ptr GetProp(const std::string &cod) const;
  };


} //fin namespace XC
#endif