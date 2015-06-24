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
// $Date: 2003/02/14 23:00:41 $
// $Source: /usr/local/cvs/OpenSees/SRC/analysis/algorithm/eigenAlgo/FrequencyAlgo.h,v $
                                                                        
                                                                        
// File: ~/analysis/algorithm/eigenAlgo/FrequencyAlgo.h
//
// Written: Jun Peng
// Created: Wed Feb. 8, 1999
// Revision: A
//
// Description: This file contains the class definition of FrequencyAlgo.
// FrequencyAlgo is a class which performs a eigen solution algorithm
// to solve the Generalized eigen equations. It is not expected that 
// this class will have subclasses.
//
// This class is inheritanted from the base class of SolutionAlgorithm
// which was created by fmk (Frank).


#ifndef FrequencyAlgo_h
#define FrequencyAlgo_h

#include <solution/analysis/algorithm/eigenAlgo/EigenAlgorithm.h>

namespace XC {

//! @ingroup EigenAlgo
//
//! @brief Algoritmo para obtener los modos propios del modelo.
class FrequencyAlgo : public EigenAlgorithm
  {
    friend class SoluMethod;
    FrequencyAlgo(SoluMethod *);
    virtual SolutionAlgorithm *getCopy(void) const;
  public:
     
    virtual int solveCurrentStep(int numModes);
     
    virtual int sendSelf(CommParameters &);
    virtual int recvSelf(const CommParameters &);
     
    virtual void Print(std::ostream &s, int flag = 0);
  };
inline SolutionAlgorithm *FrequencyAlgo::getCopy(void) const
  { return new FrequencyAlgo(*this); }
} // fin namespace XC

#endif

