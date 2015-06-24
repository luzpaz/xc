# -*- coding: utf-8 -*-
from __future__ import division
import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_6dof
from materials import typical_materials

b= 0.4
h= 0.8
A= b*h
E= 200000*9.81/1e-4 # Módulo elástico aproximado del hormigón.
nu= 0.3 # Coeficiente de Poisson
G= E/(2*(1+nu)) # Módulo de elasticidad a cortante
Iy= (1/12.0*h*b**3) # Momento de inercia de la sección expresado en m4
Iz= (1/12.0*b*h**3) # Momento de inercia de la sección expresado en m4
J= 0.721e-8 # Momento de inercia a torsión expresado en m4
L= 5 # Longitud del elmento expresada en metros.

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales3D(nodos)
nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXYZ(1.0,2.0,0.0)
nod= nodos.newNodeXYZ(1.0,2.0,L)


trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf3d("lin")
lin.xzVector= xc.Vector([0,1,0])

# Elements definition
elementos= mdlr.getElementLoader

# Materials
caracMecSeccion= xc.ConstantesSecc3d()
caracMecSeccion.A= A; caracMecSeccion.E= E; caracMecSeccion.G= G;
caracMecSeccion.Iz= Iz; caracMecSeccion.Iy= Iy; caracMecSeccion.J= J
seccion= typical_materials.defElasticSectionFromMechProp3d(mdlr,"seccion",caracMecSeccion)

elementos.defaultTransformation= "lin"
#  sintaxis: elastic_beam_3d[<tag>] 
elementos.defaultMaterial= "seccion"
elementos.defaultTag= 1 #Tag for the next element.
beam3d= elementos.newElement("elastic_beam_3d",xc.ID([1,2]));



crdTransf= beam3d.getCoordTransf
P1= crdTransf.getPointGlobalCoordFromLocal(xc.Vector([0.5,0.0,0.0]))
P2= crdTransf.getPointGlobalCoordFromBasic(0.5)

ratio1= (P1-xc.Vector([1,2,0.5])).Norm()
ratio2= (P2-xc.Vector([1,2,2.5])).Norm()

''' 
print "P1: ",P1
print "P2: ",P2
print "ratio1= ",ratio1
print "ratio2= ",ratio2
  '''

import os
fname= os.path.basename(__file__)
if (ratio1 < 1e-15) & (ratio2 < 1e-15):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."