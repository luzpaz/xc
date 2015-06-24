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
                                                                        
// $Revision: 1.6 $
// $Date: 2005/06/16 21:41:03 $
// $Source: /usr/local/cvs/OpenSees/SRC/material/uniaxial/EPPGapMaterial.h,v $

// File: ~/material/EPPGapMaterial.h
//
// Written: krm
// Created: 07/2000
// Revision: A
//
// Description: This file contains the class definition for 
// ElasticMaterial. EPPGapMaterial provides the abstraction
// of an elastic perfectly plastic (tension only) path dependent uniaxial 
// material, with an initial gap offset (force-displacement units)
// For compression only behavior, enter negative gap and ep
// Damage can accumulate through specification of damage = 1 switch,
// otherwise damage = 0
//
//                  gap ep
//                  |<>|>|
//          stress  |    +-----+ fy
//                  |   /E    /
//         ---------+--+-+---+----strain
//                  |       
//                  |   
//                  |      
//
// What: "@(#) EPPGapMaterial.h, revA"

#ifndef EPPGapMaterial_h
#define EPPGapMaterial_h

#include <material/uniaxial/EPPBaseMaterial.h>

namespace XC {
//! @ingroup MatUnx
//
//! @brief Material elastoplástico perfecto con un «gap» inicial.EPPGapMaterial//! provides the abstraction of an elastic perfectly
//! plastic (tension only) path dependent uniaxial 
//! material, with an initial gap offset (force-displacement units)
//! For compression only behavior, enter negative gap and ep
//! Damage can accumulate through specification of damage = 1 switch,
//! otherwise damage = 0
class EPPGapMaterial : public EPPBaseMaterial
  {
  private:
    double fy;
    double gap;
    double eta;
    double maxElasticYieldStrain;
    double minElasticYieldStrain;
    int damage;
  protected:
    bool procesa_comando(CmdStatus &status);
  public:
    EPPGapMaterial(int tag, double E, double fy, double gap, double eta, int damage = 0);
    EPPGapMaterial(int tag);  
    EPPGapMaterial(void);  

    int setTrialStrain(double strain, double strainRate = 0.0); 
    double getStrain(void) const;          
    double getInitialTangent(void) const;

    int commitState(void);
    int revertToLastCommit(void);    
    int revertToStart(void);    

    UniaxialMaterial *getCopy(void) const;
    
    int sendSelf(CommParameters &);  
    int recvSelf(const CommParameters &);
    
    void Print(std::ostream &s, int flag =0);
    any_const_ptr GetProp(const std::string &cod) const;

  };
} // fin namespace XC


#endif