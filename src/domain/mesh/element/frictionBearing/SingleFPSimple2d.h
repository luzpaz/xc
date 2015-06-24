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

// $Revision: 1.1 $
// $Date: 2009/11/03 23:13:08 $
// $Source: /usr/local/cvs/OpenSees/SRC/element/frictionBearing/SingleFPSimple2d.h,v $

#ifndef SingleFPSimple2d_h
#define SingleFPSimple2d_h

// Written: Andreas Schellenberg (andreas.schellenberg@gmx.net)
// Created: 02/06
// Revision: A
//
// Description: This file contains the class definition for SingleFPSimple2d.
// SingleFPSimple2d is a single-concave friction pendulum element defined by
// two nodes. This simplified version uses small angle approximations and
// accounts for the rotation of the sliding surface by shifting the shear force.

#include "ElemFriccionBase.h"
#include "utility/matrix/Vector.h"
#include "utility/matrix/Matrix.h"

namespace XC {
class Channel;
class FrictionModel;
class UniaxialMaterial;
class Response;

class SingleFPSimple2d : public ElemFriccionBase
  {
  private:
    // private methods
    void setUp();
    double sgn(double x);
    
    // parameters
    double R;           // radius of concave sliding dish
    double h;           // height of articulated slider
    double Reff;        // length from center of dish to pivot point
    
    // state variables
    double ubPlastic;   // plastic displacement in basic system
    
    // committed history variables
    double ubPlasticC;  // plastic displacement in basic system

    static Matrix theMatrix;
    static Vector theVector;
    
    int sendData(CommParameters &);
    int recvData(const CommParameters &);
  public:
    // constructors
    SingleFPSimple2d(int tag, int Nd1, int Nd2,const FrictionModel &theFrnMdl,const double &R,const double &h,const double &uy,const std::vector<UniaxialMaterial *> &theMaterials, const Vector &y= Vector(), const Vector &x= Vector(),const double &mass= 0.0,const int &maxIter= 20,const double &tol= 1E-8);
    SingleFPSimple2d(void);
	
    // public methods to obtain information about dof & connectivity    
    int getNumDOF();
    void setDomain(Domain *theDomain);
	
    // public methods to set the state of the element    
    int commitState();
    int revertToLastCommit();        
    int revertToStart();        
    int update();
	
    // public methods to obtain stiffness, mass, damping and residual information    
    const Matrix &getTangentStiff();
    const Matrix &getInitialStiff();
    const Matrix &getMass();

    int addLoad(ElementalLoad *theLoad, double loadFactor);
    int addInertiaLoadToUnbalance(const Vector &accel);
    
    const Vector &getResistingForce();
    const Vector &getResistingForceIncInertia();
    
    // public methods for element output
    int sendSelf(CommParameters &);
    int recvSelf(const CommParameters &);
    void Print(std::ostream &, int flag = 0);    
	
    Response *setResponse(const std::vector<std::string> &argv, Information &eleInformation);
    int getResponse(int responseID, Information &eleInformation);
    

};
} // fin namespace XC

#endif