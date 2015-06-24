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
** file 'COPYRIGHT'  in main directory for information on usage and   **
** redistribution,  and for a DISCLAIMER OF ALL WARRANTIES.           **
**                                                                    **
** Developed by:                                                      **
**   Frank McKenna (fmckenna@ce.berkeley.edu)                         **
**   Gregory L. Fenves (fenves@ce.berkeley.edu)                       **
**   Filip C. Filippou (filippou@ce.berkeley.edu)                     **
**                                                                    **
** ****************************************************************** */

// $Revision: 1.13 $
// $Date: 2005/12/15 00:30:38 $
// $Source: /usr/local/cvs/OpenSees/SRC/LinearCrdTransf3d.cpp,v $


// Written: Remo Magalhaes de Souza (rmsouza@ce.berkeley.edu)
// Created: 04/2000
// Revision: A
//
// Modified: 04/2005 Andreas Schellenberg (getBasicTrialVel, getBasicTrialAccel)
//
// Purpose: This file contains the implementation for the
// LinearCrdTransf3d class. LinearCrdTransf3d is a linear
// transformation for a planar frame between the global
// and basic coordinate systems


#include "LinearCrdTransf3d.h"
#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>
#include <domain/mesh/node/Node.h>

//! @brief Constructor por defecto.
XC::LinearCrdTransf3d::LinearCrdTransf3d(int tag)
  : XC::SmallDispCrdTransf3d(tag, CRDTR_TAG_LinearCrdTransf3d) {}

//! @brief constructor
//! @param vecInLocXZPlane: Vector en el plano XZ local.
XC::LinearCrdTransf3d::LinearCrdTransf3d(int tag, const XC::Vector &vecInLocXZPlane)
  : XC::SmallDispCrdTransf3d(tag, CRDTR_TAG_LinearCrdTransf3d,vecInLocXZPlane) {}

//! @brief Constructor
XC::LinearCrdTransf3d::LinearCrdTransf3d(int tag, const Vector &vecInLocXZPlane,
                                     const Vector &rigJntOffset1, const Vector &rigJntOffset2)
  : XC::SmallDispCrdTransf3d(tag, CRDTR_TAG_LinearCrdTransf3d,vecInLocXZPlane)
  {
    // check rigid joint offset for node I
    set_rigid_joint_offsetI(rigJntOffset1);

    // check rigid joint offset for node J
    set_rigid_joint_offsetJ(rigJntOffset2);
  }


//! @brief Constructor; invoked by a FEM_ObjectBroker, recvSelf() needs to be invoked on this object.
XC::LinearCrdTransf3d::LinearCrdTransf3d(void)
  : XC::SmallDispCrdTransf3d(0, CRDTR_TAG_LinearCrdTransf3d) {}


//! @brief Consuma la transformación de coordenadas de acuerdo con el estado actual.
int XC::LinearCrdTransf3d::commitState(void)
  { return 0; }


//! @brief Devuelve el estado de la transformación al último consumado.
int XC::LinearCrdTransf3d::revertToLastCommit(void)
  { return 0; }

//! @brief Devuelve el estado de la transformación al inicial.
int XC::LinearCrdTransf3d::revertToStart(void)
  { return 0; }




//! @brief Actualiza la transformación.
int XC::LinearCrdTransf3d::update(void)
  { return 0; }

//! @brief Devuelve el vector de cargas expresado en el sistema global del elemento.
const XC::Vector &XC::LinearCrdTransf3d::getGlobalResistingForce(const Vector &pb, const Vector &p0) const
  {
    // transform resisting forces from the basic system to local coordinates
    const Vector &pl= basic_to_local_resisting_force(pb,p0);
    return local_to_global_resisting_force(pl);
  }

//! @brief Devuelve la matriz de rigidez expresada en el sistema global del elemento.
const XC::Matrix &XC::LinearCrdTransf3d::getGlobalStiffMatrix(const Matrix &KB, const Vector &pb) const
  {
    const Matrix &kl= basic_to_local_stiff_matrix(KB); // Local stiffness
    return local_to_global_stiff_matrix(kl);
  }

//! @brief Constructor virtual.
XC::CrdTransf3d *XC::LinearCrdTransf3d::getCopy(void) const
  { return new LinearCrdTransf3d(*this); }


const XC::Vector &XC::LinearCrdTransf3d::getPointGlobalCoordFromLocal(const Vector &xl) const
  {
    static Vector xg(3);

    //xg = nodeIPtr->getCrds() + nodeIOffset;
    xg = nodeIPtr->getCrds();

    xg(0)+= nodeIOffset(0);
    xg(1)+= nodeIOffset(1);
    xg(2)+= nodeIOffset(2);

    if(!nodeIInitialDisp.empty())
      {
        xg(0)-= nodeIInitialDisp[0];
        xg(1)-= nodeIInitialDisp[1];
        xg(2)-= nodeIInitialDisp[2];
      }

    // xg = xg + Rlj'*xl
    //xg.addMatrixTransposeVector(1.0, Rlj, xl, 1.0);
    xg(0)+= R(0,0)*xl(0) + R(1,0)*xl(1) + R(2,0)*xl(2);
    xg(1)+= R(0,1)*xl(0) + R(1,1)*xl(1) + R(2,1)*xl(2);
    xg(2)+= R(0,2)*xl(0) + R(1,2)*xl(1) + R(2,2)*xl(2);

    return xg;
  }

const XC::Vector &XC::LinearCrdTransf3d::getPointGlobalDisplFromBasic(double xi, const Vector &uxb) const
  {
    // determine global displacements
    const Vector &disp1 = nodeIPtr->getTrialDisp();
    const Vector &disp2 = nodeJPtr->getTrialDisp();

    static double ug[12];
    inic_ug(disp1,disp2,ug);
    modif_ug_init_disp(ug);

    // transform global end displacements to local coordinates
    //ul.addMatrixVector(0.0, Tlg,  ug, 1.0);       //  ul = Tlg *  ug;
    static double ul[12];

    ul[0]  = R(0,0)*ug[0] + R(0,1)*ug[1] + R(0,2)*ug[2];
    ul[1]  = R(1,0)*ug[0] + R(1,1)*ug[1] + R(1,2)*ug[2];
    ul[2]  = R(2,0)*ug[0] + R(2,1)*ug[1] + R(2,2)*ug[2];

    ul[7]  = R(1,0)*ug[6] + R(1,1)*ug[7] + R(1,2)*ug[8];
    ul[8]  = R(2,0)*ug[6] + R(2,1)*ug[7] + R(2,2)*ug[8];

    static double Wu[3];
    calc_Wu(ug,ul,Wu);

    // compute displacements at point xi, in local coordinates
    static double uxl[3];
    static Vector uxg(3);

    uxl[0] = uxb(0) +        ul[0];
    uxl[1] = uxb(1) + (1-xi)*ul[1] + xi*ul[7];
    uxl[2] = uxb(2) + (1-xi)*ul[2] + xi*ul[8];

    // rotate displacements to global coordinates
    // uxg = Rlj'*uxl
    //uxg.addMatrixTransposeVector(0.0, Rlj, uxl, 1.0);
    uxg(0) = R(0,0)*uxl[0] + R(1,0)*uxl[1] + R(2,0)*uxl[2];
    uxg(1) = R(0,1)*uxl[0] + R(1,1)*uxl[1] + R(2,1)*uxl[2];
    uxg(2) = R(0,2)*uxl[0] + R(1,2)*uxl[1] + R(2,2)*uxl[2];

    return uxg;
  }


void XC::LinearCrdTransf3d::Print(std::ostream &s, int flag)
  {
    s << "\nCrdTransf: " << this->getTag() << " Type: LinearCrdTransf3d";
    s << "\tNode I offset: " << nodeIOffset(0) << " " << nodeIOffset(1) << " "<< nodeIOffset(2) << std::endl;
    s << "\tNode J offset: " << nodeJOffset(0) << " " << nodeJOffset(1) << " "<< nodeJOffset(2) << std::endl;
  }