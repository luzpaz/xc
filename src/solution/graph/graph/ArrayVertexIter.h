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
                                                                        
// $Revision: 1.1.1.1 $
// $Date: 2000/09/15 08:23:21 $
// $Source: /usr/local/cvs/OpenSees/SRC/graph/graph/ArrayVertexIter.h,v $
                                                                        
                                                                        
// File: ~/analysis/model/simple/ArrayVertexIter.h
//
// Written: fmk 
// Created: Fri Sep 20 15:27:47: 1996
// Revision: A
//
// Description: This file contains the class definition for ArrayVertexIter.
// ArrayVertexIter is an iter for returning the vertices of an object of class
// ArrayGraph. ArrayVertexIter must be written for each subclass of ArrayGraph:
// wherin the vertices are stored differently to that in ArrayGraph.

#ifndef ArrayVertexIter_h
#define ArrayVertexIter_h

#include <solution/graph/graph/VertexIter.h>

namespace XC {
class ArrayGraph;

//! @ingroup Graph
//
//! @brief ArrayVertexIter is an iter for returning the 
//! vertices of an object of class ArrayGraph. ArrayVertexIter
//! must be written for each subclass of ArrayGraph:
//! wherin the vertices are stored differently to that in ArrayGraph.
class ArrayVertexIter: public VertexIter
  {
  private:
    ArrayGraph &myGraph;
    size_t currIndex;
    int numDone;
  protected:
    friend class ArrayGraph;
    ArrayVertexIter(ArrayGraph &theArrayGraph);
  public:
    virtual void reset(void);
    virtual Vertex *operator()(void);
  };
} // fin namespace XC

#endif




