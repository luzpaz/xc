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
// File: ~/system_of_eqn/eigenSOE/BandArpackSOE.C
//
// Written: Jun Peng
// Created: Febuary 1999
// Revision: A
//
// Description: This file contains the class definition for XC::BandArpackSOE
// BandArpackSOE is a subclass of XC::ArpackSOE. It uses the LAPACK storage
// scheme to store the components of the K, M matrix, which is a full matrix.
// It uses the ARPACK to do eigenvalue analysis.

#include <solution/system_of_eqn/eigenSOE/BandArpackSOE.h>
#include <solution/system_of_eqn/eigenSOE/BandArpackSolver.h>
#include <utility/matrix/Matrix.h>
#include <solution/graph/graph/Graph.h>
#include <solution/graph/graph/Vertex.h>
#include <solution/graph/graph/VertexIter.h>
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_utils/src/base/any_const_ptr.h"

//! @brief Constructor.
XC::BandArpackSOE::BandArpackSOE(SoluMethod *owr, double theShift)
  :ArpackSOE(owr,EigenSOE_TAGS_BandArpackSOE,theShift),
   numSuperD(0), numSubD(0) {}

//! @brief Asigna el solver que se empleará en la solución.
bool XC::BandArpackSOE::setSolver(EigenSolver *newSolver)
  {
    bool retval= false;
    BandArpackSolver *tmp= dynamic_cast<BandArpackSolver *>(newSolver);
    if(tmp)
      {
        tmp->setEigenSOE(*this);
        retval= ArpackSOE::setSolver(tmp);
      }
    else
      std::cerr << "BandArpackSOE::setSolver; solver incompatible con sistema de ecuaciones." << std::endl;
    return retval;
  }

//! @brief Lee un objeto XC::BandArpackSOE desde archivo
bool XC::BandArpackSOE::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(BandArpackSOE) Procesando comando: " << cmd << std::endl;
    return ArpackSOE::procesa_comando(status);
  }

//! @brief Determina el número de superdiagonales y subdiagonales.
int XC::BandArpackSOE::setSize(Graph &theGraph)
  {
    int result = 0;
    size = theGraph.getNumVertex();
    if(size==0)
      std::cerr << "¡OJO! error en " << nombre_clase() << "::setSize; el modelo no tiene ningún grado de libertad,"
                << " agrege algún nodo o cambie el gestor de coacciones." << std::endl;

    // determine the number of superdiagonals and subdiagonals
    theGraph.getBand(numSubD,numSuperD);

    const int newSize = size * (2*numSubD + numSuperD +1);
    if(newSize > A.Size())
      A.resize(newSize);
    A.Zero();
    factored = false;

    // invoke setSize() on the XC::Solver
    EigenSolver *theSolvr = this->getSolver();
    int solverOK = theSolvr->setSize();
    if(solverOK < 0)
      {
        std::cerr << "WARNING: BandArpackSOE::setSize :";
        std::cerr << " solver failed setSize()\n";
        return solverOK;
      }
    return result;
  }

//! @brief Ensambla en A la matriz que se pasa como parámetro multiplicada por el parámetro fact.
int XC::BandArpackSOE::addA(const Matrix &m, const ID &id, double fact)
  {
    // check for a quick return
    if(fact == 0.0)  return 0;

    // check that m and id are of same size
    int idSize = id.Size();
    if(idSize != m.noRows() && idSize != m.noCols())
      {
        std::cerr << "BandArpackSOE::addA(); Matrix and ID not of similar sizes\n";
        return -1;
      }

    const int ldA = 2*numSubD + numSuperD + 1;

    if(fact == 1.0)
      { // do not need to multiply
        for(int i=0; i<idSize; i++)
          {
            const int col= id(i);
            if(col < size && col >= 0)
              {
                double *coliiPtr= A.getDataPtr() + col*ldA + numSubD + numSuperD;
                for(int j=0; j<idSize; j++)
                  {
                    const int row = id(j);
                    if(row <size && row >= 0)
                      {
                        int diff = col - row;
                        if(diff > 0)
                          {
                            if(diff <= numSuperD)
                              {
                                double *APtr = coliiPtr - diff;
                                *APtr += m(j,i);
                              }
                          }
                        else
                          {
                            diff*= -1;
                            if(diff <= numSubD)
                              {
                                double *APtr = coliiPtr + diff;
                                *APtr += m(j,i);
                              }
                          }
                      }
                  }  // for j
              }
          }  // for i
      }
    else
      {
        for(int i=0;i<idSize;i++)
          {
            const int col = id(i);
            if(col < size && col >= 0)
              {
                double *coliiPtr = A.getDataPtr() + col*ldA + numSubD + numSuperD;
                for(int j=0; j<idSize; j++)
                  {
                    const int row = id(j);
                    if(row <size && row >= 0)
                      {
                        int diff = col - row;
                        if(diff > 0)
                          {
                            if(diff <= numSuperD)
                              {
                                double *APtr = coliiPtr - diff;
                                *APtr+= m(j,i)*fact;
                              }
                          }
                        else
                          {
                            diff*= -1;
                            if(diff <= numSubD)
                              {
                                double *APtr = coliiPtr + diff;
                                *APtr+= m(j,i)*fact;
                              }
                          }
                      }
                  }  // for j
              }
          }  // for i
      }
    return 0;
  }

//! @brief Anula la matriz M.
void XC::BandArpackSOE::zeroA(void)
  {
    A.Zero();
    factored = false;
  }

//! @brief Ensambla en M la matriz que se pasa como parámetro multiplicada por el parámetro fact.
int XC::BandArpackSOE::addM(const Matrix &m, const ID &id, double fact)
  {
    bool retval= 0;
    //Añadido LCPT.
    if(fact!=0.0)
      {
        const int idSize = id.Size();
        // check that m and id are of same size
        if(idSize != m.noRows() && idSize != m.noCols())
          {
            std::cerr << "BandArpackSOE::addM(); Matrix and ID not of similar sizes\n";
            retval= -1;
          }
        else
          {
            resize_mass_matrix_if_needed(size);      
            int col= 0, row= 0;
            if(fact==1.0)
              {
                for(int i=0; i<idSize; i++)
                  for(int j=0; j<idSize; j++)
                    {
                      col= id(i);
                      row = id(j);
                      massMatrix(row,col)+= m(i,j);
                    }
              }
            else
              {
                for(int i=0; i<idSize; i++)
                  for(int j=0; j<idSize; j++)
                    {
                      col= id(i);
                      row = id(j);
                      massMatrix(row,col)+= m(i,j)*fact;
                    }
              }
          }
      }
    //Fin añadido LCPT.
    retval= this->addA(m, id, -shift);
    return retval;
  }

//! @brief Anula la matriz M.
void XC::BandArpackSOE::zeroM(void)
  {
    EigenSOE::zeroM();
  }


int XC::BandArpackSOE::sendSelf(CommParameters &cp)
  { return 0; }

int XC::BandArpackSOE::recvSelf(const CommParameters &cp)
  { return 0; }

//! \brief Devuelve la propiedad del objeto cuyo código (de la propiedad) se pasa
//! como parámetro.
any_const_ptr XC::BandArpackSOE::GetProp(const std::string &cod) const
  {
    if(cod=="getASize")
      {
        tmp_gp_szt= A.Size();
        return any_const_ptr(tmp_gp_szt);
      }
    else
      return ArpackSOE::GetProp(cod);
  }