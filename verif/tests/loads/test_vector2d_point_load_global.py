# -*- coding: utf-8 -*-
# home made test
# Referencia:  Expresiones de la flecha el el Prontuario de
# Estructuras Metálicas del CEDEX. Apartado 3.3 Carga puntual sobre ménsula.
#  Se comprueba el funcionamiento del comando vector2d_point_load_global.

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_3dof
from materials import typical_materials
import math

E= 2e6 # Módulo elástico
L= 20 # Longitud de la barra.
h= 0.30 # Canto de la viga.
b= 0.2 # Ancho de la viga.
A= b*h # Área de la sección.
I= b*h**3/12 # Inercia de la viga en pulgadas a la cuarta
x= 0.5 # Abcisa relativa en la que se aplica la carga puntual.
P= 1e3 # Carga transversal.
n= 1e6 # Carga axial.

prueba= xc.ProblemaEF()
mdlr= prueba.getModelador   
nodos= mdlr.getNodeLoader

# Problem type
predefined_spaces.gdls_resist_materiales2D(nodos)

vDisp= [0,0]
vReac1= [0,0]
vReac2= [0,0]

ptoAplic=  geom.Pos2d(1+x*L*math.sqrt(2)/2,2+x*L*math.sqrt(2)/2) # Punto de aplicación de la carga.


nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXY(1,2)
nod= nodos.newNodeXY(1+L*math.sqrt(2)/2,2+L*math.sqrt(2)/2)
    
# Definimos transformaciones geométricas
trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf2d("lin")

    
# Materials definition
scc= typical_materials.defElasticSection2d(mdlr,"scc",A,E,I)


# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin"
elementos.defaultMaterial= "scc"
elementos.defaultTag= 1 #Tag for next element.
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([1,2]))
beam2d.h= h
    
# Constraints
coacciones= mdlr.getConstraintLoader
fix_node_3dof.fixNode000(coacciones,1)

# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
casos.currentLoadPattern= "0"

mesh= prueba.getDomain.getMesh
eIter= mesh.getElementIter
elem= eIter.next()
while not(elem is None):
  crdTransf= elem.getCoordTransf
  vIElem= crdTransf.getIVector
  vJElem= crdTransf.getJVector
  vCarga= n*vIElem-P*vJElem
  elem.vector2dPointLoadGlobal(xc.Vector([ptoAplic.x,ptoAplic.y]),vCarga)
  elem= eIter.next()

#We add the load case to domain.
casos.addToDomain("0")

# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)

nod2= nodos.getNode(2)
vDisp= nod2.getDisp

a= x*L
delta0= vDisp.dot(vIElem)
delta0Teor= (n*a/E/A)
ratio0= ((delta0-delta0Teor)/delta0Teor)
delta1= vDisp.dot(vJElem)
delta1Teor= (-P*a**2*(3*L-a)/6/E/I)
ratio1= ((delta1-delta1Teor)/delta1Teor)


# print "delta0= ",delta0
# print "delta0Teor= ",delta0Teor
# print "ratio0= ",ratio0
# print "delta1= ",delta1
# print "delta1Teor= ",delta1Teor
# print "ratio1= ",ratio1

import os
fname= os.path.basename(__file__)
if (abs(ratio0)<1e-10) & (abs(ratio1)<1e-11):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
