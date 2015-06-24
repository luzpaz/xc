# -*- coding: utf-8 -*-
# home made test
# Verificación del funcionamiento del comando «gira» para una sección elástica en 3d.
#    Ménsula sometida a carga vertical en su extremo con la sección y la carga inclinadas
#    45 grados.

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_6dof
from materials import typical_materials
import math

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
L= 1 # Bar length (m)

# Load
F= 1e3 # Magnitud de la carga en kN

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador   
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales3D(nodos)

nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXYZ(0,0.0,0.0)
nod= nodos.newNodeXYZ(L,0.0,0.0)

trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf3d("lin")
lin.xzVector= xc.Vector([0,1,0])

# Materials definition
scc= typical_materials.defElasticSection3d(mdlr,"scc",E,G,A,Iz,Iy,J)
scc.sectionProperties.gira(math.radians(90))


# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin"
elementos.defaultMaterial= "scc"
elementos.defaultTag= 1
beam= elementos.newElement("force_beam_column_3d",xc.ID([1,2]))

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
lp0.newNodalLoad(2,xc.Vector([0,0,-F,0,0,0]))
#We add the load case to domain.
casos.addToDomain("0")

# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)


nodos= mdlr.getNodeLoader 
nod2= nodos.getNode(2)
delta= nod2.getDisp[2]  # Desplazamiento del nodo 2 según z

elementos= mdlr.getElementLoader

elem1= elementos.getElement(1)
elem1.getResistingForce()
scc0= elem1.getSections()[0]

M= scc0.getStressResultantComponent("Mz")


deltateor= (-F*L**3/(3*E*Iy))
ratio1= abs(delta-deltateor)/deltateor
ratio2= abs((F*L)-M)/(F*L)

''' 
print "delta= ",delta
print "deltaTeor= ",deltateor
print "ratio1= ",ratio1
print "M= ",M
print "MTeor= ",(F*L)
print "ratio2= ",ratio2
 '''

import os
fname= os.path.basename(__file__)
if((ratio1<1e-9) & (ratio2<1e-15)): 
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."

