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
// $Date: 2007/11/29 19:31:09 $
// $Source: /usr/local/cvs/OpenSees/SRC/system_of_eqn/eigenSOE/FullGenEigenSOE.cpp,v $

// Written: Andreas Schellenberg (andreas.schellenberg@gmx.net)
// Created: 11/07
// Revision: A
//
// Description: This file contains the implementation of the
// FullGenEigenSOE class.

#include "FullGenEigenSOE.h"
#include "FullGenEigenSolver.h"
#include <utility/matrix/Matrix.h>
#include <solution/graph/graph/Graph.h>
#include <solution/graph/graph/Vertex.h>
#include <solution/graph/graph/VertexIter.h>
#include <f2c.h>
#include <cmath>

XC::FullGenEigenSOE::FullGenEigenSOE(SoluMethod *owr)
  : EigenSOE(owr,EigenSOE_TAGS_FullGenEigenSOE) {}

//! @brief Asigna el solver que se empleará para resolver el problema de autovalores.
bool XC::FullGenEigenSOE::setSolver(EigenSolver *newSolver)
  {
    bool retval= false;
    FullGenEigenSolver *tmp= dynamic_cast<FullGenEigenSolver *>(newSolver);
    if(tmp)
      {
        tmp->setEigenSOE(*this);
        retval= EigenSOE::setSolver(tmp);
      }
    else
      std::cerr << "BandArpackSOE::setSolver; solver incompatible con sistema de ecuaciones." << std::endl;
    return retval;
  }

int XC::FullGenEigenSOE::setSize(Graph &theGraph)
  {
    int result = 0;
    size = theGraph.getNumVertex();
    if(size==0)
      std::cerr << "¡OJO! error en " << nombre_clase() << "::setSize; el modelo no tiene ningún grado de libertad,"
                << " agrege algún nodo o cambie el gestor de coacciones." << std::endl;

    int newSize = size*size;

    if(newSize > A.Size())
      A.resize(newSize);
    A.Zero();

    if(newSize > M.Size())
      M.resize(newSize);
    M.Zero();

    factored = false;

    // invoke setSize() on the Solver
    EigenSolver *theSolver = this->getSolver();
    if(theSolver)
      {
        int solverOK= theSolver->setSize();
        if(solverOK < 0)
          {
            std::cerr << "WARNING FullGenEigenSOE::setSize() - ";
            std::cerr << "solver failed in setSize()\n";
            return solverOK;
          }
      }
    else
      std::cerr << "FullGenEigenSOE::setSize(); no se ha asignado el solver." << std::endl;
    return result;
  }


int XC::FullGenEigenSOE::addA(const Matrix &a, const ID &id, double fact)
  {
    // check for quick return 
    if(fact == 0.0)
      return 0;

    // check that a and id are of similar size
    int idSize = id.Size();    
    if(idSize != a.noRows() && idSize != a.noCols())
      {
        std::cerr << "FullGenEigenSOE::addA() - Matrix and ID not of similar sizes\n";
        return -1;
      }

    if(fact == 1.0)
      { // do not need to multiply 
        for(int i=0; i<idSize; i++)
          {
            int col = id(i);
            if(col < size && col >= 0)
              {
                double *startColiPtr = A.getDataPtr() + col*size;
                for(int j=0; j<idSize; j++)
                  {
                    int row = id(j);
                    if(row <size && row >= 0)
                      {
                        double *APtr = startColiPtr + row;
                        *APtr += a(j,i);
                      }
                  }  // for j
              } 
          }  // for i
      }
    else
      {
        for(int i=0; i<idSize; i++)
          {
            int col = id(i);
            if(col < size && col >= 0)
              {
                double *startColiPtr = A.getDataPtr() + col*size;
                for(int j=0; j<idSize; j++)
                  {
                    int row = id(j);
                    if(row <size && row >= 0)
                      {
                        double *APtr = startColiPtr + row;
                        *APtr += a(j,i)*fact;
                      }
                  }  // for j
              } 
          }  // for i
      }
    return 0;
  }


int XC::FullGenEigenSOE::addM(const Matrix &m, const ID &id, double fact)
  {
    if(fact != 0.0)
      {
        // check that m and id are of similar size
        int idSize = id.Size();
        if(idSize != m.noRows() && idSize != m.noCols())
          {
            std::cerr << "FullGenEigenSOE::addM() - Matrix and ID not of similar sizes\n";
            return -1;
          }

        resize_mass_matrix_if_needed(size);      

        if(fact == 1.0)
          { // do not need to multiply 
            for(int i=0; i<idSize; i++)
              {
                const int col = id(i);
                if(col < size && col >= 0)
                  {
                    double *startColiPtr = M.getDataPtr() + col*size;
                    for(int j=0; j<idSize; j++)
                      {
                        const int row = id(j);
                        if(row <size && row >= 0)
                          {
                            double *MPtr= startColiPtr + row;
                            *MPtr += m(j,i);
                            massMatrix(row,col)+= m(j,i);
                            if(i!=j)
                              massMatrix(col,row)+= m(j,i);
                          }
                      }  // for j
                  } 
              }  // for i
          }
        else
          {
            for(int i=0; i<idSize; i++)
              {
                int col = id(i);
                if(col < size && col >= 0)
                  {
                    double *startColiPtr = M.getDataPtr() + col*size;
                    for(int j=0; j<idSize; j++)
                      {
                        int row = id(j);
                        if(row <size && row >= 0)
                          {
                            double *MPtr = startColiPtr + row;
                            *MPtr += m(j,i)*fact;
                            massMatrix(row,col)+= m(j,i)*fact;
                            if(i!=j)
                              massMatrix(col,row)+= m(j,i)*fact;
                          }
                      }  // for j
                  } 
              }  // for i
          }
      }
    return 0;
  }


void XC::FullGenEigenSOE::zeroA(void)
  {
    A.Zero();
    factored = false;
  }


void XC::FullGenEigenSOE::zeroM(void)
  {
    EigenSOE::zeroM();
    M.Zero();
    factored = false;
  }


int XC::FullGenEigenSOE::sendSelf(CommParameters &cp)
  { return 0; }


int XC::FullGenEigenSOE::recvSelf(const CommParameters &cp)
  { return 0; }