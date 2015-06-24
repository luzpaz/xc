# -*- coding: utf-8 -*-
# Test tomado del ejemplo 9-1 del libro: Introducción Al Estudio Del Elemento Finito en Ingeniería. Segunda Edición. Tirupathi R. Chandrupatla, Ashok D. Belegundu. Pearson Educación, 1999
import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
# Materials definition
elast= typical_materials.defElasticIsotropic3d(mdlr,"elast3d",200000,0.3,0.0)

nodos= mdlr.getNodeLoader 
predefined_spaces.gdls_elasticidad3D(nodos)
nodos.defaultTag= 1 #Next node number.
nod1= nodos.newNodeXYZ(100,0,100)
nod2= nodos.newNodeXYZ(0,0,100)
nod3= nodos.newNodeXYZ(0,0,200)
nod4= nodos.newNodeXYZ(100,0,200)
nod5= nodos.newNodeXYZ(100,100,100)
nod6= nodos.newNodeXYZ(0,100,100)
nod7= nodos.newNodeXYZ(0,100,200)
nod8= nodos.newNodeXYZ(100,100,200)

nod9= nodos.newNodeXYZ(100,200,100)
nod10= nodos.newNodeXYZ(0,200,100)
nod11= nodos.newNodeXYZ(0,200,200)
nod12= nodos.newNodeXYZ(100,200,200)
nod13= nodos.newNodeXYZ(100,300,100)
nod14= nodos.newNodeXYZ(0,300,100)
nod15= nodos.newNodeXYZ(0,300,200)

nod16= nodos.newNodeXYZ(100,300,200)
nod17= nodos.newNodeXYZ(100,200,0)
nod18= nodos.newNodeXYZ(100,300,0)
nod19= nodos.newNodeXYZ(0,300,0)
nod20= nodos.newNodeXYZ(0,200,0)

elementos= mdlr.getElementLoader
elementos.defaultMaterial= "elast3d"
elementos.defaultTag= 1 #Tag for the next element.

brick1= elementos.newElement("brick",xc.ID([1,2,3,4,5,6,7,8]))
brick2= elementos.newElement("brick",xc.ID([5,6,7,8,9,10,11,12]))
brick3= elementos.newElement("brick",xc.ID([9,10,11,12,13,14,15,16]))
brick4= elementos.newElement("brick",xc.ID([9,10,14,13,17,20,19,18]))

nod17.fix(xc.ID([0,1,2]),xc.Vector([0,0,0]))
nod18.fix(xc.ID([0,1,2]),xc.Vector([0,0,0]))
nod19.fix(xc.ID([0,1,2]),xc.Vector([0,0,0]))
nod20.fix(xc.ID([0,1,2]),xc.Vector([0,0,0]))

# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(4,xc.Vector([0,0,-80000]))
#We add the load case to domain.
casos.addToDomain("0")

# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)

dN1Teor= xc.Vector([-2.1569e-2,-3.7891e-3,-4.0982e-1])
nodos= mdlr.getNodeLoader
dN1= nodos.getNode(1).getDisp

ratio= (dN1-dN1Teor).Norm()

''' 
print "dN1= ",dN1
print "dN1Teor= ",dN1Teor
print "rario= ",ratio
   '''

import os
fname= os.path.basename(__file__)
if(ratio<1e-4):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."