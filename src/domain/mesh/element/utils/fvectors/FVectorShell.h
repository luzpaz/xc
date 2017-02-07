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
//FVectorShell.h

#ifndef FVectorShell_h
#define FVectorShell_h

#include "domain/mesh/element/utils/fvectors/FVectorData.h"

namespace XC {
class Vector;

//! \ingroup ElemFV
//
//! @brief Esfuerzos en un elemento de tipo lámina.
class FVectorShell: public FVectorData<24>
  {
  public:
    FVectorShell(void);
    FVectorShell(const FVectorShell &otro);
    explicit FVectorShell(const Vector &);
    FVectorShell &operator=(const FVectorShell &otro);
    void zero(void);

    FVectorShell &operator*=(const double &d);
    FVectorShell &operator+=(const FVectorShell &otro);
    FVectorShell &operator-=(const FVectorShell &otro);

    void addForce(const size_t &inod,const double &,const double &,const double &);
    void addMoment(const size_t &inod,const double &,const double &,const double &);

    void Print(std::ostream &os) const;
  };

} // end of XC namespace

#endif

