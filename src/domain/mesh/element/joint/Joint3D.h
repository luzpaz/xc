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

// $Revision: 1.2 $
// $Date: 2004/09/01 04:01:27 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/joint/Joint3D.h,v $

// Written: Arash Altoontash, Gregory Deierlein
// Created: 04/03
// Revision: Arash

// Joint3D.h: interface for the Joint3D class.
//
//////////////////////////////////////////////////////////////////////

#ifndef Joint3D_h
#define Joint3D_h


#include <domain/mesh/element/ElemWithMaterial.h>
#include <domain/mesh/element/physical_properties/Joint3DPhysicalProperties.h>
#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include <utility/matrix/ID.h>
#include <domain/domain/Domain.h>

namespace XC {
class Node;
class UniaxialMaterial;
class Response;

//! \ingroup Elem
//
//! @defgroup ElemJoint Elementos para modelizar conexiones.
//!
//! \ingroup ElemJoint
//
//! @brief Elemento conexión para probleas tridimensionales.
class Joint3D: public ElemWithMaterial<7,Joint3DPhysicalProperties>  
  {
  private:
    ID ExternalNodes;
    ID InternalConstraints;
    int numDof, nodeDbTag, dofDbTag;
    static Matrix K;
    static Vector V;
  protected:
   int addMP_Joint(Domain *theDomain, int mpNum, int RetNodeID, int ConNodeID,
                    int RotNodeID, int Rdof, int DspNodeID, int Ddof, 
                    int LrgDispFlag );   
  public:
    Joint3D(void);
    Joint3D(int tag, int nd1, int nd2, int nd3, int nd4, int nd5, int nd6, int IntNodeTag, const UniaxialMaterial &springx, const UniaxialMaterial &springy, const UniaxialMaterial &springz, Domain *theDomain, int LrgDisp);
    Element *getCopy(void) const;  
    ~Joint3D(void);
  
    // methods dealing with domain
    int getNumDOF(void) const;
  
    void setDomain(Domain *theDomain);  
    bool isSubdomain(void) { return false; } ;
        
    // methods dealing with committed state and update
    int update(void);
  
    // methods to return the current linearized stiffness,
    // damping and mass matrices
    const        Matrix &getTangentStiff(void) const;
    const Matrix &getInitialStiff(void) const;   
    const        Matrix &getDamp(void) const;
    const        Matrix &getMass(void) const;
        
    // methods for returning and applying loads
    //virtual Vector &getUVLoadVector(double q1, double q2);
    int addLoad(ElementalLoad *theLoad, double loadFactor);
    int addInertiaLoadToUnbalance(const Vector &accel);

    const        Vector &getResistingForce(void) const;
    const        Vector &getResistingForceIncInertia(void) const;     

    // method for obtaining information specific to an element
    Response* setResponse(const std::vector<std::string> &argv, Information &eleInformation);
    int getResponse(int responseID, Information &eleInformation);
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
    void Print(std::ostream &s, int flag =0);
  };
} // fin namespace XC

#endif