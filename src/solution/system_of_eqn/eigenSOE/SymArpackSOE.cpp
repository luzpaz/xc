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
// Written: Jun Peng
// Created: 12/98
// Revision: A
//
// Description: This file contains the class definition for 
// LawSparseinSolver. It solves the SymArpackSOEobject by calling
// some "C" functions. The solver used here is generalized sparse
// solver. The user can choose three different ordering schema.
//
// What: "@(#) SymArpackSOE.C, revA"

#include "solution/system_of_eqn/eigenSOE/SymArpackSOE.h"
#include "solution/system_of_eqn/eigenSOE/SymArpackSolver.h"
#include "solution/system_of_eqn/linearSOE/sparseSYM/symbolic.h"
#include <utility/matrix/Matrix.h>
#include "solution/graph/graph/Graph.h"
#include <solution/graph/graph/Vertex.h>
#include <solution/graph/graph/VertexIter.h>
#include <cmath>
#include <utility/matrix/Vector.h>
#include "xc_utils/src/base/CmdStatus.h"

//! @brief Constructor.
XC::SymArpackSOE::SymArpackSOE(SoluMethod *owr,double theShift)
  :ArpackSOE(owr,EigenSOE_TAGS_SymArpackSOE,theShift),
   nnz(0), colA(0), rowStartA(0), 
   nblks(0), xblk(0), invp(0), diag(0), penv(0), rowblks(0),
   begblk(0), first(0) {}

//! @brief Asigna el solver que se empleará para resolver el problema de autovalores.
bool XC::SymArpackSOE::setSolver(EigenSolver *newSolver)
  {
    bool retval= false;
    SymArpackSolver *tmp= dynamic_cast<SymArpackSolver *>(newSolver);
    if(tmp)
      {
        tmp->setEigenSOE(*this);
        retval= ArpackSOE::setSolver(tmp);
      }
    else
      std::cerr << "XC::BandArpackSOE::setSolver; solver incompatible con sistema de ecuaciones." << std::endl;
    return retval;
  }

//! @brief Lee un objeto XC::BandArpackSOE desde archivo
bool XC::SymArpackSOE::procesa_comando(CmdStatus &status)
  {
    const std::string cmd= deref_cmd(status.Cmd());
    if(verborrea>2)
      std::clog << "(SymArpackSOE) Procesando comando: " << cmd << std::endl;
    return ArpackSOE::procesa_comando(status);
  }

//! @brief Destructor.
XC::SymArpackSOE::~SymArpackSOE(void)
  {
  /*
    if(invp != 0) free((void *)invp);
    if(xblk != 0) free((void *)xblk); 
    if(penv != 0) {
      free((void *)penv[0]); 
      free((void *)penv); 
    }
    if(diag != 0) free((void *)diag); 
    if(rowblks != 0) free((void *)rowblks); 
    if(begblk != 0) free((void *)begblk); 
    if(first  != 0) free((void *)first); 

    if(rowStartA != 0) delete [] rowStartA;
    if(colA != 0) delete [] colA;
  */
  }


//! @brief Establece el tamaño del problema.				
int XC::SymArpackSOE::setSize(Graph &theGraph)
  {
    int result = 0;
    //int oldSize = size;
    size = theGraph.getNumVertex();
    if(size==0)
      std::cerr << "¡OJO! error en " << nombre_clase() << "::setSize; el modelo no tiene ningún grado de libertad,"
                << " agrege algún nodo o cambie el gestor de coacciones." << std::endl;

    // fist itearte through the vertices of the graph to get nnz
    Vertex *theVertex;
    int newNNZ = 0;
    VertexIter &theVertices = theGraph.getVertices();
    while((theVertex = theVertices()) != 0)
      {
        const std::set<int> &theAdjacency = theVertex->getAdjacency();
	newNNZ += theAdjacency.size(); 
      }
    nnz = newNNZ;

    colA.resize(newNNZ);	
    factored = false;

    rowStartA.resize(size+1); 

    // fill in rowStartA and colA
    if(size != 0)
      {
        rowStartA[0] = 0;
        int startLoc = 0;
	int lastLoc = 0;

	for(int a=0; a<size; a++)
          {
	    theVertex = theGraph.getVertexPtr(a);
	    if(theVertex == 0)
              {
	        std::cerr << "WARNING:XC::SymArpackSOE::setSize :";
	        std::cerr << " vertex " << a << " not in graph! - size set to 0\n";
	        size = 0;
	        return -1;
	      }

	    const std::set<int> &theAdjacency = theVertex->getAdjacency();
	
	    // now we have to place the entries in the XC::ID into order in colA
            for(std::set<int>::const_iterator i=theAdjacency.begin(); i!=theAdjacency.end(); i++)
              {
	        const int row = *i;
	        bool foundPlace = false;
	 
	        for(int j=startLoc; j<lastLoc; j++)
	          if(colA[j] > row)
                    { 
	              // move the entries already there one further on
	              // and place col in current location
	              for(int k=lastLoc; k>j; k--)
		        colA[k] = colA[k-1];
                      colA[j] = row;
		      foundPlace = true;
    	              j = lastLoc;
		    }
		  
	        if(foundPlace == false) // put in at the end
	      	  colA[lastLoc] = row;
                lastLoc++;
	      }
	    rowStartA[a+1] = lastLoc;;	    
	    startLoc = lastLoc;
	  }
      }

    int LSPARSE = 1;

// call "C" function to form elimination tree and do the symbolic factorization.
//    nblks = symFactorization(rowStartA, colA, size, LSPARSE);
    nblks = symFactorization(rowStartA.getDataPtr(), colA.getDataPtr(), size, LSPARSE,
			     &xblk, &invp, &rowblks, &begblk, &first, &penv, &diag);

    // invoke setSize() on the XC::Solver
    EigenSolver *theSolvr = this->getSolver();
    int solverOK = theSolvr->setSize();
    if(solverOK < 0)
      {
	std::cerr << "WARNING:XC::SymArpackSOE::setSize :";
	std::cerr << " solver failed setSize()\n";
	return solverOK;
      } 
    return result;
  }

//! @brief Ensambla en A la matriz que se pasa como parámetro multiplicada por el parámetro fact.
int XC::SymArpackSOE::addA(const Matrix &m, const ID &id, double fact)
  {
    // check for a XC::quick return
    if(fact == 0.0)  
	return 0;

    const int idSize = id.Size();
    
    // check that m and id are of similar size
    if(idSize != m.noRows() && idSize != m.noCols())
      {
	std::cerr << "SymArpackSOE::addA() ";
	std::cerr << " - Matrix and XC::ID not of similar sizes\n";
	return -1;
      }
    
    std::vector<int> newID(idSize);
    std::vector<int> isort(idSize);

    int i;    
    for(i=0; i<idSize; i++)
      {
        newID[i] = id(i);
	 if(newID[i] >= 0)
	       newID[i] = invp[newID[i]];
      }
       
   long int  i_eq, j_eq; //, iadd;
   int  j, nee, lnee;
   int  k, ipos, jpos;
   int  it, jt;
   int  iblk;
   //int  jblk;
   OFFDBLK  *ptr;
   OFFDBLK  *saveblk;
   double  *fpt, *iloc, *loc;

   nee = idSize;
   lnee = nee;
   
   /* initialize isort */
   for(i = 0, k = 0; i < lnee ; i++ )
   {
       if( newID[i] >= 0 ) {
	    isort[k] = i;
	    k++;
       }
    }
      
   lnee = k;

   i = k - 1;
   do
   {
       k = 0 ;
       for(j = 0 ; j < i ; j++)
       {  
	   if( newID[isort[j]] > newID[isort[j+1]]) {  
	       int temp = isort[j];
	       isort[j] = isort[j+1];
	       isort[j+1] = temp;

/*	       isort[j] ^= isort[j+1] ;
	       isort[j+1] ^= isort[j] ;
	       isort[j] ^= isort[j+1] ;
*/
	       k = j ;
	   }
      }
      i = k ;
   }  while( k > 0) ;

      i = 0 ;
      ipos = isort[i] ;
      k = rowblks[newID[ipos]] ;
      saveblk  = begblk[k] ;

      for(i=0; i<lnee; i++)
      { 
	 ipos = isort[i] ;
         i_eq = newID[ipos] ;
	 iblk = rowblks[i_eq] ;
	 iloc = penv[i_eq +1] - i_eq ;
	 if(k < iblk)
	    while(saveblk->row != i_eq) 
	         saveblk = saveblk->bnext ;
	 
	 ptr = saveblk ;
	 for(j=0; j< i ; j++)
	 {   
	    jpos = isort[j] ;
	    j_eq = newID[jpos] ;

		if(ipos > jpos) {
			jt = ipos;
			it = jpos;
		} else {
			it = ipos;
			jt = jpos;
		}

	    if(j_eq >= xblk[iblk]) /* diagonal block */
	    {  loc = iloc + j_eq ;
	       *loc += m(it, jt) * fact;
            } else /* row segment */
	    {  while((j_eq >= (ptr->next)->beg) && ((ptr->next)->row == i_eq))
		  ptr = ptr->next ;
	       fpt = ptr->nz ;
	       fpt[j_eq - ptr->beg] += m(it,jt) * fact;
            }
         }
	 diag[i_eq] += m(ipos, ipos) * fact;
      }
   
    return 0;
  }

//! @brief Ensambla en M la matriz que se pasa como parámetro multiplicada por el parámetro fact.    
int XC::SymArpackSOE::addM(const Matrix &m, const ID &id, double fact)
  { 
    //Añadido LCPT.
    // check that m and id are of same size
    const int idSize = id.Size();
    if(idSize != m.noRows() && idSize != m.noCols())
      {
        std::cerr << "BandArpackSOE::addA(); Matrix and ID not of similar sizes\n";
        return -1;
      }
    resize_mass_matrix_if_needed(idSize);      

    for(int i=0; i<idSize; i++)
      for(int j=0; j<idSize; j++)
        massMatrix(i,j)+= m(j,i);
    //Fin añadido LCPT.

    return this->addA(m, id, -shift);
  }

//! @brief Anula la matriz A.
void XC::SymArpackSOE::zeroA(void)
  { factored = false; }

//! @brief Anula la matriz M.
void XC::SymArpackSOE::zeroM(void)
  {
    EigenSOE::zeroM();
  }

int XC::SymArpackSOE::sendSelf(CommParameters &cp)
  { return 0; }

int XC::SymArpackSOE::recvSelf(const CommParameters &cp)
  { return 0; }











