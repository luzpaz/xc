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
// File: ~/system_of_eqn/linearSOE/symLinSolver/SymSparseLinSOE.CPP
//
// Written: Jun Peng  (junpeng@stanford.edu)
//          Advisor: Prof. Kincho H. Law
//          Stanford University
// Created: 12/1998
// Revised: 12/2001
//
// Description: This file contains the class definition for 
// SymSparseinSolver. It solves the SymSparseLinSOEobject by calling
// some "C" functions. The solver used here is a symmtric generalized sparse
// solver. The user can choose three different ordering scheme.
//
// What: "@(#) SymSparseLinSOE.C, revA"


#include <fstream>
#include <cstdlib>

#include <solution/system_of_eqn/linearSOE/sparseSYM/SymSparseLinSOE.h>
#include <solution/system_of_eqn/linearSOE/sparseSYM/SymSparseLinSolver.h>
#include <solution/system_of_eqn/linearSOE/sparseSYM/symbolic.h>
#include <utility/matrix/Matrix.h>
#include <utility/matrix/Vector.h>
#include "solution/graph/graph/Graph.h"
#include <solution/graph/graph/Vertex.h>
#include <solution/graph/graph/VertexIter.h>
#include <cmath>


XC::SymSparseLinSOE::SymSparseLinSOE(SoluMethod *owr,int lSparse)
  :SparseSOEBase(owr,LinSOE_TAGS_SymSparseLinSOE),
   nblks(0), xblk(0), invp(0), diag(0), penv(0), rowblks(0),
   begblk(0), first(0) 
  {
    LSPARSE = lSparse;
  }


bool XC::SymSparseLinSOE::setSolver(LinearSOESolver *newSolver)
  {
    bool retval= false;
    SymSparseLinSolver *tmp= dynamic_cast<SymSparseLinSolver *>(newSolver);
    if(tmp)
      retval= SparseSOEBase::setSolver(tmp);
    else
      std::cerr << "SymSparseLinSOE::setSolver; solver incompatible con sistema de ecuaciones." << std::endl;
    return retval;
  }

/* A destructor for cleanning memory.
 * For diag and penv, it is rather straightforward to clean.
 * For row segments, since the memory of nz is allocated for each
 * row, the deallocated needs some special care.
 */
XC::SymSparseLinSOE::~SymSparseLinSOE(void)
  {
    // free the diagonal vector
    if(diag != nullptr) free(diag);

    // free the diagonal blocks
    if(penv != nullptr)
      {
	if(penv[0] != nullptr)
          { free(penv[0]); }
        free(penv);
      }

    // free the row segments.
    OFFDBLK *blkPtr = first;
    OFFDBLK *tempBlk;
    int curRow = -1;

    while(1)
      {
        if(blkPtr->next == blkPtr)
          {
	    if(blkPtr != nullptr)
              { free(blkPtr); }     
	    break;
          }

        tempBlk = blkPtr->next;
        if(blkPtr->row != curRow)
          {
	    if(blkPtr->nz != nullptr)
              { free(blkPtr->nz); }
            curRow = blkPtr->row;
          }
        free(blkPtr);
        blkPtr = tempBlk;
      }

    // free the "C" style vectors.
    if(xblk != 0)  free(xblk);
    if(rowblks != 0)   free(rowblks);
    if(invp != 0)  free(invp);
    
  }


/* Based on the graph (the entries in A), set up the pair (rowStartA, colA).
 * It is the same as the pair (ADJNCY, XADJ).
 * Then perform the symbolic factorization by calling symFactorization().
 */
int XC::SymSparseLinSOE::setSize(Graph &theGraph)
  {
    int result = 0;
    size = theGraph.getNumVertex();
    if(size==0)
      std::cerr << "¡OJO! error en " << nombre_clase() << "::setSize; el modelo no tiene ningún grado de libertad,"
                << " agrege algún nodo o cambie el gestor de coacciones." << std::endl;

    // first itearte through the vertices of the graph to get nnz
    Vertex *theVertex;
    int newNNZ = 0;
    VertexIter &theVertices = theGraph.getVertices();
    while((theVertex = theVertices()) != 0)
      {
        const std::set<int> &theAdjacency = theVertex->getAdjacency();
	newNNZ += theAdjacency.size(); 
      }
    nnz = newNNZ;
 
    colA= ID(newNNZ);	
    if(colA.Nulo())
      {
        std::cerr << "WARNING SymSparseLinSOE::setSize :";
	std::cerr << " ran out of memory for colA with nnz = ";
      	std::cerr << newNNZ << " \n";
       	size = 0; nnz = 0;
       	result =  -1;
      } 
	
    factored = false;
    
    if(size > B.Size())
      {
	inic(size);
	rowStartA= ID(size+1); 
      }

    // fill in rowStartA and colA
    if(size != 0)
      {
        rowStartA(0) = 0;
        int startLoc = 0;
	int lastLoc = 0;

	for (int a=0; a<size; a++) {
	   theVertex = theGraph.getVertexPtr(a);
	   if(theVertex == 0) {
	        std::cerr << "WARNING:XC::SymSparseLinSOE::setSize :";
	        std::cerr << " vertex " << a << " not in graph! - size set to 0\n";
	        size = 0;
	        return -1;
	   }

	   const std::set<int> &theAdjacency = theVertex->getAdjacency();
	
	// now we have to place the entries in the ID into order in colA
           for(std::set<int>::const_iterator i=theAdjacency.begin(); i!=theAdjacency.end(); i++)
  	     {
	      const int row= *i;
	      bool foundPlace = false;
	 
	      for (int j=startLoc; j<lastLoc; j++)
		if(colA(j) > row) { 
	      // move the entries already there one further on
	      // and place col in current location
	              for (int k=lastLoc; k>j; k--)
			colA(k)= colA(k-1);
                      colA(j) = row;
		      foundPlace = true;
    	              j = lastLoc;
		  }
		  
	      if(foundPlace == false) // put in at the end
		colA(lastLoc) = row;

	      lastLoc++;
	   }
	   rowStartA(a+1)= lastLoc;	    
	   startLoc = lastLoc;
	}
    }
    
    // call "C" function to form elimination tree and to do the symbolic factorization.
    nblks = symFactorization(rowStartA.getDataPtr(), colA.getDataPtr(), size, this->LSPARSE,
			     &xblk, &invp, &rowblks, &begblk, &first, &penv, &diag);

    return result;
}


/* Perform the element stiffness assembly here.
 */
int XC::SymSparseLinSOE::addA(const XC::Matrix &in_m, const XC::ID &in_id, double fact)
{
   // check for a XC::quick return
   if(fact == 0.0)  
       return 0;

   int idSize = in_id.Size();
   if(idSize == 0)  return 0;

   // check that m and id are of similar size
   if(idSize != in_m.noRows() && idSize != in_m.noCols()) {
       std::cerr << "XC::SymSparseLinSOE::addA() ";
       std::cerr << " - Matrix and XC::ID not of similiar sizes\n";
       return -1;
   }

   // construct m and id based on non-negative id values.
   int newPt = 0;
   std::vector<int> id(idSize);
   
   for(int jj = 0; jj < idSize; jj++)
      {
       if(in_id(jj) >= 0 && in_id(jj) < size) {
	   id[newPt] = in_id(jj);
	   newPt++;
       }
   }

   idSize = newPt;
   if(idSize == 0)  return 0;
   std::vector<double> m(idSize*idSize);

   int newII = 0;
   for (int ii = 0; ii < in_id.Size(); ii++) {
       if(in_id(ii) >= 0 && in_id(ii) < size) {

	   int newJJ = 0;
	   for (int jj = 0; jj < in_id.Size(); jj++) {
	       if(in_id(jj) >= 0 && in_id(jj) < size) {
		   m[newII*idSize + newJJ] = in_m(ii, jj);
		   newJJ++;
	       }
	   }
	   newII++;
       }
   }

   // forming the new_ id based on invp.

   std::vector<int> newID(idSize);
   std::vector<int> isort(idSize);

   for (int kk=0; kk<idSize; kk++) {
       newID[kk] = id[kk];
       if(newID[kk] >= 0)
	   newID[kk] = invp[newID[kk]];
   }
   
   long int  i_eq, j_eq;
   int  i, j, nee, lnee;
   int  k, ipos, jpos;
   int  it, jt;
   int  iblk;
   OFFDBLK  *ptr;
   OFFDBLK  *saveblk;
   double  *fpt, *iloc, *loc;

   nee = idSize;
   lnee = nee;
   
   /* initialize isort */
   for( i = 0, k = 0; i < lnee ; i++ )
   {
       if( newID[i] >= 0 ) {
	   isort[k] = i;
	   k++;
       }
   }
      
   lnee = k;

   /* perform the sorting of isort here */
   i = k - 1;
   do
   {
       k = 0 ;
       for (j = 0 ; j < i ; j++)
       {  
	   if( newID[isort[j]] > newID[isort[j+1]]) {  
	       isort[j] ^= isort[j+1] ;
	       isort[j+1] ^= isort[j] ;
	       isort[j] ^= isort[j+1] ;
	       k = j ;
	   }
      }
      i = k ;
   }  while ( k > 0) ;

      i = 0 ;
      ipos = isort[i] ;
      k = rowblks[newID[ipos]] ;
      saveblk  = begblk[k] ;

      /* iterate through the element stiffness matrix, assemble each entry */
      for (i=0; i<lnee; i++)
      { 
	 ipos = isort[i] ;
         i_eq = newID[ipos] ;
	 iblk = rowblks[i_eq] ;
	 iloc = penv[i_eq +1] - i_eq ;
	 if(k < iblk)
	    while (saveblk->row != i_eq) saveblk = saveblk->bnext ;
	 
	 ptr = saveblk ;
	 for (j=0; j< i ; j++)
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

	    if(j_eq >= xblk[iblk]) /* diagonal block (profile) */
	    {  
	        loc = iloc + j_eq ;
		*loc += m[it*idSize + jt] * fact;
            } 
	    else /* row segment */
	    { 
	        while((j_eq >= (ptr->next)->beg) && ((ptr->next)->row == i_eq))
		    ptr = ptr->next ;
		fpt = ptr->nz ;
		fpt[j_eq - ptr->beg] += m[it*idSize + jt] * fact;
            }
         }
	 diag[i_eq] += m[ipos*idSize + ipos] * fact; /* diagonal element */
      }
  	  
    return 0;
  }

    
/* assemble the force vector B (A*X = B).
 */
int XC::SymSparseLinSOE::addB(const XC::Vector &in_v, const ID &in_id,const double &fact)
  {
    // check for a XC::quick return 
    if(fact == 0.0)  return 0;

    int idSize = in_id.Size();    
    // check that m and id are of similar size
    if(idSize != in_v.Size() )
      {
	std::cerr << "XC::SymSparseLinSOE::addB() ";
	std::cerr << " - Vector and XC::ID not of similar sizes\n";
	return -1;
      }

     // construct v and id based on non-negative id values.
     int newPt = 0;
     std::vector<int> id(idSize);
     Vector v(idSize);

     for(int ii = 0; ii < idSize; ii++)
       {
         if(in_id(ii) >= 0 && in_id(ii) < size)
           {
	     id[newPt] = in_id(ii);
	     v[newPt] = in_v(ii);
	     newPt++;
           }
       }

     idSize = newPt;
     if(idSize == 0)
       return 0;

     ID newID(idSize);

     for(int i=0; i<idSize; i++)
       {
         newID[i]= id[i];
	 if(newID[i] >= 0)
	   newID[i] = invp[newID[i]];
       }

    SparseSOEBase::addB(v,newID,fact);

    return 0;
  }


/* It is used to set all the entries of A to be zero.
 * This method will be called if the structure of A stays the same while the
 * value of A needs to be changed.  e.g. if Newton-Raphson method is used
 * for analysis, after each iteration, the A matrix needs to be updated.
 */
void XC::SymSparseLinSOE::zeroA(void)
  {
    memset(diag, 0, size*sizeof(double));

    int profileSize = penv[size] - penv[0];
    memset(penv[0], 0, profileSize*sizeof(double));
    
    OFFDBLK *blkPtr = first;
    int rLen = 0;
    while (1) {
        if(blkPtr->beg == size)  break;
	rLen = xblk[rowblks[blkPtr->beg]+1] - blkPtr->beg;
	memset(blkPtr->nz, 0, rLen*sizeof(double));

	blkPtr = blkPtr->next;
    }

    factored = false;
  }
	
/* Create a linkage between SOE and XC::Solver.
 */
int XC::SymSparseLinSOE::setSymSparseLinSolver(SymSparseLinSolver *newSolver)
  {    
    if(size)
      {
        int solverOK= newSolver->setSize();
	if(solverOK < 0)
          {
	    std::cerr << "WARNING:SymSparseLinSOE::setSolver :";
	    std::cerr << "the new_ solver could not setSeize() - staying with old\n";
	    return -1;
	  }
      }
    return this->SparseSOEBase::setSolver(newSolver);
  }


int XC::SymSparseLinSOE::sendSelf(CommParameters &cp)
  {
    // not implemented.
    return 0;
  }


int XC::SymSparseLinSOE::recvSelf(const CommParameters &cp)
  {
    // not implemented.
    return 0;
  }
