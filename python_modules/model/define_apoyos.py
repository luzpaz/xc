# -*- coding: utf-8 -*-

import xc

def defApoyoXY(mdlr,iNod, iElem, matKX, matKY):
  '''
  Define un apoyo para el nodo cuyo tag se pasa como parámetro.
  Las rigideces del apoyo vienen dadas por los materiales que se pasan como parámetro.
  '''
  # Definimos elementos
  nodos= mdlr.getNodeLoader
  nod= nodos.duplicateNode(iNod)
  retvalNodTag= nod.tag

  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKX
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([retvalNodTag,iNod]))
  zl.clearMaterials()
  zl.setMaterial(0,matKX)
  zl.setMaterial(1,matKY)
  # fin de la definición de elementos
  # Condiciones de contorno
  coacciones= mdlr.getConstraintLoader
  spc= coacciones.newSPConstraint(retvalNodTag,0,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,1,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,2,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,3,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,4,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,5,0.0)
  return retvalNodTag

def defApoyoXYZ(mdlr,iNod, iElem, matKX, matKY, matKZ):
  '''
  Define un apoyo para el nodo cuyo tag se pasa como parámetro.
  Las rigideces del apoyo vienen dadas por los materiales que se pasan como parámetro.
  '''
  # Definimos elementos
  nodos= mdlr.getNodeLoader
  nod= nodos.duplicateNode(iNod)
  retvalNodTag= nod.tag

  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKX
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([retvalNodTag,iNod]))
  zl.clearMaterials()
  zl.setMaterial(0,matKX)
  zl.setMaterial(1,matKY)
  zl.setMaterial(2,matKZ)
  # fin de la definición de elementos
  # Condiciones de contorno
  coacciones= mdlr.getConstraintLoader
  spc= coacciones.newSPConstraint(retvalNodTag,0,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,1,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,2,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,3,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,4,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,5,0.0)
  return retvalNodTag

'''
Define un apoyo para el nodo cuyo tag se pasa como parámetro.
   Las rigidez del apoyo viene dada por el material que se pasa como parámetro.
   La dirección en la que actúa el apoyo la pasamos mediante un vector
'''
def defApoyoUniaxialProb2D(mdlr, iNod, iElem, nmbMat, dir):
  # Node duplication
  nodos= mdlr.getNodeLoader
  nod= nodos.duplicateNode(iNod)
  retvalNodTag= nod.tag

  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= nmbMat
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([retvalNodTag,iNod]))
  zl.setupVectors(xc.Vector([dir[0],dir[1],0]),xc.Vector([-dir[1],dir[0],0]))
  zl.clearMaterials()
  zl.setMaterial(0,nmbMat)
  # fin de la definición de elementos
  # Condiciones de contorno
  coacciones= mdlr.getConstraintLoader
  spc= coacciones.newSPConstraint(retvalNodTag,0,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,1,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,2,0.0)
  return retvalNodTag

# Coloca apoyos en todos los nodos de la lista (numera los elementos con el mismo índice de los nodos).
def defApoyoNodosListaXYZ(mdlr,tagNodos, tagElementos, matKX, matKY, matKZ):
  sz= len(tagNodos)
  nodosNuevos= []
  for i in range(0,sz):
    nodosNuevos.append(defApoyoXYZ(mdlr,tagNodos[i],tagElementos[i],matKX,matKY,matKZ))
  return nodosNuevos

# Coloca apoyos en todos los nodos de la lista de puntos.
def defApoyoPuntos(lstPuntosApoyo, offset, matKX, matKY, matKZ):
  nodosApoyo= [] 
  elementosApoyo= [] 
  szListaPuntos= lstPuntosApoyo.size
  # Formamos la lista de nodos de arranque.
  for ii in range(0,szListaPuntos+1):
    pnt= mdlr.getCad.getPnt(lstPuntosApoyo[ii])
    tagNodo= pnt.getTagNodo()
    nodosApoyo.append(tagNodo)
    elementosApoyo.append(tagNodo+offset)
  nodosArranque= defApoyoNodosLista(nodosApoyo,elementosApoyo,matKX,matKY,matKZ)
  return nodosArranque

# Define un apoyo para el nodo cuyo tag se pasa como parámetro.
def defApoyoXYRigZ(mdlr,iNod, iElem, matKX, matKY):
  # Node duplication
  nodos= mdlr.getNodeLoader
  nod= nodos.duplicateNode(iNod)
  retvalNodTag= nod.tag

  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKX
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([retvalNodTag,iNod]))
  zl.clearMaterials()
  zl.setMaterial(0,matKX)
  zl.setMaterial(1,matKY)
  # fin de la definición de elementos
  # Condiciones de contorno
  coacciones= mdlr.getConstraintLoader
  spc= coacciones.newSPConstraint(retvalNodTag,0,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,1,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,2,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,3,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,4,0.0)
  spc= coacciones.newSPConstraint(retvalNodTag,5,0.0)
  eDofs= coacciones.newEqualDOF(retvalNodTag,iNod,xc.ID([2]))
  #eDofs.dofs= xc.ID([2])
  return retvalNodTag


'''
Define un elemento entre los nodos cuyos tags se pasan como parámetros.
   La rigidez del elemento viene definida por los nombres de materiales que
   se pasan como parámetro.
'''
def colocaApoyoEntreNodos(mdlr,iNodA, iNodB, iElem, matKX, matKY, matKZ, matKTHX, matKTHY, matKTHZ):
  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKX
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([iNodA,iNodB]))
  zl.clearMaterials()
  zl.setMaterial(0,matKX)
  zl.setMaterial(1,matKY)
  zl.setMaterial(2,matKZ)
  zl.setMaterial(3,matKTHX)
  zl.setMaterial(4,matKTHY)
  zl.setMaterial(5,matKTHZ)

'''
Define un elemento entre los nodos cuyos tags se pasan como parámetros.
   La rigidez del elemento viene definida por los nombres de materiales que
   se pasan como parámetro.
'''
def colocaApoyoXYEntreNodos(mdlr,iNodA, iNodB, iElem, matKX, matKY):
  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKX
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([iNodA,iNodB]))
  zl.clearMaterials()
  zl.setMaterial(0,matKX)
  zl.setMaterial(1,matKY)

'''
Define un elemento entre los nodos cuyos tags se pasan como parámetros.
   La rigidez del elemento viene definida por el nombre del material que
   se pasa como parámetro.
'''
def colocaApoyoXEntreNodos(mdlr,iNodA, iNodB, iElem, matKX):
  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKX
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([iNodA,iNodB]))
  zl.clearMaterials()
  zl.setMaterial(0,matKX)

# Define un elemento entre los nodos cuyos tags se pasan como parámetros.
def colocaApoyoYEntreNodos(mdlr,iNodA, iNodB, iElem, matKY):
  # Definimos elementos
  elems= mdlr.getElementLoader
  elems.dimElem= 3 # Tridimensional space.
  elems.defaultMaterial= matKY
  elems.defaultTag= iElem #Next element number.
  zl= elems.newElement("zero_length",xc.ID([iNodA,iNodB]))
  zl.clearMaterials()
  zl.setMaterial(1,matKY)

# Define un elemento entre los nodos pertenecientes a los puntos cuyos tags se pasan como parámetros.
def colocaApoyoEntrePuntos(mdlr,iPtA, iPtB, iElem, matKX, matKY, matKZ, matKTHX, matKTHY, matKTHZ):
  tgNodA= mdlr.getCad.getPnt(iPtA).getTagNode()
  tgNodB= mdlr.getCad.getPnt(iPtB).getTagNode()
  colocaApoyoNodos(tgNodA,tgNodB,iElem,matKX,matKY,matKZ,matKTHX,matKTHY,matKTHZ)