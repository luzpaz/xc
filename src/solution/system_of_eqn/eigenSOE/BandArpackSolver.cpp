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
// File: ~/system_of_eqn/eigenSOE/BandArpackSolver.C
//
// Written: Jun Peng
// Created: Feb. 11, 1999
// Revision: A
//
// Description: This file contains the class definition for 
// BandArpackSolver. It solves the XC::BandArpackSOE object by calling
// Arpack routines.

#include <solution/system_of_eqn/eigenSOE/BandArpackSolver.h>
#include <solution/system_of_eqn/eigenSOE/BandArpackSOE.h>
#include <cstdio>
#include <solution/analysis/model/AnalysisModel.h>
#include <solution/analysis/model/DOF_GrpIter.h>
#include <solution/analysis/model/dof_grp/DOF_Group.h>
#include <solution/analysis/model/FE_EleIter.h>
#include <solution/analysis/model/fe_ele/FE_Element.h>
#include <solution/analysis/integrator/Integrator.h>
#include <cstring>
#include "utility/matrix/Vector.h"
#include "xc_utils/src/base/CmdStatus.h"
#include "xc_basic/src/util/mchne_eps.h"

//! @brief Constructor.
XC::BandArpackSolver::BandArpackSolver(int numE)
  :EigenSolver(EigenSOLVER_TAGS_BandArpackSolver,numE), tol(mchne_eps_dbl), maxitr(1000) {}

//! @brief Lee un objeto XC::BandArpackSolver desde archivo
bool XC::BandArpackSolver::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(BandArpackSolver) Procesando comando: " << cmd << std::endl;
    if(cmd == "numE")
      {
        numModes= interpretaInt(status.GetString());
        return true;
      }
    else if(cmd == "tol")
      {
        tol= interpretaDouble(status.GetString());
        return true;
      }
    if(cmd == "num_max_iter")
      {
        maxitr= interpretaInt(status.GetString());
        return true;
      }
    else
      return EigenSolver::procesa_comando(status);
  }


extern "C" int dgbsv_(int *N, int *KL, int *KU, int *NRHS, double *A, int *LDA,
                      int *iPiv, double *B, int *LDB, int *INFO);

extern "C" int dgbtrf_(int *M, int *N, int *KL, int *KU, double *A, int *LDA,
                       int *iPiv, int *INFO);

extern "C" int dgbtrs_(char *TRANS, int *N, int *KL, int *KU, int *NRHS,
                       double *A, int *LDA, int *iPiv, double *B, int *LDB,
                       int *INFO);

extern "C" int dsaupd_(int *ido, char* bmat, int *n, char *which, int *nev,
                       double *tol, double *resid, int *ncv, double *v, int *ldv,
                       int *iparam, int *ipntr, double *workd, double *workl,
                       int *lworkl, int *info);

extern "C" int dseupd_(bool *rvec, char *howmny, long int *select, double *d, double *z,
                       int *ldz, double *sigma, char *bmat, int *n, char *which,
                       int *nev, double *tol, double *resid, int *ncv, double *v,
                       int *ldv, int *iparam, int *ipntr, double *workd,
                       double *workl, int *lworkl, int *info);


//! @brief Imprime mensajes de error.
void XC::BandArpackSolver::print_err_info(int info)
  {
     switch(info)
       {
       case -1:
         std::cerr << "N must be positive.\n";
         break;
       case -2:
         std::cerr << "NEV must be positive.\n";
         break;
       case -3:
         std::cerr << "NCV must be greater than NEV and less than or equal to N.\n";
         break;
       case -4:
         std::cerr << "The maximum number of Arnoldi update iterations allowed";
         break;
       case -5:
         std::cerr << "WHICH must be one of 'LM', 'SM', 'LA', 'SA' or 'BE'.\n";
         break;
       case -6:
         std::cerr << "BMAT must be one of 'I' or 'G'.\n";
         break;
       case -7:
         std::cerr << "Length of private work array WORKL is not sufficient.\n";
         break;
       case -8:
         std::cerr << "Error return from trid. eigenvalue calculation";
         std::cerr << "Informatinal error from LAPACK routine dsteqr.\n";
         break;
       case -9:
         std::cerr << "Starting vector is zero.\n";
         break;
       case -10:
         std::cerr << "IPARAM(7) must be 1,2,3,4,5.\n";
         break;
       case -11:
         std::cerr << "IPARAM(7) = 1 and BMAT = 'G' are incompatable.\n";
         break;
       case -12:
         std::cerr << "IPARAM(1) must be equal to 0 or 1.\n";
         break;
       case -13:
         std::cerr << "NEV and WHICH = 'BE' are incompatable.\n";
         break;
       case -9999:
         std::cerr << "Could not build an Arnoldi factorization.";
         std::cerr << " IPARAM(5) returns the size of the current Arnoldi\n";
         std::cerr << "factorization. The user is advised to check that";
         std::cerr << "enough workspace and array storage has been allocated.\n";
         break;
       default:
         std::cerr << "unrecognised return value\n";
       }
  }

//! @brief Resuelve el problema de autovalores.
int XC::BandArpackSolver::solve(void)
  {
    if(theSOE == 0)
      {
        std::cerr << "WARNING XC::BandGenLinLapackSolver::solve(void)- ";
        std::cerr << " No LinearSOE object has been set\n";
        return -1;
      }

    int n = theSOE->size; //Número de filas de la matriz (y de columnas ya que es cuadrada).

    // check iPiv is large enough
    if(iPiv.Size() < n)
      {
        std::cerr << "WARNING XC::BandGenLinLapackSolver::solve(void)- ";
        std::cerr << " iPiv not large enough - has setSize() been called?\n";
        return -1;
      }

    // set some variables
    int kl = theSOE->numSubD; //Número de subdiagonales de la matriz.
    int ku = theSOE->numSuperD; //Número de superdiagonales de la matriz.
    int ldA = 2*kl + ku +1;
    int nrhs = 1;
    int ldB = n;
    double *Aptr = theSOE->A.getDataPtr(); //Puntero a la matriz A.
    int *iPIV = iPiv.getDataPtr(); //Índices de los pivotes.

    if(numModes==n)
      std::cerr << "BandArpackSolver::solve; el número de modos a calcular ("
                << numModes << ") debe ser inferior a N= "
                << n << ".\n" << std::endl;
    int nev= numModes;
    int ncv= getNCV(n, nev);

    // set up the space for ARPACK functions.
    // this is done each time method is called!! .. this needs to be cleaned up
    int ldv = n;
    int lworkl = ncv*ncv + 8*ncv;
    std::vector<double> v(ldv * ncv);
    std::vector<double> workl(lworkl + 1);
    std::vector<double> workd(3 * n + 1);
    Vector d(nev);
    Vector z(n * nev);
    std::vector<double> resid(n);
    int iparam[11];
    int ipntr[11];
    std::vector<long int> select(ncv);

    char which[]= "LM";
    char bmat= 'G';
    char howmy= 'A';

    // some more variables
    int mode = 3;

    iparam[0] = 1;
    iparam[2] = maxitr;
    iparam[6] = mode;

    bool rvec = true;

    int ido= 0;

    int ierr= 0;

    // Do the factorization of Matrix (A-dM) here.
    dgbtrf_(&n, &n, &kl, &ku, Aptr, &ldA, iPiv.getDataPtr(), &ierr);

    if(ierr != 0)
      {
        std::cerr << " XC::BandArpackSolver::Error in dgbtrf_ " << std::endl;
        return -1;
      }

    int info= 0;
    char ene[] = "N"; 
    while(1)
      {
        dsaupd_(&ido, &bmat, &n, which, &nev, &tol, &resid[0], &ncv, &v[0], &ldv,iparam, ipntr, &workd[0], &workl[0], &lworkl, &info);
        if(ido == -1)
          {
            myMv(n, &workd[ipntr[0]-1], &workd[ipntr[1]-1]);
            dgbtrs_(ene, &n, &kl, &ku, &nrhs, Aptr, &ldA, iPIV,&workd[ipntr[1] - 1], &ldB, &ierr);
            if(ierr != 0)
              {
                std::cerr << "XC::BandArpackSolver::Error with dgbtrs_ 1" <<std::endl;
                exit(0);
              }
            continue;
          }
        else if(ido == 1)
          {

            //          double ratio = 1.0;
            myCopy(n, &workd[ipntr[2]-1], &workd[ipntr[1]-1]);
            dgbtrs_(ene, &n, &kl, &ku, &nrhs, Aptr, &ldA, iPIV,&workd[ipntr[1] - 1], &ldB, &ierr);
            if(ierr != 0)
              {
                std::cerr << "XC::BandArpackSolver::Error with dgbtrs_ 2" <<std::endl;
                exit(0);
              }
            continue;
          }
        else if(ido == 2)
          {
            myMv(n, &workd[ipntr[0]-1], &workd[ipntr[1]-1]);
            continue;
          }
        break;
      }
    if(info < 0)
      {
        std::cerr << "BandArpackSolver::Error with _saupd info = " 
                  << info << std::endl;
        print_err_info(info);
        return info;
      }
    else
      {
        if(info == 1)
          {
            std::cerr << "BandArpackSolver::Maximum number of iteration reached."
                      << std::endl;
          }
        else if (info == 3)
          {
            std::cerr << "BandArpackSolver::No Shifts could be applied during implicit,";
            std::cerr << "Arnoldi update, try increasing NCV." << std::endl;
          }

        double sigma = theSOE->shift;
        if(iparam[4] > 0)
          {
            rvec = true;
            n= theSOE->size;
            ldv = n;

            dseupd_(&rvec, &howmy, &select[0], d.getDataPtr(), z.getDataPtr(), &ldv, &sigma, &bmat, &n, which,
                    &nev, &tol, &resid[0], &ncv, &v[0], &ldv, iparam, ipntr, &workd[0],
                    &workl[0], &lworkl, &info);
            if(info != 0)
              {
                std::cerr << "BandArpackSolver::Error with dseupd_" << info;
                switch(info)
                  {
                  case -1:
                    std::cerr << " N must be positive.\n";
                    break;
                  case -2:
                    std::cerr << " NEV must be positive.\n";
                    break;
                  case -3:
                    std::cerr << " NCV must be greater than NEV and less than or equal to N.\n";
                    break;
                  case -5:
                    std::cerr << " WHICH must be one of 'LM', 'SM', 'LA', 'SA' or 'BE'.\n";
                    break;
                  case -6:
                    std::cerr << " BMAT must be one of 'I' or 'G'.\n";
                    break;
                  case -7:
                    std::cerr << " Length of private work WORKL array is not sufficient.\n";
                    break;
                  case -8:
                    std::cerr << " Error return from trid. eigenvalue calculation";
                    std::cerr << "Information error from LAPACK routine dsteqr.\n";
                    break;
                  case -9:
                    std::cerr << " Starting vector is zero.\n";
                    break;
                  case -10:
                    std::cerr << " IPARAM(7) must be 1,2,3,4,5.\n";
                    break;
                  case -11:
                    std::cerr << " IPARAM(7) = 1 and BMAT = 'G' are incompatibl\n";
                    break;
                  case -12:
                    std::cerr << " NEV and WHICH = 'BE' are incompatible.\n";
                    break;
                  case -14:
                    std::cerr << " DSAUPD did not find any eigenvalues to sufficient accuracy.\n";
                    break;
                  case -15:
                    std::cerr << " HOWMNY must be one of 'A' or 'S' if RVEC = .true.\n";
                    break;
                  case -16:
                    std::cerr << " HOWMNY = 'S' not yet implemented\n";
                    break;
                  default:
                    ;
                  }
                return info;
              }
          }
      }
    value= d;
    eigenvector= z;
    theSOE->factored = true;

    return 0;
  }

int XC::BandArpackSolver::solve(int nModes)
  {
    numModes = nModes;
    return solve();
  }

//! @brief Calcula un valor adecuado para ncv.
int XC::BandArpackSolver::getNCV(int n, int nev)
  { return std::min(std::min(2*nev,nev+8),n); }


void XC::BandArpackSolver::myMv(int n, double *v, double *result)
  {
    Vector x(v, n);
    Vector y(result,n);

    y.Zero();
    AnalysisModel *theAnalysisModel= theSOE->getAnalysisModelPtr();

    // loop over the FE_Elements
    FE_Element *elePtr;
    FE_EleIter &theEles = theAnalysisModel->getFEs();
    while((elePtr = theEles()) != 0)
      {
        const Vector &b = elePtr->getM_Force(x, 1.0);
        y.Assemble(b, elePtr->getID(), 1.0);
      }

    // loop over the DOF_Groups
    DOF_Group *dofGroupPtr= nullptr;
    DOF_GrpIter &theDofGroups = theAnalysisModel->getDOFGroups();
    while ((dofGroupPtr= theDofGroups()) != 0)
      {
        const Vector &a = dofGroupPtr->getM_Force(x,1.0);
        y.Assemble(a,dofGroupPtr->getID(),1.0);
      }
  }

void XC::BandArpackSolver::myCopy(int n, double *v, double *result)
  {
    for(int i=0; i<n; i++)
      { result[i] = v[i]; }
  }

//! @brief Asigna el problema de autovalores a resolver.
bool XC::BandArpackSolver::setEigenSOE(EigenSOE *soe)
  {
    bool retval= false;
    BandArpackSOE *tmp= dynamic_cast<BandArpackSOE *>(soe);
    if(tmp)
      {
        theSOE= tmp;
        retval= true;
      }
    else
      std::cerr << nombre_clase() << "::setEigenSOE: el sistema de ecuaciones no es del tipo adecuado para este solver." << std::endl;
    return retval;
  }

//! @brief Asigna el problema de autovalores a resolver.
bool XC::BandArpackSolver::setEigenSOE(BandArpackSOE &theBandSOE)
  { return setEigenSOE(&theBandSOE); }

//! @brief Devuelve el autovector correspondiente al modo que se pasa como parámetro.
const XC::Vector &XC::BandArpackSolver::getEigenvector(int mode) const
  {
    if(mode <= 0 || mode > numModes)
      {
        std::cerr << "BandArpackSOE::getEigenvector() - mode is out of range(1 - nev)";
        eigenV.Zero();
        return eigenV;
      }

    const int size= theSOE->size;
    int index= (mode - 1) * size;

    if(!eigenvector.Nulo())
      {
        for(int i=0; i<size; i++)
          eigenV(i)= eigenvector(index++);
      }
    else
      {
        std::cerr << "XC::BandArpackSOE::getEigenvector() - eigenvectors not yet determined";
        eigenV.Zero();
      }
    return eigenV;
  }


//! @brief Devuelve el autovalor correspondiente al modo que se pasa como parámetro.
const double &XC::BandArpackSolver::getEigenvalue(int mode) const
  {
    static double retval= 0.0;
    if(mode <= 0 || mode > numModes)
      {
        std::cerr << "BandArpackSOE::getEigenvalue() - mode is out of range(1 - nev)";
        retval= -1.0;
      }
    if(!value.Nulo())
      return value[mode-1];
    else
      {
        std::cerr << "BandArpackSOE::getEigenvalue() - eigenvalues not yet determined";
        retval= -2.0;
      }
    return retval;
  }

//! @brief Establece el tamaño del problema.
int XC::BandArpackSolver::setSize(void)
  {
    int size = theSOE->size;
    if(iPiv.Size() < size)
      iPiv.resize(size);

    if(eigenV.Size() != size)
      eigenV.resize(size);

    return 0;
  }

//! @brief Devuelve la dimensión de los autovectores.
const int &XC::BandArpackSolver::getSize(void) const
  { return theSOE->size; }


int XC::BandArpackSolver::sendSelf(CommParameters &cp)
  { return 0; }

int XC::BandArpackSolver::recvSelf(const CommParameters &cp)
  {
    // nothing to do
    return 0;
  }


