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
/* ****************************************************************** **
**    OpenSees - Open System for Earthquake Engineering Simulation    **
**          Pacific Earthquake Engineering Research Center            **
**                                                                    **
**                                                                    **
** (C) Copyright 1999, The Regents of the University of California    **
** All Rights Reserved.                                               **
**                                                                    **
** Commercial use of this program without express permission of the   **
** University of California, Berkeley, is strictly prohibited.  See   **
** file 'COPYRIGHT'  in XC::main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */

// $Revision: 1.23 $
// $Date: 2003/08/13 23:52:23 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/truss/Truss.cpp,v $


// File: ~/element/truss/Truss.C
//
// Written: fmk
// Created: 07/98
// Revision: A
//
// Description: This file contains the implementation for the XC::Truss class.
//
// What: "@(#) Truss.C, revA"

#include "domain/mesh/element/truss_beam_column/truss/Truss.h"
#include <domain/mesh/element/Information.h>

#include <domain/domain/Domain.h>
#include <domain/mesh/node/Node.h>
#include <utility/actor/objectBroker/FEM_ObjectBroker.h>
#include <material/uniaxial/UniaxialMaterial.h>
#include <material/uniaxial/CableMaterial.h>
#include <domain/load/ElementalLoad.h>
#include "domain/load/beam_loads/TrussStrainLoad.h"
#include <utility/matrix/Matrix.h>
#include <utility/recorder/response/ElementResponse.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"
#include "domain/component/Parameter.h"
#include "utility/actor/actor/ArrayCommMetaData.h"

void XC::Truss::libera_material(void)
  {
    if(theMaterial)
      {
        delete theMaterial;
        theMaterial= nullptr;
      }
  }

void XC::Truss::libera_load_sens(void)
  {
    if(theLoadSens)
      {
        delete theLoadSens;
        theLoadSens= nullptr;
      }
  }

void XC::Truss::libera(void)
  {
    libera_material();
    libera_load_sens();
  }

void XC::Truss::set_material(const UniaxialMaterial &mat)
  {
    libera_material();
    // get a copy of the material and check we obtained a valid copy
    theMaterial= mat.getCopy();
    if(!theMaterial)
      {
        std::cerr << "FATAL Truss::set_material - " << getTag() <<
          "failed to get a copy of material with tag " << mat.getTag() << std::endl;
        exit(-1);
      }
  }

void XC::Truss::set_load_sens(const Vector &v)
  {
    libera_load_sens();
    // get a copy of the material and check we obtained a valid copy
    theLoadSens= new Vector(v);
    if(!theLoadSens)
      {
        std::cerr << "FATAL Truss::set_load_sens - " << getTag() <<
          "failed to get a copy of vector: " << v << std::endl;
        exit(-1);
      }
  }

//! @brief Pone a cero los punteros a nodo, los cosenos directores y los parámetros de sensibilidad.
void XC::Truss::inicializa(void)
  {
    TrussBase::inicializa();
// AddingSensitivity:BEGIN /////////////////////////////////////
    parameterID = 0;
// AddingSensitivity:END //////////////////////////////////////
  }

//! @brief constructor:
//!  responsible for allocating the necessary space needed by each object
//!  and storing the tags of the truss end nodes.
XC::Truss::Truss(int tag,int dim,int Nd1, int Nd2, UniaxialMaterial &theMat,double a)
  :TrussBase(ELE_TAG_Truss,tag,dim,Nd1,Nd2), theMaterial(nullptr),A(a),theLoadSens(nullptr)
  {
    set_material(theMat);
    inicializa();
  }

//! @brief constructor:
//!  responsible for allocating the necessary space needed by each object
//!  and storing the tags of the truss end nodes.
XC::Truss::Truss(int tag,int dim,const Material *ptr_mat)
  :TrussBase(ELE_TAG_Truss,tag,dim), theMaterial(nullptr), A(0.0),theLoadSens(nullptr)
  {
    UniaxialMaterial *tmp= cast_material<UniaxialMaterial>(ptr_mat);
    if(tmp)
      set_material(*tmp);
    else
      std::cerr << "Truss::Truss; el material no es del tipo adecuado." << std::endl;
    inicializa();
  }


//! constructor:
//!   invoked by a FEM_ObjectBroker - blank object that recvSelf needs
//!   to be invoked upon
XC::Truss::Truss(void)
  :TrussBase(ELE_TAG_Truss), theMaterial(nullptr), A(0.0),theLoadSens(nullptr)
  { inicializa(); }

//! @brief Constructor de copia.
XC::Truss::Truss(const Truss &otro)
  :TrussBase(otro), theMaterial(nullptr), A(otro.A),theLoadSens(nullptr)
  {
    if(otro.theMaterial)
      set_material(*otro.theMaterial);
    if(otro.theLoadSens)
      set_load_sens(*otro.theLoadSens);
  }

//! @brief Operador asignación.
XC::Truss &XC::Truss::operator=(const Truss &otro)
  {
    TrussBase::operator=(otro);
    if(otro.theMaterial)
      set_material(*otro.theMaterial);
    A= otro.A;
    if(otro.theLoadSens)
      set_load_sens(*otro.theLoadSens);
    return *this;
  }


//! @brief Constructor virtual.
XC::Element* XC::Truss::getCopy(void) const
  { return new Truss(*this); }

//! @brief Lee un objeto Truss desde archivo
bool XC::Truss::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(Truss) Procesando comando: " << cmd << std::endl;
    if(cmd == "A")
      {
        A= interpretaDouble(status.GetString());
        return true;
      }
    else
      return TrussBase::procesa_comando(status);
  }

//!  destructor
//!     delete must be invoked on any objects created by the object
//!     and on the matertial object.
XC::Truss::~Truss(void)
  { libera(); }

//! method: setDomain()
//!    to set a link to the enclosing XC::Domain and to set the node pointers.
//!    also determines the number of dof associated
//!    with the truss element, we set matrix and vector pointers,
//!    allocate space for t matrix, determine the length
//!    and set the transformation matrix.
void XC::Truss::setDomain(Domain *theDomain)
  {
    TrussBase::setDomain(theDomain);
    if(!theDomain)
      {
        L= 0;
        return;
      }

    // if can't find both - send a warning message
    if((theNodes[0] == 0) || (theNodes[1] == 0))
      {
        // fill this in so don't segment fault later
        numDOF = 2;
        theMatrix = &trussM2;
        theVector = &trussV2;
        return;
      }

    // now determine the number of dof and the dimesnion
    int dofNd1 = theNodes[0]->getNumberDOF();
    int dofNd2 = theNodes[1]->getNumberDOF();

    // if differing dof at the ends - print a warning message
    if(dofNd1 != dofNd2)
      {
        std::cerr <<"WARNING XC::Truss::setDomain(): nodes " << theNodes[0]->getTag()
                  << " and " <<  theNodes[1]->getTag()
                  << "have differing dof at ends for truss " << this->getTag() << std::endl;

        // fill this in so don't segment fault later
        numDOF = 2;
        theMatrix = &trussM2;
        theVector = &trussV2;
        return;
      }

    setup_matrix_vector_ptrs(dofNd1);

    alloc_load(numDOF);

    // now determine the length, cosines and fill in the transformation
    setup_L_cos_dir();
    CableMaterial *ptrCableMaterial= dynamic_cast<CableMaterial *>(theMaterial);
    if(ptrCableMaterial)
      ptrCableMaterial->setLength(L);
  }

//! @brief Consuma el estado del elemento.
int XC::Truss::commitState()
  {
    int retVal = 0;
    // call element commitState to do any base class stuff
    if((retVal = this->XC::TrussBase::commitState()) != 0)
      { std::cerr << "XC::Truss::commitState () - failed in base class"; }
    retVal = theMaterial->commitState();
    return retVal;
  }

//! @brief Devuelve el estado del elemento al último consumado.
int XC::Truss::revertToLastCommit()
  { return theMaterial->revertToLastCommit(); }

//! @brief Devuelve el estado del elemento al inicial.
int XC::Truss::revertToStart()
  { return theMaterial->revertToStart(); }

//! @brief Calcula la deformación actual a partir de los desplazamientos de prueba en los nodos.
int XC::Truss::update(void)
  {
    double strain = this->computeCurrentStrain();
    double rate = this->computeCurrentStrainRate();
    return theMaterial->setTrialStrain(strain, rate);
  }

//! @brief Devuelve la matriz de rigidez tangente.
const XC::Matrix &XC::Truss::getTangentStiff(void) const
  {
    if(L == 0.0)
      { // - problem in setDomain() no further warnings
        theMatrix->Zero();
        return *theMatrix;
      }

    double E = theMaterial->getTangent();

    // come back later and redo this if too slow
    Matrix &stiff= *theMatrix;

    int numDOF2 = numDOF/2;
    double temp;
    double EAoverL = E*A/L;
    for(int i = 0; i < getNumDIM(); i++)
      {
        for(int j = 0; j < getNumDIM(); j++)
          {
            temp = cosX[i]*cosX[j]*EAoverL;
            stiff(i,j) = temp;
            stiff(i+numDOF2,j) = -temp;
            stiff(i,j+numDOF2) = -temp;
            stiff(i+numDOF2,j+numDOF2) = temp;
          }
      }
    if(isDead())
      stiff*=dead_srf;
    return stiff;
  }

//! @brief Devuelve la matriz de rigidez inicial.
const XC::Matrix &XC::Truss::getInitialStiff(void) const
  {
    if(L == 0.0)
      { // - problem in setDomain() no further warnings
        theMatrix->Zero();
        return *theMatrix;
      }

    const double E = theMaterial->getInitialTangent();

    // come back later and redo this if too slow
    Matrix &stiff = *theMatrix;

    int numDOF2 = numDOF/2;
    double temp;
    double EAoverL = E*A/L;
    for(int i = 0; i < getNumDIM(); i++) {
      for(int j = 0; j < getNumDIM(); j++) {
        temp = cosX[i]*cosX[j]*EAoverL;
        stiff(i,j) = temp;
        stiff(i+numDOF2,j) = -temp;
        stiff(i,j+numDOF2) = -temp;
        stiff(i+numDOF2,j+numDOF2) = temp;
      }
    }

    if(isDead())
      (*theMatrix)*=dead_srf;
    return *theMatrix;
  }

//! @brief Devuelve la matriz de amortiguamiento.
const XC::Matrix &XC::Truss::getDamp(void) const
  {
    if(L == 0.0) { // - problem in setDomain() no further warnings
        theMatrix->Zero();
        return *theMatrix;
    }

    double eta = theMaterial->getDampTangent();

    // come back later and redo this if too slow
    Matrix &damp = *theMatrix;

    int numDOF2 = numDOF/2;
    double temp;
    double etaAoverL = eta*A/L;
    for(int i = 0; i < getNumDIM(); i++)
      {
        for(int j = 0; j < getNumDIM(); j++)
          {
            temp = cosX[i]*cosX[j]*etaAoverL;
            damp(i,j) = temp;
            damp(i+numDOF2,j) = -temp;
            damp(i,j+numDOF2) = -temp;
            damp(i+numDOF2,j+numDOF2) = temp;
          }
      }
    if(isDead())
      damp*=dead_srf;
    return damp;
  }

const XC::Material *XC::Truss::getMaterial(void) const
  { return theMaterial; }
XC::Material *XC::Truss::getMaterial(void)
  { return theMaterial; }
//! @brief Devuelve la densidad del material.
double XC::Truss::getRho(void) const
  { return theMaterial->getRho(); }

//! @brief Devuelve la matriz de masas.
const XC::Matrix &XC::Truss::getMass(void) const
  {
    // zero the matrix
    Matrix &mass= *theMatrix;
    mass.Zero();

    const double rho= getRho();
    // check for XC::quick return
    if(L == 0.0 || rho == 0.0)
      { // - problem in setDomain() no further warnings
        return mass;
      }

    double M = 0.5*rho*L;
    int numDOF2 = numDOF/2;
    for(int i = 0; i < getNumDIM(); i++)
      {
        mass(i,i) = M;
        mass(i+numDOF2,i+numDOF2) = M;
      }
    if(isDead())
      mass*=dead_srf;
    return mass;
  }

//! @brief Añade una carga.
int XC::Truss::addLoad(ElementalLoad *theLoad, double loadFactor)
  {
    if(isDead())
      std::cerr << nombre_clase() 
                << "; se intentó cargar el elemento "
                << getTag() << " que está desactivado." 
                << std::endl;
    else
      {
        if(TrussStrainLoad *trsLoad= dynamic_cast<TrussStrainLoad *>(theLoad))
          {
            const double &e1= trsLoad->E1()*loadFactor;
            const double &e2= trsLoad->E2()*loadFactor;
            double ezero= theMaterial->getInitialStrain();
            ezero+= (e2+e1)/2;
            theMaterial->setInitialStrain(ezero);
          }
        else
          {
            std::cerr <<"XC::Truss::addLoad - load type unknown for truss with tag: " << this->getTag() << std::endl;
            return -1;
          }
      }
    return 0;
  }

//! @brief Añade las fuerzas de inercia.
int XC::Truss::addInertiaLoadToUnbalance(const XC::Vector &accel)
  {
    const double rho= getRho();
    // check for a quick return
    if(L == 0.0 || rho == 0.0)
        return 0;

    // get R * accel from the nodes
    const Vector &Raccel1 = theNodes[0]->getRV(accel);
    const Vector &Raccel2 = theNodes[1]->getRV(accel);

    int nodalDOF = numDOF/2;

    const double M = 0.5*rho*L;
    // want to add ( - fact * M R * accel ) to unbalance
    for(int i=0; i<getNumDIM(); i++)
      {
        double val1 = Raccel1(i);
        double val2 = Raccel2(i);

        // perform - fact * M*(R * accel) // remember M a diagonal matrix
        val1*= -M;
        val2*= -M;

        (*getLoad())(i) += val1;
        (*getLoad())(i+nodalDOF) += val2;
      }

    return 0;
  }


int XC::Truss::addInertiaLoadSensitivityToUnbalance(const XC::Vector &accel, bool somethingRandomInMotions)
  {
    if(!theLoadSens)
      { set_load_sens(Vector(numDOF)); }
    else
      { theLoadSens->Zero(); }
    const double rho= getRho();

  if(somethingRandomInMotions)
    {

    // check for a XC::quick return
    if(L == 0.0 || rho == 0.0)
      return 0;

    // get R * accel from the nodes
    const XC::Vector &Raccel1 = theNodes[0]->getRV(accel);
    const XC::Vector &Raccel2 = theNodes[1]->getRV(accel);

    int nodalDOF = numDOF/2;

#ifdef _G3DEBUG
    if(nodalDOF != Raccel1.Size() || nodalDOF != Raccel2.Size()) {
      std::cerr << "XC::Truss::addInertiaLoadToUnbalance " <<
        "matrix and vector sizes are incompatable\n";
      return -1;
    }
#endif

        double M  = 0.5*rho*L;
    // want to add ( - fact * M R * accel ) to unbalance
    for(int i=0; i<getNumDIM(); i++) {
      double val1 = Raccel1(i);
      double val2 = Raccel2(i);

      // perform - fact * M*(R * accel) // remember M a diagonal matrix
      val1 *= M;
      val2 *= M;

      (*theLoadSens)(i) = val1;
      (*theLoadSens)(i+nodalDOF) = val2;
    }
  }
  else {

    // check for a XC::quick return
    if(L == 0.0 || rho == 0.0)
      return 0;

    // get R * accel from the nodes
    const XC::Vector &Raccel1 = theNodes[0]->getRV(accel);
    const XC::Vector &Raccel2 = theNodes[1]->getRV(accel);

    int nodalDOF = numDOF/2;

#ifdef _G3DEBUG
    if(nodalDOF != Raccel1.Size() || nodalDOF != Raccel2.Size()) {
      std::cerr << "XC::Truss::addInertiaLoadToUnbalance " <<
        "matrix and vector sizes are incompatable\n";
      return -1;
    }
#endif

    double massDerivative = 0.0;
    if(parameterID == 2) {
      massDerivative = 0.5*L;
    }

    // want to add ( - fact * M R * accel ) to unbalance
    for(int i=0; i<getNumDIM(); i++) {
      double val1 = Raccel1(i);
      double val2 = Raccel2(i);

      // perform - fact * M*(R * accel) // remember M a diagonal matrix

      val1 *= massDerivative;
      val2 *= massDerivative;

      (*theLoadSens)(i) = val1;
      (*theLoadSens)(i+nodalDOF) = val2;
    }
  }
  return 0;
}

//! @brief Devuelve la reacción del elemento.
const XC::Vector &XC::Truss::getResistingForce(void) const
  {
    if(L == 0.0)
      { // - problem in setDomain() no further warnings
        theVector->Zero();
        return *theVector;
      }

    // R = Ku - Pext
    // Ku = F * transformation
    const double force = A*theMaterial->getStress();
    int numDOF2 = numDOF/2;
    double temp;
    for(int i = 0; i < getNumDIM(); i++)
      {
        temp = cosX[i]*force;
        (*theVector)(i) = -temp;
        (*theVector)(i+numDOF2) = temp;
      }

    // subtract external load:  Ku - P
    (*theVector)-= *getLoad();

    if(isDead())
      (*theVector)*=dead_srf;
    return *theVector;
  }

//! @brief Devuelve la reacción del elemento incluyendo fuerzas de inercia.
const XC::Vector &XC::Truss::getResistingForceIncInertia(void) const
  {
    this->getResistingForce();

    const double rho= getRho();
    // now include the mass portion
    if(L != 0.0 && rho != 0.0)
      {

        const XC::Vector &accel1 = theNodes[0]->getTrialAccel();
        const XC::Vector &accel2 = theNodes[1]->getTrialAccel();

        int numDOF2 = numDOF/2;
        double M = 0.5*rho*L;
        for(int i = 0; i < getNumDIM(); i++)
          {
            (*theVector)(i) += M*accel1(i);
            (*theVector)(i+numDOF2) += M*accel2(i);
          }

        // add the damping forces if rayleigh damping
        if(!rayFactors.Nulos())
          (*theVector) += this->getRayleighDampingForces();
      }
    else
      {
        // add the damping forces if rayleigh damping
        if(!rayFactors.KNulos())
          (*theVector) += this->getRayleighDampingForces();
      }
    if(isDead())
      (*theVector)*=dead_srf; //XXX Se aplica 2 veces sobre getResistingForce: arreglar.
    return *theVector;
  }

//! @brief Devuelve un vector para almacenar los dbTags
//! de los miembros de la clase.
XC::DbTagData &XC::Truss::getDbTagData(void) const
  {
    static DbTagData retval(28);
    return retval;
  }

//! @brief Envía los miembros por el canal que se pasa como parámetro.
int XC::Truss::sendData(CommParameters &cp)
  {
    int res= TrussBase::sendData(cp);
    res+= cp.sendBrokedPtr(theMaterial,getDbTagData(),BrokedPtrCommMetaData(20,21,22));
    res+= cp.sendDouble(A,getDbTagData(),CommMetaData(23));
    res+= cp.sendInt(parameterID,getDbTagData(),CommMetaData(24));
    res+= cp.sendVectorPtr(theLoadSens,getDbTagData(),ArrayCommMetaData(25,26,27)); 
    return res;
  }

//! @brief Recibe los miembros por el canal que se pasa como parámetro.
int XC::Truss::recvData(const CommParameters &cp)
  {
    int res= TrussBase::recvData(cp);
    theMaterial= cp.getBrokedMaterial(theMaterial,getDbTagData(),BrokedPtrCommMetaData(20,21,22));
    res+= cp.receiveDouble(A,getDbTagData(),CommMetaData(23));
    res+= cp.receiveInt(parameterID,getDbTagData(),CommMetaData(24));
    theLoadSens= cp.receiveVectorPtr(theLoadSens,getDbTagData(),ArrayCommMetaData(25,26,27));
    return res;
  }

//! @brief Envía el objeto por el canal que se pasa como parámetro.
int XC::Truss::sendSelf(CommParameters &cp)
  {
    setDbTag(cp);
    const int dataTag= getDbTag();
    inicComm(28);
    int res= sendData(cp);

    res+= cp.sendIdData(getDbTagData(),dataTag);
    if(res < 0)
      std::cerr << "Truss::sendSelf - failed to send data.\n";
    return res;
  }

//! @brief Recibe el objeto por el canal que se pasa como parámetro.
int XC::Truss::recvSelf(const CommParameters &cp)
  {
    inicComm(28);
    
    const int dbTag= getDbTag();
    int res= cp.receiveIdData(getDbTagData(),dbTag);
    if(res<0)
      std::cerr << "Truss::recvSelf - failed to receive ids.\n";
    else
      {
        res+= recvData(cp);
        if(res<0)
           std::cerr << "Truss::recvSelf - failed to receive data.\n";
      }
    return res;
  }

void XC::Truss::Print(std::ostream &s, int flag)
  {
    // compute the strain and axial force in the member
    const double strain = theMaterial->getStrain();
    const double force = A * theMaterial->getStress();

    if(flag == 0)
      { // print everything
        s << "Element: " << this->getTag();
        s << " type: Truss  iNode: " << theNodes.getTagNode(0);
        s << " jNode: " << theNodes.getTagNode(1);

        s << " \n\t strain: " << strain;
        s << " axial load: " <<  force;
        if(L != 0.0)
          {
            int numDOF2 = numDOF/2;
            double temp;
            for(int i = 0; i < getNumDIM(); i++)
              {
                temp = cosX[i]*force;
                (*theVector)(i) = -temp;
                (*theVector)(i+numDOF2) = temp;
              }
            s << " \n\t unbalanced load: " << *theVector;
          }
        s << " \t XC::Material: " << *theMaterial;
        s << std::endl;
      }
    else
     if(flag == 1)
       {
         s << this->getTag() << "  " << strain << "  ";
         s << force << std::endl;
       }
  }

double XC::Truss::getAxil(void) const
  { return A*theMaterial->getStress(); }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
//!
//! Soporta los códigos:
//! strain: Devuelve la deformación del elemento.
any_const_ptr XC::Truss::GetProp(const std::string &cod) const
  {
    if(cod=="getStrain")
      {
        tmp_gp_dbl= theMaterial->getStrain();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getStress")
      {
        tmp_gp_dbl= theMaterial->getStress();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getAxil")
      {
        tmp_gp_dbl= getAxil();
        return any_const_ptr(tmp_gp_dbl);
      }
    else if(cod=="getArea")
      return any_const_ptr(A);
    else
      return TrussBase::GetProp(cod);
  }

double XC::Truss::computeCurrentStrain(void) const
  {
    // NOTE method will not be called if L == 0

    // determine the strain
    const XC::Vector &disp1 = theNodes[0]->getTrialDisp();
    const XC::Vector &disp2 = theNodes[1]->getTrialDisp();

    double dLength = 0.0;
    for(int i = 0; i < getNumDIM(); i++)
      dLength += (disp2(i)-disp1(i))*cosX[i];

    // this method should never be called with L == 0
    return dLength/L;
  }

double XC::Truss::computeCurrentStrainRate(void) const
  {
    // NOTE method will not be called if L == 0

    // determine the strain
    const XC::Vector &vel1 = theNodes[0]->getTrialVel();
    const XC::Vector &vel2 = theNodes[1]->getTrialVel();

    double dLength = 0.0;
    for(int i = 0; i < getNumDIM(); i++)
      {
        dLength += (vel2(i)-vel1(i))*cosX[i];
      }

    // this method should never be called with L == 0
    return dLength/L;
  }

XC::Response *XC::Truss::setResponse(const std::vector<std::string> &argv, Information &eleInfo)
  {
    //
    // we compare argv[0] for known response types for the XC::Truss
    //

    if(argv[0] == "force" || argv[0] == "forces" || argv[0] == "axialForce")
      return new ElementResponse(this, 1, 0.0);

    else if(argv[0] == "defo" || argv[0] == "deformations" ||
           argv[0] == "deformation")
      return new ElementResponse(this, 2, 0.0);
    // tangent stiffness matrix
    else if(argv[0] == "stiff")
      return new ElementResponse(this, 3, *theMatrix);
    // a material quantity
    else if(argv[0] == "material" || argv[0] == "-material")
      return  setMaterialResponse(theMaterial,argv,1,eleInfo);
    else
      return 0;
  }

int XC::Truss::getResponse(int responseID, Information &eleInfo)
  {
    switch (responseID)
      {
      case 1:
        return eleInfo.setDouble(A * theMaterial->getStress());
      case 2:
        return eleInfo.setDouble(L * theMaterial->getStrain());
      case 3:
        return eleInfo.setMatrix(this->getTangentStiff());
      default:
        return 0;
      }
  }

// AddingSensitivity:BEGIN ///////////////////////////////////
int XC::Truss::setParameter(const std::vector<std::string> &argv, Parameter &param)
  {
    if(argv.size() < 1)
        return -1;

    // Cross sectional area of the truss itself
    if(argv[0] == "A")
      return param.addObject(1, this);
    // Mass densitity (per unit volume) of the truss itself
    else if(argv[0] == "rho")
      return param.addObject(1, this);
    // a material parameter
    else if(argv[0] == "-material" || argv[0] == "material")
      {
        int ok =  setMaterialParameter(theMaterial,argv,1,param);
        if(ok < 0)
          return -1;
        else
         return ok + 100;
      }
    // otherwise parameter is unknown for the XC::Truss class
    else
      return -1;
  }

int XC::Truss::updateParameter(int parameterID, Information &info)
  {
    switch (parameterID)
      {
      case -1:
        return -1;
      case 1:
        this->A = info.theDouble;
        return 0;
//       case 2: //El parámetro rho es del material.
//         this->rho= info.theDouble;
//         return 0;
      default:
        if(parameterID >= 100)
          return theMaterial->updateParameter(parameterID-100, info);
        else
          return -1;
      }
  }
int XC::Truss::activateParameter(int passedParameterID)
  {
    parameterID = passedParameterID;

    // The identifier needs to be passed "downwards" also when it's zero
    if(passedParameterID == 0 || passedParameterID == 1 || passedParameterID == 2)
      { theMaterial->activateParameter(0); }
    // If the identifier is non-zero and the parameter belongs to the material
    else if( passedParameterID > 100)
      { theMaterial->activateParameter(passedParameterID-100); }
    return 0;
  }


const XC::Matrix &XC::Truss::getKiSensitivity(int gradNumber)
  {
    Matrix &stiff = *theMatrix;
    stiff.Zero();

    if(parameterID == 0)
      {}
    else if(parameterID == 1)
      {
    // If cross sectional area is random
    double E = theMaterial->getInitialTangent();

    int numDOF2 = numDOF/2;
    double temp;
    double EoverL = E/L;
    for(int i = 0; i < getNumDIM(); i++) {
      for(int j = 0; j < getNumDIM(); j++) {
        temp = cosX[i]*cosX[j]*EoverL;
        stiff(i,j) = temp;
        stiff(i+numDOF2,j) = -temp;
        stiff(i,j+numDOF2) = -temp;
        stiff(i+numDOF2,j+numDOF2) = temp;
      }
    }
  }
  else if(parameterID == 2)
    {
      // Nothing here when 'rho' is random
    }
  else
    {
      double Esens = theMaterial->getInitialTangentSensitivity(gradNumber);

      int numDOF2 = numDOF/2;
      double temp;
      double EAoverL = Esens*A/L;
      for(int i = 0; i < getNumDIM(); i++)
        {
          for(int j = 0; j < getNumDIM(); j++)
            {
              temp = cosX[i]*cosX[j]*EAoverL;
              stiff(i,j) = temp;
              stiff(i+numDOF2,j) = -temp;
              stiff(i,j+numDOF2) = -temp;
              stiff(i+numDOF2,j+numDOF2) = temp;
            }
        }
    }
  return stiff;
  }

const XC::Matrix &XC::Truss::getMassSensitivity(int gradNumber)
  {
    Matrix &mass = *theMatrix;
    mass.Zero();

    if(parameterID == 2)
      {
        double massDerivative = 0.5*L;

        int numDOF2 = numDOF/2;
        for(int i = 0; i < getNumDIM(); i++)
          {
            mass(i,i) = massDerivative;
            mass(i+numDOF2,i+numDOF2) = massDerivative;
          }
      }
    return mass;
  }

const XC::Vector &XC::Truss::getResistingForceSensitivity(int gradNumber)
  {
    theVector->Zero();

    // Initial declarations
    int i;
    double stressSensitivity, temp1, temp2;

    // Make sure the material is up to date
    double strain = this->computeCurrentStrain();
    double rate = this->computeCurrentStrainRate();
    theMaterial->setTrialStrain(strain,rate);

    // Contribution from material
    stressSensitivity = theMaterial->getStressSensitivity(gradNumber,true);

    // Check if a nodal coordinate is random
    double dcosXdh[3];
    dcosXdh[0] = 0.0;
    dcosXdh[1] = 0.0;
    dcosXdh[2] = 0.0;

    int nodeParameterID0 = theNodes[0]->getCrdsSensitivity();
    int nodeParameterID1 = theNodes[1]->getCrdsSensitivity();
    if(nodeParameterID0 != 0 || nodeParameterID1 != 0) {

      double dx = L*cosX[0];
      double dy = L*cosX[1];
      //double dz = L*cosX[2];

        // Compute derivative of transformation matrix (assume 4 dofs)
        if(nodeParameterID0 == 1) { // here x1 is random
            temp1 = (-L+dx*dx/L)/(L*L);
            temp2 = dx*dy/(L*L*L);
            //dtdh(0) = -temp1;
            //dtdh(1) = -temp2;
            //dtdh(2) = temp1;
            //dtdh(3) = temp2;
            dcosXdh[0] = temp1;
            dcosXdh[1] = temp2;
            dcosXdh[2] = 0.0;
        }
        if(nodeParameterID0 == 2) { // here y1 is random
            temp1 = (-L+dy*dy/L)/(L*L);
            temp2 = dx*dy/(L*L*L);
            //dtdh(0) = -temp2;
            //dtdh(1) = -temp1;
            //dtdh(2) = temp2;
            //dtdh(3) = temp1;
            dcosXdh[0] = temp2;
            dcosXdh[1] = temp1;
            dcosXdh[2] = 0.0;
        }
        if(nodeParameterID1 == 1) { // here x2 is random
            temp1 = (L-dx*dx/L)/(L*L);
            temp2 = -dx*dy/(L*L*L);
            //dtdh(0) = -temp1;
            //dtdh(1) = -temp2;
            //dtdh(2) = temp1;
            //dtdh(3) = temp2;
            dcosXdh[0] = temp1;
            dcosXdh[1] = temp2;
            dcosXdh[2] = 0.0;
        }
        if(nodeParameterID1 == 2) { // here y2 is random
            temp1 = (L-dy*dy/L)/(L*L);
            temp2 = -dx*dy/(L*L*L);
            //dtdh(0) = -temp2;
            //dtdh(1) = -temp1;
            //dtdh(2) = temp2;
            //dtdh(3) = temp1;
            dcosXdh[0] = temp2;
            dcosXdh[1] = temp1;
            dcosXdh[2] = 0.0;
        }

        const XC::Vector &disp1 = theNodes[0]->getTrialDisp();
        const XC::Vector &disp2 = theNodes[1]->getTrialDisp();
        double dLengthDerivative = 0.0;
        for(i = 0; i < getNumDIM(); i++) {
            dLengthDerivative += (disp2(i)-disp1(i))*dcosXdh[i];
        }

        double materialTangent = theMaterial->getTangent();
        double strainSensitivity= 0.0;

        if(nodeParameterID0 == 1) {        // here x1 is random
            strainSensitivity = (dLengthDerivative*L+strain*dx)/(L*L);
        }
        if(nodeParameterID0 == 2) {    // here y1 is random
            strainSensitivity = (dLengthDerivative*L+strain*dy)/(L*L);
        }
        if(nodeParameterID1 == 1) {        // here x2 is random
            strainSensitivity = (dLengthDerivative*L-strain*dx)/(L*L);
        }
        if(nodeParameterID1 == 2) {    // here y2 is random
            strainSensitivity = (dLengthDerivative*L-strain*dy)/(L*L);
        }
        stressSensitivity += materialTangent * strainSensitivity;
    }


    // Compute sensitivity depending on 'parameter'
    double stress = theMaterial->getStress();
    int numDOF2 = numDOF/2;
    double temp;
    if(parameterID == 1) {            // Cross-sectional area
      for(i = 0; i < getNumDIM(); i++) {
        temp = (stress + A*stressSensitivity)*cosX[i];
        (*theVector)(i) = -temp;
        (*theVector)(i+numDOF2) = temp;
      }
    }
    else {        // Density, material parameter or nodal coordinate
      for(i = 0; i < getNumDIM(); i++) {
        temp = A*(stressSensitivity*cosX[i] + stress*dcosXdh[i]);
        (*theVector)(i) = -temp;
        (*theVector)(i+numDOF2) = temp;
      }
    }

    // subtract external load sensitivity
    if(!theLoadSens)
      set_load_sens(Vector(numDOF));

    (*theVector)-= *theLoadSens;

    return *theVector;
  }

int XC::Truss::commitSensitivity(int gradNumber, int numGrads)
  {
    // Initial declarations
    int i;
    double strainSensitivity, temp1, temp2;

    // Displacement difference between the two ends
    double strain = this->computeCurrentStrain();
    double dLength = strain*L;

    // Displacement sensitivity difference between the two ends
    double sens1;
    double sens2;
    double dSensitivity = 0.0;
    for(i=0; i<getNumDIM(); i++){
      sens1 = theNodes[0]->getDispSensitivity(i+1, gradNumber);
      sens2 = theNodes[1]->getDispSensitivity(i+1, gradNumber);
      dSensitivity += (sens2-sens1)*cosX[i];
    }

    strainSensitivity = dSensitivity/L;

    // Check if a nodal coordinate is random
    int nodeParameterID0 = theNodes[0]->getCrdsSensitivity();
    int nodeParameterID1 = theNodes[1]->getCrdsSensitivity();
    if(nodeParameterID0 != 0 || nodeParameterID1 != 0) {

      double dx = L*cosX[0];
      double dy = L*cosX[1];
      //double dz = L*cosX[2];

        // Compute derivative of transformation matrix (assume 4 dofs)
        double dcosXdh[3];

        if(nodeParameterID0 == 1) { // here x1 is random
            temp1 = (-L+dx*dx/L)/(L*L);
            temp2 = dx*dy/(L*L*L);
            //dtdh(0) = -temp1;
            //dtdh(1) = -temp2;
            //dtdh(2) = temp1;
            //dtdh(3) = temp2;
            dcosXdh[0] = temp1;
            dcosXdh[1] = temp2;
            dcosXdh[2] = 0.0;
        }
        if(nodeParameterID0 == 2) { // here y1 is random
            temp1 = (-L+dy*dy/L)/(L*L);
            temp2 = dx*dy/(L*L*L);
            //dtdh(0) = -temp2;
            //dtdh(1) = -temp1;
            //dtdh(2) = temp2;
            //dtdh(3) = temp1;
            dcosXdh[0] = temp2;
            dcosXdh[1] = temp1;
            dcosXdh[2] = 0.0;
        }

        if(nodeParameterID1 == 1) { // here x2 is random
            temp1 = (L-dx*dx/L)/(L*L);
            temp2 = -dx*dy/(L*L*L);
            //dtdh(0) = -temp1;
            //dtdh(1) = -temp2;
            //dtdh(2) = temp1;
            //dtdh(3) = temp2;
            dcosXdh[0] = temp1;
            dcosXdh[1] = temp2;
            dcosXdh[2] = 0.0;
        }
        if(nodeParameterID1 == 2) { // here y2 is random
            temp1 = (L-dy*dy/L)/(L*L);
            temp2 = -dx*dy/(L*L*L);
            //dtdh(0) = -temp2;
            //dtdh(1) = -temp1;
            //dtdh(2) = temp2;
            //dtdh(3) = temp1;
            dcosXdh[0] = temp2;
            dcosXdh[1] = temp1;
            dcosXdh[2] = 0.0;
        }

        const XC::Vector &disp1 = theNodes[0]->getTrialDisp();
        const XC::Vector &disp2 = theNodes[1]->getTrialDisp();
        double dLengthDerivative = 0.0;
        for(i = 0; i < getNumDIM(); i++){
            dLengthDerivative += (disp2(i)-disp1(i))*dcosXdh[i];
        }

        strainSensitivity += dLengthDerivative/L;

        if(nodeParameterID0 == 1) {        // here x1 is random
            strainSensitivity += dLength/(L*L*L)*dx;
        }
        if(nodeParameterID0 == 2) {    // here y1 is random
            strainSensitivity += dLength/(L*L*L)*dy;
        }
        if(nodeParameterID1 == 1) {        // here x2 is random
            strainSensitivity -= dLength/(L*L*L)*dx;
        }
        if(nodeParameterID1 == 2) {    // here y2 is random
            strainSensitivity -= dLength/(L*L*L)*dy;
        }
    }

    // Pass it down to the material
    theMaterial->commitSensitivity(strainSensitivity, gradNumber, numGrads);

   return 0;
  }

// AddingSensitivity:END /////////////////////////////////////////////