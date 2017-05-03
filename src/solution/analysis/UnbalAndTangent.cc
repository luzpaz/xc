//----------------------------------------------------------------------------
//  XC program; finite element analysis code
//  for structural analysis and design.
//
//  Copyright (C)  Luis Claudio Pérez Tato
//
//  This program derives from OpenSees <http://opensees.berkeley.edu>
//  developed by the  «Pacific earthquake engineering research center».
//
//  Except for the restrictions that may arise from the copyright
//  of the original program (see copyright_opensees.txt)
//  XC is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or 
//  (at your option) any later version.
//
//  This software is distributed in the hope that it will be useful, but 
//  WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details. 
//
//
// You should have received a copy of the GNU General Public License 
// along with this program.
// If not, see <http://www.gnu.org/licenses/>.
//----------------------------------------------------------------------------
//UnbalAndTangent.cpp

#include "UnbalAndTangent.h"


bool XC::UnbalAndTangent::libera(void)
  {
    // delete tangent and residual if created specially
    if(nDOF>=unbalAndTangentArray.size())
      {
        if(theTangent) delete theTangent;
        theTangent= nullptr;
        if(theResidual) delete theResidual;
        theResidual= nullptr;
        return true;
      }
    else
      return false;
  }

void XC::UnbalAndTangent::alloc(void)
  {
    // create matrices and vectors for each object instance
    if(libera())
      {
        theResidual=new Vector(nDOF);
        theTangent=new Matrix(nDOF, nDOF);
        if(theResidual == 0 || theResidual->Size() ==0 ||  theTangent ==0 || theTangent->noRows() ==0)
          {       
            std::cerr << "UnbalAndTangent::alloc() ";
            std::cerr << " ran out of memory for vector/Matrix of size :";
            std::cerr << nDOF << std::endl;
            exit(-1);
          }
      }
    else
      {
        theResidual= unbalAndTangentArray.setUnbalance(nDOF);
        theTangent= unbalAndTangentArray.setTangent(nDOF);
      }             
  }

void XC::UnbalAndTangent::copia(const UnbalAndTangent &otro)
  {
    // create matrices and vectors for each object instance
    if(libera())
      {
        if(otro.theResidual) theResidual=new Vector(*otro.theResidual);
        if(otro.theTangent) theTangent=new Matrix(*otro.theTangent);
        if(theResidual == 0 || theResidual->Size() ==0 ||  theTangent ==0 || theTangent->noRows() ==0)
          {       
            std::cerr << "UnbalAndTangent::copia() ";
            std::cerr << " ran out of memory for vector/Matrix of size :";
            std::cerr << nDOF << std::endl;
            exit(-1);
          }
      }
    else
      {
        theResidual= otro.theResidual;
        theTangent= otro.theTangent;
      }             
  }

//! @brief Constructor.
XC::UnbalAndTangent::UnbalAndTangent(const size_t &n,UnbalAndTangentStorage &a)
  :nDOF(n), theResidual(nullptr), theTangent(nullptr), unbalAndTangentArray(a) 
  { alloc(); }

//! @brief Copy constructor.
XC::UnbalAndTangent::UnbalAndTangent(const UnbalAndTangent &otro)
  :nDOF(0), theResidual(nullptr), theTangent(nullptr), unbalAndTangentArray(otro.unbalAndTangentArray) 
  {
    libera();
    nDOF= otro.nDOF;
    copia(otro);
  }

//! @brief Assignment operator.
XC::UnbalAndTangent &XC::UnbalAndTangent::operator=(const UnbalAndTangent &otro)
  {
    libera();
    unbalAndTangentArray= otro.unbalAndTangentArray;
    nDOF= otro.nDOF;
    copia(otro);
    return *this;
  }

//! @brief destructor.
XC::UnbalAndTangent::~UnbalAndTangent(void)
  { libera(); }

//! @brief Return the tangent stiffness matrix.
const XC::Matrix &XC::UnbalAndTangent::getTangent(void) const
  {
    assert(theTangent);
    return *theTangent;
  }

//! @brief Return the tangent stiffness matrix.
XC::Matrix &XC::UnbalAndTangent::getTangent(void)
  {
    assert(theTangent);
    return *theTangent;
  }

//! @brief Returns the residual vector.
const XC::Vector &XC::UnbalAndTangent::getResidual(void) const
  {
    assert(theResidual);
    return *theResidual;
  }

//! @brief Return the residual vector.
XC::Vector &XC::UnbalAndTangent::getResidual(void)
  {
    assert(theResidual);
    return *theResidual;
  }
