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
  

                                                                        
// $Revision: 1.1 $
// $Date: 2004/09/01 03:54:28 $
// $Source: /usr/local/cvs/OpenSees/SRC/damage/ParkAng.h,v $
                                                                        
#ifndef ParkAng_h
#define ParkAng_h         
                                                               
// Written: AA,GGD
// Created: 10/02
// Revision: AA
//
// Description: This file contains the class definition for 
// Cimbined damage model. It is a subclass od DamageModel
//

#include <utility/handler/ErrorHandler.h>
#include <material/damage/DamageModel.h>

namespace XC {
class DamageResponse;


class ParkAng : public DamageModel
  {
  private:
    double DeltaU , Beta , SigmaY;
    double TrialInfo[6];
    double CommitInfo[6];
    double LastCommitInfo[6];
  public:
    ParkAng(int tag, double deltaU , double beta , double sigmaY );
    ParkAng();  

    int setTrial(double scalar, double scalarRate = 0.0 );
    int setTrial(const Vector &trialVector);
    int setTrial(void); 

    int setInputResponse(Element *elem , const std::vector<std::string> & , int ndof );

    double getDamage(void);
    double getPosDamage(void);
    double getNegDamage(void);
    
    int commitState(void);
    int revertToLastCommit(void);
    int revertToStart(void);

    DamageModel *getCopy(void) const;

    int setVariable(const std::string &argv);
    int getVariable(int variableID, double &info);

    int setParameter(const std::vector<std::string> &argv, Information &eleInformation);
    int updateParameter(int responseID, Information &eleInformation);	

    Response *setResponse(const std::vector<std::string> &argv, Information &info);
    int getResponse(int responseID, Information &info);

    int sendSelf(CommParameters &);  
    int recvSelf(const CommParameters &);
    void Print(std::ostream &s, int flag =0);    

    // method for this damage model to update itself according to its new parameters
    void update(void) {return;}
  };
} // fin namespace XC


#endif