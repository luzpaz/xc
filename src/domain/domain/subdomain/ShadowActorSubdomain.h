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
                                                                        
// $Revision: 1.4 $
// $Date: 2005/11/30 23:47:00 $
// $Source: /usr/local/cvs/OpenSees/SRC/domain/subdomain/ShadowActorSubdomain.h,v $
                                                                        
                                                                        
// Written: fmk 
// Revision: A
//
// Description: This file contains the integer codes used in ShadowSubdomain
// and the ActorSubdomain classes.
//
// What: "@(#) ShadowSubdomain.h, revA"
namespace XC {
static const int ShadowActorSubdomain_setTag = 72;
static const int ShadowActorSubdomain_newStep = 73;
static const int ShadowActorSubdomain_buildSubdomain = 70;
static const int ShadowActorSubdomain_getRemoteData = 71;
static const int ShadowActorSubdomain_addElement = 1;
static const int ShadowActorSubdomain_addNode = 2;
static const int ShadowActorSubdomain_addExternalNode =  3;
static const int ShadowActorSubdomain_addSP_Constraint = 4;
static const int ShadowActorSubdomain_addMP_Constraint = 5;
static const int ShadowActorSubdomain_addLoadPattern = 6;
static const int ShadowActorSubdomain_addNodalLoadToPattern  = 7;
static const int ShadowActorSubdomain_addElementalLoadToPattern = 8;
static const int ShadowActorSubdomain_addSP_ConstraintToPattern  = 9;
static const int ShadowActorSubdomain_removeElement = 10;
static const int ShadowActorSubdomain_removeNode = 11;
static const int ShadowActorSubdomain_removeSP_Constraint = 12;
static const int ShadowActorSubdomain_removeMP_Constraint = 13;
static const int ShadowActorSubdomain_removeLoadPattern = 14;
static const int ShadowActorSubdomain_removeNodalLoadFromPattern = 15;
static const int ShadowActorSubdomain_removeElementalLoadFromPattern = 16;
static const int ShadowActorSubdomain_removeSP_ConstraintFromPattern = 17;
static const int ShadowActorSubdomain_getElement = 20;
static const int ShadowActorSubdomain_getNode = 21;
static const int ShadowActorSubdomain_hasElement = 22;
static const int ShadowActorSubdomain_hasNode = 23;
static const int ShadowActorSubdomain_applyLoad = 32;
static const int ShadowActorSubdomain_setLoadConstant = 31;
static const int ShadowActorSubdomain_update = 33;
static const int ShadowActorSubdomain_updateTimeDt = 38;
static const int ShadowActorSubdomain_computeNodalResponse = 37;
static const int ShadowActorSubdomain_commit = 34;
static const int ShadowActorSubdomain_revertToLastCommit = 35;
static const int ShadowActorSubdomain_revertToStart = 36;
static const int ShadowActorSubdomain_setDomainDecompAnalysis = 40;
static const int ShadowActorSubdomain_domainChange = 41;
static const int ShadowActorSubdomain_clearAnalysis = 42;
static const int ShadowActorSubdomain_setAnalysisAlgorithm = 43;
static const int ShadowActorSubdomain_setAnalysisIntegrator = 44;
static const int ShadowActorSubdomain_setAnalysisLinearSOE = 45;
static const int ShadowActorSubdomain_setAnalysisConvergenceTest = 46;
static const int ShadowActorSubdomain_getTang = 53;
static const int ShadowActorSubdomain_getResistingForce = 54;
static const int ShadowActorSubdomain_computeTang = 55;
static const int ShadowActorSubdomain_computeResidual = 56;
static const int ShadowActorSubdomain_getCost = 60;
static const int ShadowActorSubdomain_setCommitTag = 25;
static const int ShadowActorSubdomain_setCurrentTime = 26;
static const int ShadowActorSubdomain_setCommittedTime = 27;
static const int ShadowActorSubdomain_DIE = 0;
static const int ShadowActorSubdomain_getElementPtr = 90;
static const int ShadowActorSubdomain_getNodePtr = 91;
static const int ShadowActorSubdomain_Print = 150;
static const int ShadowActorSubdomain_addRecorder = 151;
static const int ShadowActorSubdomain_removeRecorders = 152;
static const int ShadowActorSubdomain_getNodeDisp = 92;
static const int ShadowActorSubdomain_setMass = 93;
static const int ShadowActorSubdomain_setRayleighDampingFactors = 94;
static const int ShadowActorSubdomain_wipeAnalysis = 95;
static const int ShadowActorSubdomain_clearAll = 96;

} // fin namespace XC