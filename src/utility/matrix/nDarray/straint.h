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
///*
//################################################################################
//# COPYRIGHT (C):     :-))                                                      #
//# PROJECT:           Object Oriented Finite Element Program                    #
//# PURPOSE:           strain tensor with all necessery functions                #
//# CLASS:             straintensor                                              #
//#                                                                              #
//# VERSION:                                                                     #
//# LANGUAGE:          C++.ver >= 2.0 ( Borland C++ ver=3.00, SUN C++ ver=2.1 )  #
//# TARGET OS:         DOS || UNIX || . . .                                      #
//# DESIGNER(S):       Boris Jeremic                                             #
//# PROGRAMMER(S):     Boris Jeremic                                             #
//#                                                                              #
//#                                                                              #
//# DATE:              July 25 '93                                               #
//# UPDATE HISTORY:    August 22-29 '94 choped to separate files and worked on   #
//#                                   const and & issues                         #
//#                    August 30-31 '94 added use_def_dim to full the CC,        #
//#                                   resolved problem with temoraries for       #
//#                                   operators + and - ( +=, -= )               #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//#                                                                              #
//################################################################################
//*/
//
#ifndef STRAINTENSOR_HH
#define STRAINTENSOR_HH

#include "utility/matrix/nDarray/BJtensor.h"
#include <iostream>

namespace XC {

  //! @brief Tensor de deformaciones.
class straintensor : public BJtensor
  {
  public: // just send appropriate arguments to the base constructor
//    straintensor (int rank_of_tensor=2, double initval=0.00000003141528);
    straintensor (int rank_of_tensor=2, double initval=0.0);
// default constructor           // this is just PI/10^8 to check default constructor

    straintensor(double *values);
    straintensor(double initvalue);

    straintensor(const straintensor & x );
    straintensor(const BJtensor & x); // copy-initializer
    straintensor(const nDarray & x); // copy-initializer

    straintensor operator=(const straintensor & rval); // straintensor assignment
    straintensor operator=(const BJtensor & rval);// tensor assignment to straintensor
    straintensor operator=(const nDarray & rval);// nDarray assignment to straintensor

    straintensor deep_copy(void);
//..    straintensor * p_deep_copy(void);

//ini  // use "from" and initialize already allocated strain tensor from "from" values
//ini      void Initialize( const straintensor & from );

//___// operator() overloading for 3D Gauss points!
//___    straintensor & operator()(short ir, short is, short it,
//___                              short tr, short ts, short tt  );
 
    double Iinvariant1(void) const;
    double Iinvariant2(void) const;
    double Iinvariant3(void) const;

    double Jinvariant1(void) const;
    double Jinvariant2(void) const;
    double Jinvariant3(void) const;

    double equivalent(void) const;	  //Zhaohui added 09-02-2000

    straintensor deviator(void) const;
    straintensor principal(void) const;

    double sigma_octahedral(void) const;
    double tau_octahedral(void) const;

    double ksi(void) const;
    double ro(void) const;
    double theta(void) const;
    double thetaPI(void) const;

    double p_hydrostatic(void) const;
    double q_deviatoric(void) const;


    straintensor pqtheta2strain( double, double, double );
    straintensor evoleq2strain( double, double );

    void report(const std::string &) const;
    void reportshort(const std::string &) const;

    friend std::ostream &operator<<(std::ostream &os, const straintensor &rhs);

//..// polinomial root solver friend functions definitions
//..public:
//..friend void laguer(complex *, int , complex *, double , int );
//..friend void zroots(complex *, int , complex *, int );
//..
  };

std::ostream &operator<<(std::ostream &os, const straintensor &rhs);

} // fin namespace XC

#endif
