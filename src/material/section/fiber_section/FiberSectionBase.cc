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
//FiberSectionBase.cc

#include "FiberSectionBase.h"
#include "utility/matrix/Vector.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/nucleo/CmdParser.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "xc_utils/src/base/utils_any.h"
#include "xc_utils/src/nucleo/aux_any.h"
#include "material/section/ResponseId.h"
#include "material/section/repres/section/FiberSectionRepr.h"
#include "material/section/repres/geom_section/GeomSection.h"
#include "material/section/diag_interaccion/PlanoDeformacion.h"
#include "material/section/diag_interaccion/ParamAgotTN.h"
#include "material/section/diag_interaccion/DatosDiagInteraccion.h"
#include "material/section/diag_interaccion/CalcPivotes.h"
#include "material/section/diag_interaccion/Pivotes.h"
#include "material/section/diag_interaccion/DiagInteraccion.h"
#include "xc_utils/src/geom/pos_vec/Vector3d.h"
#include "xc_utils/src/geom/d2/MallaTriang3d.h"
#include "xc_utils/src/geom/d3/ConvexHull3d.h"
#include "xc_utils/src/geom/d2/Semiplano2d.h"
#include "xc_utils/src/geom/d2/poligonos2d/bool_op_poligono2d.h"
#include "xc_utils/src/geom/d1/SemiRecta2d.h"
#include "xc_utils/src/geom/d1/Segmento2d.h"
#include "xc_utils/src/nucleo/InterpreteRPN.h"

//! @brief Constructor.
XC::FiberSectionBase::FiberSectionBase(int tag,int num,int classTag,int dim,MaterialLoader *mat_ldr)
  : SeccionBarraPrismatica(tag, classTag,mat_ldr), eTrial(dim), eInic(dim), eCommit(dim), kr(dim), fibras(num), tag_fibra(num+1), section_repres(nullptr)
  {}

//! @brief Constructor.
XC::FiberSectionBase::FiberSectionBase(int tag, int classTag,int dim,MaterialLoader *mat_ldr)
  : SeccionBarraPrismatica(tag, classTag,mat_ldr), eTrial(dim), eInic(dim), eCommit(dim), kr(dim), fibras(0), tag_fibra(0), section_repres(nullptr)
  {}

// constructor for blank object that recvSelf needs to be invoked upon
XC::FiberSectionBase::FiberSectionBase(int classTag,int dim,MaterialLoader *mat_ldr)
  : SeccionBarraPrismatica(0, classTag,mat_ldr), eTrial(dim), eInic(dim), eCommit(dim), kr(dim),fibras(0), tag_fibra(0), section_repres(nullptr)
  {}

//! @brief Constructor de copia.
XC::FiberSectionBase::FiberSectionBase(const FiberSectionBase &otro)
  : SeccionBarraPrismatica(otro), eTrial(otro.eTrial), eInic(otro.eInic), eCommit(otro.eCommit), kr(otro.kr), fibras(otro.fibras), tag_fibra(otro.tag_fibra), section_repres(nullptr)
  {
    if(otro.section_repres)
      section_repres= otro.section_repres->getCopy();
  }

//! @brief Operador asignación.
XC::FiberSectionBase &XC::FiberSectionBase::operator=(const FiberSectionBase &otro)
  {
    SeccionBarraPrismatica::operator=(otro);
    eTrial= otro.eTrial;
    eInic= otro.eInic;
    eCommit= otro.eCommit;
    kr= otro.kr;
    fibras= otro.fibras;
    tag_fibra= otro.tag_fibra;
    if(otro.section_repres)
      section_repres= otro.section_repres->getCopy();
    else
      {
        delete section_repres;
        section_repres= nullptr;
      }
    return *this;
  }

//! @brief Prepara la representación de la sección.
void XC::FiberSectionBase::setup_repres(void)
  {
    if(!section_repres)
      {
        section_repres= new FiberSectionRepr(0,getMaterialLoader());
        section_repres->set_owner(this);
      }
  }

//! @brief Returns fiber section representation.
XC::FiberSectionRepr *XC::FiberSectionBase::getFiberSectionRepr(void)
  {
    setup_repres();
    return section_repres;
  }

void XC::FiberSectionBase::def_section_repres(CmdStatus &status)
  {
    setup_repres();
    section_repres->LeeCmd(status);
    setupFibers();
  }

//! @brief Crea un nuevo conjunto de fibras.
void XC::FiberSectionBase::crea_set_fibras(const std::string &nmb)
  { sets_fibras.crea_set_fibras(nmb); }

//! @brief Crea un conjunto de fibras cuyo nombre se pasa como parámetro.
XC::FiberSectionBase::set_fibras_iterator XC::FiberSectionBase::get_set_fibras(const std::string &nmb_set)
  { return sets_fibras.get_set_fibras(nmb_set); }


//! @brief Crea un conjunto de fibras que cumplen la condición que se pasa como parámetro.
XC::FiberSectionBase::set_fibras_iterator XC::FiberSectionBase::sel(const std::string &nmb_set,const std::string &cond)
  {
    set_fibras_iterator i= get_set_fibras(nmb_set);
    fibras.Cumplen(cond,(*i).second);
    return i;
  }

//! @brief Crea un conjunto de fibras que perteneciendo al conjunto nmb_set_org, cumplen la condición que se pasa como parámetro.
XC::FiberSectionBase::set_fibras_iterator XC::FiberSectionBase::resel(const std::string &nmb_set,const std::string &nmb_set_org,const std::string &cond)
  {
    set_fibras_iterator i= sets_fibras.end();
    if(nmb_set != nmb_set_org)
      {
        i= get_set_fibras(nmb_set);
        set_fibras_iterator j= sets_fibras.find(nmb_set_org);
        if(j == sets_fibras.end())
          {
            std::clog << "Origin fibers set: '" << nmb_set_org
                      << "' doesn't exists; command ignored." << std::endl;
          }
        else
          (*j).second.Cumplen(cond,(*i).second);
      }
    return i;
  }

//! @brief Crea un conjunto de fibras cuyo material tiene el tag que se pasa como parámetro.
XC::FiberSectionBase::set_fibras_iterator XC::FiberSectionBase::sel_mat_tag(const std::string &nmb_set,const int &matTag)
  {
    set_fibras_iterator i= get_set_fibras(nmb_set);
    fibras.SelMatTag(matTag,(*i).second);
    return i;
  }

//! @brief Crea un conjunto de fibras que perteneciendo al conjunto nmb_set_org, cumplen que el material tiene el tag que se pasa como parámetro.
XC::FiberSectionBase::set_fibras_iterator XC::FiberSectionBase::resel_mat_tag(const std::string &nmb_set,const std::string &nmb_set_org,const int &matTag)
  { return sets_fibras.resel_mat_tag(nmb_set,nmb_set_org,matTag); }
 

//! @brief Lee un objeto FiberSectionBase desde archivo
bool XC::FiberSectionBase::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    const std::string cabecera_error= "(FiberSectionBase) Procesando comando: '"+cmd+"'";
    if(verborrea>2)
      std::clog << cabecera_error << std::endl;

    const CmdParser &parser= status.Parser();
    std::string nmb_set= "";
    std::string nmb_set_org= "";
    std::deque<boost::any> fnc_indices;
    if(parser.TieneIndices())
      {
        fnc_indices= status.Parser().SeparaIndices(this);
        if(fnc_indices.size()>0)
          nmb_set= convert_to_string(fnc_indices[0]); //Nombre del conjunto de fibras.
        if(fnc_indices.size()>1)
          nmb_set_org= convert_to_string(fnc_indices[1]); //Nombre del conjunto de fibras.
      }

    if(cmd == "def_section_repr")
      {
        def_section_repres(status);
        return true;
      }
    else if(cmd == "section_repr")
      {
        if(section_repres)
          section_repres->LeeCmd(status);
        else
          {
            const std::string posLectura= status.GetEntradaComandos().getPosicionLecturaActual();
            std::cerr << cabecera_error
                      << " no se ha definido representación para la sección."
                      << posLectura << std::endl;
          }
        return true;
      }
    else if(cmd == "fibras")
      {
        fibras.LeeCmd(status);
        return true;
      }
    else if(cmd == "addFiber")
      {
        const CmdParser &parser= status.Parser();
	std::string nmbMat= "nil";
        double area= 0.0;
        Vector coo;
        if(parser.TieneIndices())
          {
            interpreta(parser.GetIndices());
            if(InterpreteRPN::HayArgumentos(1,cmd))
              {
                tag_fibra= convert_to_int(InterpreteRPN::Pila().Pop()); //Tag  de la fibra.
              }
          }
        else
          tag_fibra++;
        const std::string parametros= status.GetBloque();
        const std::deque<boost::any> tmp= crea_deque_boost_any(parametros);
        const size_t sz= tmp.size();
        if(sz>0)
          {
            if(boost_any_is_string(tmp[0]))
              nmbMat= convert_to_string(tmp[0]);
	    else
	      std::cerr << cabecera_error << "; se esperaba el nombre del material." << std::endl;
            if(sz>1)
              {
                if(boost_any_is_number(tmp[1]))
                  area= convert_to_double(tmp[1]);
                else
		  std::cerr << cabecera_error << "; se esperaba el área de la fibra, se obtuvo un objeto de tipo: "
                            << boost_any_tipo_operando(tmp[1]) << std::endl;
                if(sz>2)
                  {
                    if(boost_any_is_vector_any(tmp[2]))
                      {
                        coo= Vector(convert_to_vector_double(tmp[2]));
	                addFiber(tag_fibra,*GetMaterialLoader(),nmbMat,area,coo);
                      }
                    else
	              std::cerr << cabecera_error << "; se esperaban las coordenadas de la fibra." << std::endl;
                  }
               }
           }
        return true;
      }
    else if(cmd == "nuevo_set_fibras")
      { 
        const std::string nmb_set= interpretaString(status.GetString());
        crea_set_fibras(nmb_set);
        return true;
      }
    else if(cmd == "sel")
      {
        if(fnc_indices.size()<1)
          {
            std::cerr << cmd << " - uso: " << cmd 
                      << "[nmb_set]{cond}" << std::endl;
            status.GetBloque(); //Ignoramos entrada.
            return true;
          }
        const std::string cond= status.GetString(); //Condición.
        sel(nmb_set,cond);
        return true;
      }
    else if(cmd == "resel")
      {
        if(fnc_indices.size()<2)
          {
            std::cerr << cmd << " - uso: " << cmd 
                      << "[nmb_set,nmb_set_org]{cond}" << std::endl;
            status.GetBloque(); //Ignoramos entrada.
            return true;
          }
        if(nmb_set == nmb_set_org)
	  {
	    std::cerr << cmd << " los nombres del conjunto origen"
                      << " y el conjunto destino coinciden. Se ignora el comando." << std::endl;
            status.GetBloque(); //Ignoramos entrada.
          }
        else
          {
            const std::string cond= status.GetString(); //Condición.
            resel(nmb_set,nmb_set_org,cond);
          }
        return true;
      }
    else if(cmd == "sel_mat_tag")
      {
        if(fnc_indices.size()<1)
          {
            std::cerr << cmd << " - uso: " << cmd 
                      << "[nmb_set]{matTag}" << std::endl;
            status.GetBloque(); //Ignoramos entrada.
            return true;
          }
        const int matTag= interpretaInt(status.GetString());
        sel_mat_tag(nmb_set,matTag);
        return true;
      }
    else if(cmd == "resel_mat_tag")
      {
        if(fnc_indices.size()<2)
          {
            std::cerr << cmd << " - uso: " << cmd 
                      << "[nmb_set,nmb_set_org]{matTag}" << std::endl;
            status.GetBloque(); //Ignoramos entrada.
            return true;
          }
        if(nmb_set == nmb_set_org)
	  {
	    std::cerr << cmd << " los nombres del conjunto origen"
                      << " y el conjunto destino coinciden. Se ignora el comando." << std::endl;
            status.GetBloque(); //Ignoramos entrada.
          }
        else
          {
            const int matTag= interpretaInt(status.GetString());
            resel_mat_tag(nmb_set,nmb_set_org,matTag);
          }
        return true;
      }
    else if(cmd == "calc_recubrimientos")
      {
        const std::string nmbSetFibras= interpretaString(status.GetString());
        calcRecubrimientos(nmbSetFibras);
        return true;
      }
    else if(cmd == "calc_separaciones")
      {
        const std::string nmbSetFibras= interpretaString(status.GetString());
        calcSeparaciones(nmbSetFibras);
        return true;
      }
    if(sets_fibras.find(cmd)!=sets_fibras.end())
      {
        sets_fibras[cmd].LeeCmd(status);
        return true;
      }
    else
      return SeccionBarraPrismatica::procesa_comando(status);
  }

//! @brief Destructor:
XC::FiberSectionBase::~FiberSectionBase(void)
  {
    if(section_repres)
      delete section_repres;
    section_repres= nullptr;
  }


//! @brief Add a fiber to the section.
XC::Fiber *XC::FiberSectionBase::addFiber(const std::string &nmbMat,const double &area,const Vector &coo)
  {
    tag_fibra++;
    return addFiber(tag_fibra,*GetMaterialLoader(),nmbMat,area,coo);
  }


//! @brief Establece los valores de las deformaciones iniciales.
int XC::FiberSectionBase::setInitialSectionDeformation(const Vector &deforms)
  {
    eInic= deforms;
    return 0;
  }

//! @brief Establece los valores de las deformaciones de prueba.
int XC::FiberSectionBase::setTrialSectionDeformation(const Vector &deforms)
  {
    eTrial= deforms;
    return 0;
  }

//! @brief Devuelve el vector de deformaciones generalizadas.
const XC::Vector &XC::FiberSectionBase::getSectionDeformation(void) const
  {
    static Vector retval;
    retval= eTrial-eInic;
    return retval;
  }

//! @brief Devuelve el contorno de la sección.
Poligono2d XC::FiberSectionBase::getContornoRegiones(void) const
  {
    Poligono2d retval;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          retval= geom->getContornoRegiones();
        else
	  std::cerr << "FiberSectionBase::getContornoRegiones; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getContornoRegiones; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el canto con el que trabaja la sección a partir de la posición de la fibra neutra.
double XC::FiberSectionBase::getCantoMecanico(void) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          retval= geom->getCantoMecanico(getTrazaPlanoFlexion());
        else
	  std::cerr << "FiberSectionBase::getCantoMecanico; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getCantoMecanico; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el canto desde la recta que se pasa como parámetro a la fibra más comprimida.
double XC::FiberSectionBase::getCantoMecanicoZonaComprimida(const Recta2d &r) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          {
            const Semiplano2d comp= getSemiplanoCompresiones(r);
            if(comp.exists())
              retval= geom->getCantoMecanicoZonaComprimida(comp);
            else
              {
                retval= NAN;
	        std::cerr << "FiberSectionBase::getCantoMecanicoZonaComprimidaR; no se ha podido obtener el semiplano comprimido."
                          << std::endl;
	      }
          }
        else
	  std::cerr << "FiberSectionBase::getCantoMecanicoZonaComprimidaR; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getCantoMecanicoZonaComprimidaR; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el canto desde la fibra neutra a la fibra más comprimida.
double XC::FiberSectionBase::getCantoMecanicoZonaComprimida(void) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          {
            const Semiplano2d comp= getSemiplanoCompresiones();
            if(comp.exists())
              retval= geom->getCantoMecanicoZonaComprimida(comp);
            else
              {
                retval= NAN;
	        std::cerr << "FiberSectionBase::getCantoMecanicoZonaComprimida; no se ha podido obtener el semiplano comprimido."
                          << std::endl;
              }
          }
        else
	  std::cerr << "FiberSectionBase::getCantoMecanicoZonaComprimida; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getCantoMecanicoZonaComprimida; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el canto desde la fibra neutra a la fibra más traccionada.
double XC::FiberSectionBase::getCantoMecanicoZonaTraccionada(void) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          {
            const Semiplano2d comp= getSemiplanoCompresiones();
            if(comp.exists())
              retval= geom->getCantoMecanicoZonaTraccionada(comp);
            else //Toda la sección esta en tracción.
              retval= geom->getCantoMecanico(getTrazaPlanoFlexion());
          }
        else
	  std::cerr << "FiberSectionBase::getCantoMecanicoZonaTraccionada; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getCantoMecanicoZonaTraccionada; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el canto desde la recta que se pasa como parámetro a la fibra más traccionada.
double XC::FiberSectionBase::getCantoMecanicoZonaTraccionada(const Recta2d &r) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          {
            const Semiplano2d comp= getSemiplanoCompresiones(r);
            if(comp.exists())
              retval= geom->getCantoMecanicoZonaTraccionada(comp);
            else
              {
                retval= NAN;
	        std::cerr << "FiberSectionBase::getCantoMecanicoZonaTraccionada; no se ha podido obtener el semiplano comprimido."
                          << std::endl;
              }
          }
        else
	  std::cerr << "FiberSectionBase::getCantoMecanicoZonaTraccionada; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getCantoMecanicoZonaTraccionada; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve la profundidad de la fibra neutra.
double XC::FiberSectionBase::getNeutralAxisDepth(void) const
  {
    double retval= getCantoMecanicoZonaComprimida();
    return retval;
  }

//! @brief Devuelve la distancia desde la fibra neutra
//! hasta el punto cuyas coordenadas se pasan como parámetro.
double XC::FiberSectionBase::getDistFibraNeutra(const double &y,const double &) const
  {
    double retval= 0.0;
    const Recta2d fn= getFibraNeutra();
    if(fn.exists())
      retval= fn.dist(Pos2d(y,0));
    return retval;
  }

//! @brief Devuelve la recta que limita el área eficaz de hormigón Ac,ef
//! según el artículo 49.2.4 de la EHE-08 (área rallada figura 49.2.4b).
//! Ver también figuras 47.5 y 47.6 del tomo II del libro "Proyecto y cálculo de estructuras
//! de hormigón" de Calavera.
Recta2d XC::FiberSectionBase::getRectaLimiteAcEficaz(const double &hEfMax) const
  {
    Recta2d retval;
    Recta2d fn= getFibraNeutra();
    if(!fn.exists())
      fn= fibras.getFibraNeutra();
    if(fn.exists())
      {
        const double hef= getCantoMecanicoZonaTraccionada();
        assert(!isnan(hef));
        if(hef<hEfMax)
          retval= fn;
        else
          {
            const double d= -(hef-hEfMax); //Cambiamos el signo para desplazar hacia las tracciones.
            const Vector v= normalize(getVectorBrazoMecanico())*d;
            retval= fn.Offset(Vector2d(v[0],v[1]));
          }
      }
    return retval;
  }

//! @brief Devuelve la el contorno que limita el área eficaz de hormigón Ac,ef
//! según el artículo 49.2.4 de la EHE-08 (área rallada figura 49.2.4b).
//! Ver también figuras 47.5 y 47.6 del tomo II del libro "Proyecto y cálculo de estructuras
//! de hormigón" de Calavera.
std::list<Poligono2d> XC::FiberSectionBase::getContornoAcEficazBruta(const double &hEfMax) const
  {
    std::list<Poligono2d> retval;
    Poligono2d contorno= getContornoRegiones();

    const double epsMin= fibras.getStrainMin();
    const double epsMax= fibras.getStrainMax();
    if(epsMin>0) //Toda la sección en tracción.
      retval.push_back(contorno);
    else if(epsMax>0) //Flexión.
      {
        if(hEfMax>1e-6)
          {
            const Recta2d limite= getRectaLimiteAcEficaz(hEfMax);
            if(limite.exists())
              {
                const Semiplano2d areaTracciones= getSemiplanoTracciones(limite);
                assert(areaTracciones.exists());
                retval= contorno.Interseccion(areaTracciones);
              }
            else
              retval.push_back(contorno);
          }
        else
          std::cerr << "FiberSectionBase::getContornoAcEficazBruta; la altura eficaz máxima es nula." << std::endl;
      }
    if(retval.empty())
      {
        std::cerr << "FiberSectionBase::getContornoAcEficazBruta; no se pudo determinar el contorno del área eficaz bruta."
                  << std::endl;
      }
    return retval;
  }

double XC::FiberSectionBase::getAcEficazBruta(const double &hEfMax) const
  {
    std::list<Poligono2d> tmp= getContornoAcEficazBruta(hEfMax);
    return area(tmp.begin(),tmp.end());
  }

//! @brief Devuelve la suma de las áreas eficaces de las barras a tracción.
double XC::FiberSectionBase::getAcEficazNeta(const double &hEfMax,const std::string &nmbSetArmaduras,const double &factor) const
  {
    double retval= 0.0;
    std::list<Poligono2d> contornoAcEficazBruta= getContornoAcEficazBruta(hEfMax);
    if(!contornoAcEficazBruta.empty())
      {
        set_fibras_const_iterator i= sets_fibras.find(nmbSetArmaduras);
        if(i!=sets_fibras.end())
          {
            const DqFibras &armaduras= (*i).second; //Armaduras.
            retval= armaduras.calcAcEficazFibras(contornoAcEficazBruta,factor);
          }
        else
          std::cerr << "No se encotró el conjunto de fibras: "
                    << nmbSetArmaduras << std::endl;
      }
    else
      std::cerr << "La sección no tiene área eficaz." << std::endl;
    return retval;
  }

//! @brief Calcula las áreas eficaces a fisuración en torno a las fibras
double XC::FiberSectionBase::calcAcEficazFibras(const double &hEfMax,const std::string &nmbSetArmaduras,const double &factor) const
  {
    double retval= 0;
    std::list<Poligono2d> contornoAcEficazBruta= getContornoAcEficazBruta(hEfMax);
    if(!contornoAcEficazBruta.empty())
      {
        set_fibras_const_iterator i= sets_fibras.find(nmbSetArmaduras);
        if(i!=sets_fibras.end())
          {
            const DqFibras &armaduras= (*i).second; //Armaduras.
            retval= armaduras.calcAcEficazFibras(contornoAcEficazBruta,factor);
          }
        else
          std::cerr << "FiberSectionBase::calcAcEficazFibras; no se encotró el conjunto de fibras: "
                    << nmbSetArmaduras << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::calcAcEficazFibras; la sección no tiene área eficaz." << std::endl;
    return retval;
  }

//! @brief Calcula los recubrimientos de las fibras.
void XC::FiberSectionBase::calcRecubrimientos(const std::string &nmbSetArmaduras) const
  {
    set_fibras_const_iterator i= sets_fibras.find(nmbSetArmaduras);
    if(i!=sets_fibras.end())
      {
        const DqFibras &armaduras= (*i).second; //Armaduras.
        if(section_repres)
          {
            const GeomSection *geom= section_repres->getGeom();
            if(geom)
              armaduras.calcRecubrimientos(*geom);
            else
	      std::cerr << "FiberSectionBase::calcRecubrimientos; no se ha definido la geometría de la sección."
                        << std::endl;
          }
        else
          std::cerr << "FiberSectionBase::calcRecubrimientos; no se ha definido la representación de la sección."
                    << std::endl;    
      }
    else
      std::cerr << "FiberSectionBase::calcRecubrimientos; no se encotró el conjunto de fibras: "
                << nmbSetArmaduras << std::endl;
  }

//! @brief Calcula las separaciones de las fibras.
void XC::FiberSectionBase::calcSeparaciones(const std::string &nmbSetArmaduras) const
  {
    set_fibras_const_iterator i= sets_fibras.find(nmbSetArmaduras);
    if(i!=sets_fibras.end())
      {
        const DqFibras &armaduras= (*i).second; //Armaduras.
        armaduras.calcSeparaciones();
      }
    else
      std::cerr << "FiberSectionBase::calcSeparaciones; no se encontró el conjunto de fibras: "
                << nmbSetArmaduras << std::endl;
  }

//! @brief Devuelve la distancia (con signo) desde la fibra neutra
//! hasta el punto cuyas coordenadas se pasan como parámetro.
double XC::FiberSectionBase::get_dist_to_neutral_axis(const double &y,const double &z) const
  {
    double retval= 0;
    const Semiplano2d comp= getSemiplanoCompresiones();
    if(comp.exists())
      retval= comp.DistSigno(Pos2d(y,z));
    else
      retval= NAN;
    return retval;
  }

//! @brief Devuelve la matriz de rigidez tangente.
const XC::Matrix &XC::FiberSectionBase::getSectionTangent(void) const
  { return kr.Rigidez(); }

//! @brief Devuelve la resultante de las tensiones en la sección.
const XC::Vector &XC::FiberSectionBase::getStressResultant(void) const
  { return kr.Resultante(); }

//! @brief Devuelve la componente de la resultante de tensiones en la sección de índice i.
double XC::FiberSectionBase::getStressResultant(const int &i) const
  { return SeccionBarraPrismatica::getStressResultant(i); }


//! @brief Consuma el estado de la sección.
int XC::FiberSectionBase::commitState(void)
  {
    int err= fibras.commitState();
    eCommit= eTrial;
    return err;
  }

//! @brief Devuelve el estado de la sección al último consumado
int XC::FiberSectionBase::revertToLastCommit(void)
  {
    // Last committed section deformations
    eTrial= eCommit;
    return 0;
  }

//! @brief Devuelve la sección a su estado inicial.
int XC::FiberSectionBase::revertToStart(void)
  {
    eCommit.Zero();
    return 0;
  }

//! @brief Devuelve el punto que corresponde a la resultante
//! de tensiones normales en la sección.
Pos3d XC::FiberSectionBase::Esf2Pos3d(void) const
  { return Pos3d(getStressResultant(XC::SECTION_RESPONSE_P),getStressResultant(XC::SECTION_RESPONSE_MY),getStressResultant(XC::SECTION_RESPONSE_MZ)); }

//! @brief Inserta en la lista de esfuerzos que se pasa como parámetro
//! la resultante de tensiones normales en la sección.
const Pos3d *XC::FiberSectionBase::InsertaEsfuerzo(const PlanoDeformacion &def,GeomObj::list_Pos3d &lista_esfuerzos,const Pos3d *ultimo_insertado, const double &umbral)
  {
    setTrialDeformationPlane(def);
    const Pos3d p_esf= Esf2Pos3d();
    if(ultimo_insertado)
      {
        if(dist(p_esf,*ultimo_insertado)>umbral)
          return lista_esfuerzos.Agrega(p_esf);
        else
          return ultimo_insertado;
      }
    else
      return lista_esfuerzos.Agrega(p_esf);  
  }

//! @brief Devuelve los puntos que definen el diagrama de interacción de la sección para un ángulo theta dado.
const Pos3d *XC::FiberSectionBase::get_ptos_diag_interaccion_theta(GeomObj::list_Pos3d &lista_esfuerzos,const DatosDiagInteraccion &datos_diag,const DqFibras &fsC,const DqFibras &fsS,const double &theta)
  {
    const Pos3d *ultimo_insertado= nullptr; //Ultima terna insertada.
    CalcPivotes cp(datos_diag.getDefsAgotPivotes(),fibras,fsC,fsS,theta);
    Pivotes pivotes(cp);
    //Dominios 1 y 2
    Pos3d P1= pivotes.GetPivoteA(); //Pivote.
    Pos3d P2= P1+100.0*cp.GetK(); //Flexión en torno al eje Z.
    Pos3d P3;
    PlanoDeformacion def;
    const double inc_eps_B= datos_diag.getIncEps();
    const double eps_agot_A= datos_diag.getDefsAgotPivotes().getDefAgotPivoteA();
    const double eps_agot_B= datos_diag.getDefsAgotPivotes().getDefAgotPivoteB();
    const double eps_agot_C= datos_diag.getDefsAgotPivotes().getDefAgotPivoteC();
    for(double e= eps_agot_A;e>=eps_agot_B;e-=inc_eps_B)
      {
        P3= pivotes.GetPuntoB(e);
        ultimo_insertado= InsertaEsfuerzo(PlanoDeformacion(P1,P2,P3),lista_esfuerzos,ultimo_insertado,datos_diag.getUmbral());
      }
    //Dominios 3 y 4
    P1= pivotes.GetPivoteB(); //Pivote
    P2= P1+100.0*cp.GetK(); 
    const double inc_eps_A= datos_diag.getIncEps();
    for(double e= eps_agot_A;e>=0.0;e-=inc_eps_A)
      {
        P3= pivotes.GetPuntoA(e);
        ultimo_insertado= InsertaEsfuerzo(PlanoDeformacion(P1,P2,P3),lista_esfuerzos,ultimo_insertado,datos_diag.getUmbral());
      }
    //Dominio 4a
    //Calculamos la deformación en D cuando el pivote es B
    //y la deformación en A es nula (inicio del dominio 4a).
    const Pos3d PA0= pivotes.GetPuntoA(0.0); //Deformación nula en la armadura.
    const PlanoDeformacion def_lim_4a= PlanoDeformacion(P1,P2,PA0);
    const Pos2d PD= pivotes.GetPosPuntoD();
    const double eps_D4a= def_lim_4a.Deformacion(PD);
    const double recorr_eps_D4a= eps_D4a;
    if(recorr_eps_D4a>(eps_agot_A/200.0)) //Si el recorrido es positivo y "apreciable"
      {
        const double inc_eps_D4a= datos_diag.getIncEps(); //recorr_eps_D4a/n_div_dominio; //Intervalos de cálculo. 
        for(double e= eps_D4a;e>=0.0;e-=inc_eps_D4a)
          {
            P3= pivotes.GetPuntoD(e);
            ultimo_insertado= InsertaEsfuerzo(PlanoDeformacion(P1,P2,P3),lista_esfuerzos,ultimo_insertado,datos_diag.getUmbral());
          }
      }
    //Dominio 5
    P1= pivotes.GetPivoteC(); //Pivote
    P2= P1+100.0*cp.GetK(); 
    const double inc_eps_D= datos_diag.getIncEps();
    for(double e= 0.0;e>=eps_agot_C;e-=inc_eps_D)
      {
        P3= pivotes.GetPuntoD(e);
        ultimo_insertado= InsertaEsfuerzo(PlanoDeformacion(P1,P2,P3),lista_esfuerzos,ultimo_insertado,datos_diag.getUmbral());
      }
    return ultimo_insertado;
  }

//! @brief Devuelve los puntos que definen el diagrama de interacción de la sección.
const GeomObj::list_Pos3d &XC::FiberSectionBase::get_ptos_diag_interaccion(const DatosDiagInteraccion &datos_diag)
  {
    static GeomObj::list_Pos3d lista_esfuerzos;
    lista_esfuerzos.clear();
    const DqFibras &fsC= sel_mat_tag(datos_diag.getNmbSetHormigon(),datos_diag.getTagHormigon())->second;
    if(fsC.empty())
      std::cerr << "No se han encontrado fibras del material de tag: " << datos_diag.getTagHormigon()
                << " que corresponde al hormigón." << std::endl;
    const DqFibras &fsS= sel_mat_tag(datos_diag.getNmbSetArmadura(),datos_diag.getTagArmadura())->second;
    if(fsS.empty())
      std::cerr << "No se han encontrado fibras del material de tag: " << datos_diag.getTagArmadura()
                << " que corresponde al acero de armar." << std::endl;
    if(!fsC.empty() && !fsS.empty())
      {
        const Pos3d *ultimo_insertado= nullptr; //Ultima terna insertada.
        for(double theta= 0.0;theta<2*M_PI;theta+=datos_diag.getIncTheta())
          ultimo_insertado= get_ptos_diag_interaccion_theta(lista_esfuerzos,datos_diag,fsC,fsS,theta);
        revertToStart();
      }
    else
      std::cerr << "No se pudo obtener el diagrama de interacción." << std::endl;
    return lista_esfuerzos;
  }

//! @brief Devuelve el diagrama de interacción.
XC::DiagInteraccion XC::FiberSectionBase::GetDiagInteraccion(const DatosDiagInteraccion &datos_diag)
  {
    const GeomObj::list_Pos3d lp= get_ptos_diag_interaccion(datos_diag);
    DiagInteraccion retval;
    if(!lp.empty())
      {
        retval= DiagInteraccion(Pos3d(0,0,0),MallaTriang3d(get_convex_hull(lp)));
        const double error= fabs(retval.FactorCapacidad(lp).Norm2()-lp.size())/lp.size();
        if(error>0.005)
	  std::cerr << "FiberSectionBase::GetDiagInteraccion; el error en el cálculo del diagrama de interacción ("
                    << error << ") es grande." << std::endl;
      }
    return retval;
  }

//! @brief Devuelve un vector orientado desde el centro de tracciones al de compresiones.
XC::Vector XC::FiberSectionBase::getVectorBrazoMecanico(void) const
  { return fibras.getVectorBrazoMecanico(); }

//! @brief Devuelve un vector orientado desde el centro de tracciones a
//! la fibra más comprimida.
XC::Vector XC::FiberSectionBase::getVectorCantoUtil(void) const
  {
    Vector retval(2);
    const Segmento2d tmp= getSegmentoCantoUtil();
    const Pos2d p1= tmp.Origen();
    const Pos2d p2= tmp.Destino();
    retval(0)= p2.x()-p1.x();
    retval(1)= p2.y()-p1.y();
    return retval;
  }

//! @brief Devuelve un segmento orientado desde el centro de tracciones al de compresiones.
Segmento2d XC::FiberSectionBase::getSegmentoBrazoMecanico(void) const
  {
    Segmento2d retval= fibras.getSegmentoBrazoMecanico();
    if(!retval.exists())
      {
        //Calculamos el brazo mecánico como 0.8 veces
        //el canto total.
        const Recta2d ejeX= getEjeEsfuerzos();
        const Pos2d cdg= getCdg();
        const Recta2d ejeY= ejeX.Perpendicular(cdg);
        const Poligono2d contorno= getContornoRegiones();
        retval= contorno.Clip(ejeY);
        Pos2d org= retval.Origen()+0.1*retval.GetVector();
        Pos2d dest= retval.Destino()-0.1*retval.GetVector();
        retval= Segmento2d(org,dest);
      }
    return retval;
  }

//! @brief Devuelve el segmento definido por el canto útil con el que está trabajando la sección.
//! el segmento se orienta desde el centro de tracciones hacia la fibra más comprimida.
Segmento2d XC::FiberSectionBase::getSegmentoCantoUtil(void) const
  {
    Segmento2d retval;
    const Segmento2d bm= getSegmentoBrazoMecanico();
    const SemiRecta2d sr(bm.Origen(),bm.Destino());
    const Poligono2d contorno= getContornoRegiones();
    retval= contorno.Clip(sr);
    return retval;
  }

//! @brief Devuelve la traza del plano de flexión en el plano de la sección.
Recta2d XC::FiberSectionBase::getTrazaPlanoFlexion(void) const
  {
    Recta2d retval= fibras.getTrazaPlanoFlexion();
    if(!retval.exists())
      {
        Recta2d eje= getEjeEsfuerzos();
        Pos2d cdg= getCdg();
        retval= eje.Perpendicular(cdg);
      } 
    return retval;
  }

//! @brief Devuelve la traza de un plano perpendicular al de flexión
//! que pasa por el baricentro de tracciones.
Recta2d XC::FiberSectionBase::getTrazaPlanoTraccion(void) const
  {
    Recta2d retval= fibras.getTrazaPlanoTraccion();
    if(!retval.exists())
      std::cerr << "No se encontró la traza del plano de tracción." << std::endl;
    return retval;
  }

//! @brief Devuelve la traza de un plano perpendicular al de flexión
//! que pasa por el baricentro de compresiones.
Recta2d XC::FiberSectionBase::getTrazaPlanoCompresion(void) const
  {
    Recta2d retval= fibras.getTrazaPlanoCompresion();
    if(!retval.exists())
      std::cerr << "No se encontró la traza del plano de compresión." << std::endl;
    return retval;
  }

//! @brief Devuelve el ancho con el que trabaja la sección.
double XC::FiberSectionBase::getAnchoMecanico(void) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          retval= geom->getAnchoMecanico(getTrazaPlanoFlexion());
        else
	  std::cerr << "FiberSectionBase::getAnchoMecanico; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getAnchoMecanico; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el ancho de la biela comprimida (en cortante).
double XC::FiberSectionBase::getAnchoBielaComprimida(void) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          retval= geom->getAnchoBielaComprimida(getSegmentoBrazoMecanico());
        else
	  std::cerr << "FiberSectionBase::getAnchoBielaComprimida; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getAnchoBielaComprimida; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el recubrimiento de la posición que se pasa como parámetro.
double XC::FiberSectionBase::getRecubrimiento(const Pos2d &p) const
  {
    double retval= 0.0;
    if(section_repres)
      {
        const GeomSection *geom= section_repres->getGeom();
        if(geom)
          retval= geom->getRecubrimiento(p);
        else
	  std::cerr << "FiberSectionBase::getRecubrimiento; no se ha definido la geometría de la sección."
                    << std::endl;
      }
    else
      std::cerr << "FiberSectionBase::getRecubrimiento; no se ha definido la representación de la sección."
                << std::endl;
    return retval;
  }

//! @brief Devuelve el brazo mecánico con el que está trabajando la sección.
double XC::FiberSectionBase::getBrazoMecanico(void) const
  { return fibras.getBrazoMecanico(); }

//! @brief Devuelve el canto útil con el que está trabajando la sección.
double XC::FiberSectionBase::getCantoUtil(void) const
  { return getVectorCantoUtil().Norm(); }

//! @brief Devuelve el area.
double XC::FiberSectionBase::getArea(void) const
  { return fibras.getSumaAreas(); }

//! @brief Moment of inertia relative to bending axis.
double XC::FiberSectionBase::getHomogeinizedI(const double &E0) const
  {
    const Recta2d eje= getEjeEsfuerzos();
    return fibras.getISeccHomogeneizada(E0,eje);
  }

//! @brief Static moment relative to bending axis of area that rests over this axis.
double XC::FiberSectionBase::getSPosHomogeneizada(const double &E0) const
  {
    const Recta2d eje= getEjeEsfuerzos();
    return fibras.getSPosSeccHomogeneizada(E0,Semiplano2d(eje));
  }

std::string XC::FiberSectionBase::getStrClaseEsfuerzo(const double &tol) const
  { return fibras.getStrClaseEsfuerzo(); }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::FiberSectionBase::GetProp(const std::string &cod) const
  {
    if(verborrea>4)
      std::clog << "FiberSectionBase::GetProp (" << nombre_clase() << "::GetProp) Buscando propiedad: " << cod << std::endl;
    static m_double tmp_m_double;

    set_fibras_const_iterator iter=sets_fibras.find(cod);

    if(cod=="numFibras")
      {
        tmp_gp_szt= getNumFibers();
        return any_const_ptr(tmp_gp_szt);
      }
    else if(cod=="numSets")
      {
        tmp_gp_szt= sets_fibras.size();
        return any_const_ptr(tmp_gp_szt);
      }
    else if(cod=="yCdg")
      {
        tmp_gp_dbl= getCdgY();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="zCdg")
      {
        tmp_gp_dbl= getCdgZ();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if((cod=="getSumaAreas") || (cod=="getArea"))
      {
        tmp_gp_dbl= fibras.getSumaAreas();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getResultanteTracc")
      {
        tmp_gp_dbl= fibras.ResultanteTracc();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="MzTracc")
      {
        tmp_gp_dbl= fibras.getMzTracc(fibras.getYCdg());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getBaricentroTracc")
      {
        return get_prop_vector(fibras.baricentroTracciones());
      }
    else if(cod=="getResultanteComp")
      {
        tmp_gp_dbl= fibras.ResultanteComp();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="MzComp")
      {
        tmp_gp_dbl= fibras.getMzComp(fibras.getYCdg());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getBaricentroComp")
      {
        return get_prop_vector(fibras.baricentroCompresiones());
      }
    else if(cod=="Iy")
      {
        tmp_gp_dbl= fibras.getIy(1,getCdgZ());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="Iz")
      {
        tmp_gp_dbl= fibras.getIz(1,getCdgY());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="Pyz")
      {
        tmp_gp_dbl= fibras.getPyz(1,getCdgY(),getCdgZ());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="I1")
      {
        tmp_gp_dbl= fibras.getI1(1,getCdgY(),getCdgZ());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="i1")
      {
        tmp_gp_dbl= sqrt(fibras.getI1(1,getCdgY(),getCdgZ())/fibras.getSumaAreas());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="I2")
      {
        tmp_gp_dbl= fibras.getI2(1,getCdgY(),getCdgZ());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="i2")
      {
        tmp_gp_dbl= sqrt(fibras.getI2(1,getCdgY(),getCdgZ())/fibras.getSumaAreas());
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="eje1")
      {
        tmp_gp_mdbl= vector_to_m_double(fibras.getEje1(getCdgY(),getCdgZ()));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="eje2")
      {
        tmp_gp_mdbl= vector_to_m_double(fibras.getEje2(getCdgY(),getCdgZ()));
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getIHomogeneizada")
      {
        //Devuelve el momento de inercia de la sección homogeneizada
        //respecto a la fibra neutra.
        if(InterpreteRPN::Pila().size()>0)
          {
            const double E0= convert_to_double(InterpreteRPN::Pila().Pop());
            const Recta2d eje= getEjeEsfuerzos();
            tmp_gp_dbl= fibras.getISeccHomogeneizada(E0,eje);
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);          
            tmp_gp_dbl= 0.0;
          }
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="SzPos")
      {
        double factor= 1.0;
        double y0= 0.0;
        double yf= 0.0;
        if(InterpreteRPN::Pila().size()>2)
          {
            factor= convert_to_double(InterpreteRPN::Pila().Pop());
            y0= convert_to_double(InterpreteRPN::Pila().Pop());
            yf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSzPos(yf,y0,factor);
          }
        else if(InterpreteRPN::Pila().size()>1)
          {
            y0= convert_to_double(InterpreteRPN::Pila().Pop());
            yf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSzPos(yf,y0);
          }
        else if(InterpreteRPN::Pila().size()>0)
          {
            y0= getCdgY();
            yf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSzPos(yf,y0);
          }
        else
          {
            y0= getCdgY();
            yf= y0;
            tmp_gp_dbl= fibras.getSzPos(yf,y0);
          }
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="SzNeg")
      {
        double factor= 1.0;
        double y0= 0.0;
        double yf= 0.0;
        if(InterpreteRPN::Pila().size()>2)
          {
            factor= convert_to_double(InterpreteRPN::Pila().Pop());
            y0= convert_to_double(InterpreteRPN::Pila().Pop());
            yf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSzNeg(yf,y0,factor);
          }
        else if(InterpreteRPN::Pila().size()>1)
          {
            y0= convert_to_double(InterpreteRPN::Pila().Pop());
            yf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSzNeg(yf,y0);
          }
        else if(InterpreteRPN::Pila().size()>0)
          {
            y0= getCdgY();
            yf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSzNeg(yf,y0);
          }
        else
          {
            y0= getCdgY();
            yf= y0;
            tmp_gp_dbl= fibras.getSzNeg(yf,y0);
          }
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="SyPos")
      {
        double factor= 1.0;
        double z0= 0.0;
        double zf= 0.0;
        if(InterpreteRPN::Pila().size()>2)
          {
            factor= convert_to_double(InterpreteRPN::Pila().Pop());
            z0= convert_to_double(InterpreteRPN::Pila().Pop());
            zf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSyPos(zf,z0,factor);
          }
        else if(InterpreteRPN::Pila().size()>1)
          {
            z0= convert_to_double(InterpreteRPN::Pila().Pop());
            zf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSyPos(zf,z0);
          }
        else if(InterpreteRPN::Pila().size()>0)
          {
            z0= getCdgZ();
            zf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSyPos(zf,z0);
          }
        else
          {
            z0= getCdgZ();
            zf= z0;
            tmp_gp_dbl= fibras.getSyPos(zf,z0);
          }
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="SyNeg")
      {
        double factor= 1.0;
        double z0= 0.0;
        double zf= 0.0;
        if(InterpreteRPN::Pila().size()>2)
          {
            factor= convert_to_double(InterpreteRPN::Pila().Pop());
            z0= convert_to_double(InterpreteRPN::Pila().Pop());
            zf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSyNeg(zf,z0,factor);
          }
        else if(InterpreteRPN::Pila().size()>1)
          {
            z0= convert_to_double(InterpreteRPN::Pila().Pop());
            zf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSyNeg(zf,z0);
          }
        else if(InterpreteRPN::Pila().size()>0)
          {
            z0= getCdgZ();
            zf= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= fibras.getSyNeg(zf,z0);
          }
        else
          {
            z0= getCdgZ();
            zf= z0;
            tmp_gp_dbl= fibras.getSyNeg(zf,z0);
          }
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getSPosHomogeneizada")
      {
        //Devuelve el momento estático de la sección homogeneizada
        //que queda por encima de la fibra neutra.
        if(InterpreteRPN::Pila().size()>0)
          {
            const double E0= convert_to_double(InterpreteRPN::Pila().Pop());
            const Recta2d eje= getEjeEsfuerzos();
            tmp_gp_dbl= fibras.getSPosSeccHomogeneizada(E0,Semiplano2d(eje));
          }
        else
          {
            err_num_argumentos(std::cerr,1,"GetProp",cod);          
            tmp_gp_dbl= 0.0;
          }
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="INearest")
      {
        double y= 0.0;
        double z= 0.0;
        if(InterpreteRPN::Pila().size()>1)
          {
            z= convert_to_double(InterpreteRPN::Pila().Pop());
            y= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_szt= fibras.nearest_fiber(y,z);
          }
        else if(InterpreteRPN::Pila().size()>0)
          {
            z= getCdgZ();
            y= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_szt= fibras.nearest_fiber(y,z);
          }
        else
          {
            z= getCdgZ();
            y= getCdgY();
            tmp_gp_szt= fibras.nearest_fiber(y,z);
          }
        return any_const_ptr(tmp_gp_szt);
      }
    else if(cod=="getNeutralAxisDepth")
      {
        tmp_gp_dbl= getNeutralAxisDepth();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="IMaxProp")
      return fibras.GetProp(cod);
    else if(cod=="IMinProp")
      return fibras.GetProp(cod);
    else if(cod=="fibra")
      return fibras.GetProp(cod);
    else if((iter) != sets_fibras.end())
      return any_const_ptr((*iter).second);
    else if(cod=="getVectorBrazoMecanico")
      {
        tmp_gp_mdbl= vector_to_m_double(getVectorBrazoMecanico());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getVectorCantoUtil")
      {
        tmp_gp_mdbl= vector_to_m_double(getVectorCantoUtil());
        return any_const_ptr(tmp_gp_mdbl);
      }
    else if(cod=="getBrazoMecanico")
      {
        tmp_gp_dbl= getBrazoMecanico();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getCantoMecanico")
      {
        tmp_gp_dbl= getCantoMecanico();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getCantoUtil")
      {
        tmp_gp_dbl= getCantoUtil();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getAnchoMecanico")
      {
        tmp_gp_dbl= getAnchoMecanico();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getAnchoBielaComprimida")
      {
        tmp_gp_dbl= getAnchoBielaComprimida();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getRecubrimiento")
      {
        double y,z;
        if(InterpreteRPN::Pila().size()>1)
          {
            z= convert_to_double(InterpreteRPN::Pila().Pop());
            y= convert_to_double(InterpreteRPN::Pila().Pop());
          }
        else
          err_num_argumentos(std::cerr,2,"GetProp",cod);
        tmp_gp_dbl= getRecubrimiento(Pos2d(y,z));
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStrClaseEsfuerzo")
      {
        tmp_gp_str= fibras.getStrClaseEsfuerzo();
        return any_const_ptr(tmp_gp_str);
      }
    else if(cod=="getAcEficazBruta")
      {
        double hEfMax= 0.0;
        if(InterpreteRPN::Pila().size()>0)
          {
            hEfMax= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= getAcEficazBruta(hEfMax);
          }
        else
          err_num_argumentos(std::cerr,1,"GetProp",cod);          
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getAcEficazNeta")
      {
        double hEfMax= 0.0;
        if(InterpreteRPN::Pila().size()>2)
          {
            const double factor= convert_to_double(InterpreteRPN::Pila().Pop());
            const std::string nmb= convert_to_string(InterpreteRPN::Pila().Pop());
            hEfMax= convert_to_double(InterpreteRPN::Pila().Pop());
            tmp_gp_dbl= getAcEficazNeta(hEfMax,nmb,factor);
          }
        else
          err_num_argumentos(std::cerr,3,"GetProp",cod);          
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="calcAcEficazFibras")
      {
        double hEfMax= 0.0;
	std::string nmbSetFibras;
        double factor= 0.0;
        if(InterpreteRPN::Pila().size()>2)
          {
            factor= convert_to_double(InterpreteRPN::Pila().Pop());
            nmbSetFibras= convert_to_string(InterpreteRPN::Pila().Pop());
            hEfMax= convert_to_double(InterpreteRPN::Pila().Pop());
          }
        else
          err_num_argumentos(std::cerr,3,"GetProp",cod);
        tmp_gp_dbl= calcAcEficazFibras(hEfMax,nmbSetFibras,factor);
        return any_const_ptr(tmp_gp_dbl);
      }
    else
      return SeccionBarraPrismatica::GetProp(cod);
  }