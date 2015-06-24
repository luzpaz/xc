# -*- coding: utf-8 -*-
# home made test
# Suma de acciones sobre ménsula.

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials
from model import fix_node_6dof

# Propiedades del material
E= 2.1e6*9.81/1e-4 # Módulo elástico en Pa
nu= 0.3 # Coeficiente de Poisson
G= E/(2*(1+nu)) # Módulo de elasticidad a cortante

# Propiedades de la sección (IPE-80)
A= 7.64e-4 # Área de la sección expresada en m2
Iy= 80.1e-8 # Momento de inercia de la sección expresada en m4
Iz= 8.49e-8 # Momento de inercia de la sección expresada en m4
J= 0.721e-8 # Momento de inercia a torsión expresado en m4

# Geometry
L= 1.5 # Bar length (m)

# Load
F= 1.5e3 # Magnitud de la carga en kN
GF= 1.33 # Coeficiente de ponderación de la hipótesis.
M= 1.5e3 # Magnitud del momento en kN m
GM= 1.5 # Coeficiente de ponderación de la hipótesis.

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador   
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales3D(nodos)
nodos.defaultTag= 1 #First node number.
nodos.newNodeXYZ(0,0.0,0.0)
nodos.newNodeXYZ(L,0.0,0.0)

trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf3d("lin")
lin.xzVector= xc.Vector([0,1,0])

    
# Materials definition
scc= typical_materials.defElasticSection3d(mdlr,"scc",A,E,G,Iz,Iy,J)

# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin"
elementos.defaultMaterial= "scc"
elementos.defaultTag= 1 #Tag for next element.
beam3d= elementos.newElement("elastic_beam_3d",xc.ID([1,2]));

# Constraints
coacciones= mdlr.getConstraintLoader
fix_node_6dof.fixNode6DOF(coacciones,1)

# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
lp0= casos.newLoadPattern("default","0")
lp0.gammaF= GF
lp1= casos.newLoadPattern("default","1")
lp1.gammaF= GM
lp0.newNodalLoad(2,xc.Vector([F,0,0,0,0,0]))
lp1.newNodalLoad(2,xc.Vector([0,0,0,M,0,0]))
#We add the load case to domain.
casos.addToDomain("0")
#We add the load case to domain.
casos.addToDomain("1")

# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)

delta= nodos.getNode(2).getDisp[0]
theta= nodos.getNode(2).getDisp[3]
elementos.getElement(1).getResistingForce()
N1= elementos.getElement(1).getN1
M1= elementos.getElement(1).getT

deltateor= (GF*F*L/(E*A))
ratio1= (delta-deltateor)/deltateor
ratio2= (N1/F/GF)

thetateor= (GM*M*L/(G*J))
ratio3= (M1/M/GM)
ratio4= (theta-thetateor)/thetateor

# print "delta calculado= ",delta
# print "delta teórico= ",deltateor
# print "theta prog.= ", theta
# print "theta teor.= ", thetateor
# print "ratio1= ",ratio1
# print "ratio2= ",ratio2
# print "M1= ",M1
# print "ratio3= ",ratio3
# print "ratio4= ",ratio4

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-5) & (abs(ratio2-1.0)<1e-5) & (abs(ratio3-1.0)<1e-5) & (abs(ratio4)<1e-5):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
