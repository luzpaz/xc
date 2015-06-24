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
                                                                        
// $Revision: 1.7 $
// $Date: 2005/05/27 00:12:15 $
// $Source: /usr/local/cvs/OpenSees/SRC/recorder/EnvelopeElementRecorder.h,v $
                                                                        
#ifndef EnvelopeElementRecorder_h
#define EnvelopeElementRecorder_h

// Written: fmk 
//
// What: "@(#) EnvelopeElementRecorder.h, revA"

#include "ElementRecorderBase.h"
#include "EnvelopeData.h"

namespace XC {
class Vector;
class Matrix;
class Element;

class EnvelopeElementRecorder: public ElementRecorderBase
  {
  private:	
    int initialize(void);
    EnvelopeData envelope;
  protected:
    int sendData(CommParameters &);  
    int receiveData(const CommParameters &);
  public:
    EnvelopeElementRecorder(void);
    EnvelopeElementRecorder(const ID &eleID, 
			    const std::vector<std::string> &, 
			    Domain &theDomain, 
			    DataOutputHandler &theOutputHandler,
			    double deltaT = 0.0,
			    bool echoTimeFlag = true); 


    ~EnvelopeElementRecorder(void);

    int record(int commitTag, double timeStamp);
    int restart(void);    

    int sendSelf(CommParameters &);  
    int recvSelf(const CommParameters &);
  };
} // fin namespace XC


#endif