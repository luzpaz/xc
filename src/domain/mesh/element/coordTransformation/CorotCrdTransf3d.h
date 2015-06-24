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
                                                                        
// $Revision: 1.9 $                                                              
// $Date: 2006/01/13 01:07:48 $                                                                  
// $Source: /usr/local/cvs/OpenSees/SRC/CorotCrdTransf3d.h,v $      


// Written: Remo Magalhaes de Souza (rmsouza@ce.berkeley.edu)
// Created: 04/2000
// Revision: A
//
// Description: This file contains the class definition for
// CorotCrdTransf3d.h. CorotCrdTransf3d provides the
// abstraction of a corotation transformation for a spatial frame element

// What: "@(#) CorotCrdTransf3d.h, revA"

#ifndef CorotCrdTransf3d_h
#define CorotCrdTransf3d_h

#include "CrdTransf3d.h"
#include <utility/matrix/Vector.h>
#include <utility/matrix/Matrix.h>

namespace XC {
//! \ingroup ElemCT
//
//! @brief Transformación de coordenadas corrotacional en 3d.
class CorotCrdTransf3d: public CrdTransf3d
  {
  private:
    Vector vAxis; //!< Vector that lies in local plane xz
    
    mutable Vector xAxis; //!< local x axis
    mutable double Ln; //!< deformed element length
    
    mutable Vector alphaIq; //!< quaternion for node I
    mutable Vector alphaJq; //!< quaternion for node J
    
    Vector alphaIqcommit; //!< commited quaternion for node I
    Vector alphaJqcommit; //!< commited quaternion for node J
    mutable Vector alphaI; //!< last trial rotations end i
    mutable Vector alphaJ; //!< last trial rotatations end j
    
    Vector ul; //!< local displacements
    Vector ulcommit; //!< commited local displacements
    Vector ulpr; //!< previous local displacements
    
    static Matrix RI; //!< nodal triad for node 1
    static Matrix RJ; //!< nodal triad for node 2
    static Matrix Rbar; //!< mean nodal triad 
    static Matrix e; //!< base vectors
    static Matrix Tp; //!< transformation matrix to renumber dofs
    static Matrix T; //!< transformation matrix from basic to global system
    static Matrix Lr2, Lr3, A; //!< auxiliary matrices	

    inline int computeElemtLengthAndOrient(void) const
      {
        std::cerr << "CorotCrdTransf3d::computeElemtLengthAndOrient; no implementada."
                  << std::endl;
	return 0;
      }
    void compTransfMatrixBasicGlobal(void);
    void compTransfMatrixBasicGlobalNew(void);
    const Vector &getQuaternionFromRotMatrix(const Matrix &RotMatrix) const;
    const Vector &getQuaternionFromPseudoRotVector(const Vector &theta) const;
    const Vector &getTangScaledPseudoVectorFromQuaternion(const Vector &theta) const;
    const Vector &quaternionProduct(const Vector &q1, const Vector &q2) const;
    const Matrix &getRotationMatrixFromQuaternion(const Vector &q) const;
    const Matrix &getRotMatrixFromTangScaledPseudoVector(const Vector &w) const;
    const Matrix &getSkewSymMatrix(const Vector &theta) const;
    const Matrix &getLMatrix(const Vector &ri) const;
    const Matrix &getKs2Matrix(const Vector &ri, const Vector &z) const;
    

    
    Vector &basic_to_local_element_force(const Vector &p0) const;
    const Vector &local_to_global_element_force(const Vector &) const;
  protected:    
    int sendData(CommParameters &);
    int recvData(const CommParameters &);
    virtual int calculaEjesLocales(void) const;
  public:
    CorotCrdTransf3d(int tag= 0);
    CorotCrdTransf3d(int tag, const Vector &vecInLocXZPlane,const Vector &rigJntOffsetI, const Vector &rigJntOffsetJ);
    
    int initialize(Node *nodeIPointer, Node *nodeJPointer);
    int update(void);
    double getInitialLength(void) const;
    double getDeformedLength(void) const;
    
    virtual void set_xz_vector(const Vector &vecInLocXZPlane);

    int commitState(void);
    int revertToLastCommit(void);        
    int revertToStart(void);
    
    const Vector &getBasicTrialDisp(void) const;
    const Vector &getBasicIncrDisp(void) const;
    const Vector &getBasicIncrDeltaDisp(void) const;
    const Vector &getBasicTrialVel(void) const;
    const Vector &getBasicTrialAccel(void) const;
    
    const Vector &getGlobalResistingForce(const Vector &basicForce, const Vector &uniformLoad) const;
    const Matrix &getGlobalStiffMatrix(const Matrix &basicStiff, const Vector &basicForce) const;
    const Matrix &getInitialGlobalStiffMatrix(const Matrix &basicStiff) const;
    
    CrdTransf3d *getCopy(void) const;
    
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
    
    void Print(std::ostream &s, int flag = 0);
    
    // functions used in post-processing only    
    const Vector &getPointGlobalCoordFromLocal(const Vector &) const;
    const Vector &getPointGlobalDisplFromBasic(double xi, const Vector &basicDisps) const;
    const Vector &getVectorGlobalCoordFromLocal(const Vector &localCoords) const;
    const Matrix &getVectorGlobalCoordFromLocal(const Matrix &localCoords) const;
    const Vector &getVectorLocalCoordFromGlobal(const Vector &globalCoords) const;
  };
} // fin namespace XC
#endif