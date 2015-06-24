# -*- coding: utf-8 -*-
# home made test

L= 2.0 # Lado del elemento.
t= 0.2 # Canto del elemento.
E= 2.1e6 # Módulo de Young del acero.
nu= 0.3 # Coeficiente de Poisson.
# Load
F= 10 # Magnitud de la carga

import xc_base
import geom
import xc
from model import fix_node_6dof
from model import predefined_spaces
from materials import typical_materials
from solution import predefined_solutions

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales3D(nodos)
nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXYZ(0,0,0)
nod= nodos.newNodeXYZ(L,0,0)
nod= nodos.newNodeXYZ(L,L,0)
nod= nodos.newNodeXYZ(0,L,0)


# Materials definition
nmb1= typical_materials.defElasticMembranePlateSection(mdlr,"memb1",E,nu,0.0,t)

elementos= mdlr.getElementLoader
elementos.defaultMaterial= "memb1"
elementos.defaultTag= 1
elem= elementos.newElement("shell_mitc4",xc.ID([1,2,3,4]))


# Constraints
coacciones= mdlr.getConstraintLoader

fix_node_6dof.Nodo6DOFGirosLibres(coacciones, 1)
spc= coacciones.newSPConstraint(2,1,0.0)
spc= coacciones.newSPConstraint(2,2,0.0)
spc= coacciones.newSPConstraint(3,2,0.0)
spc= coacciones.newSPConstraint(4,2,0.0)

# Loads definition
cargas= mdlr.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(3,xc.Vector([0,F,0,0,0,0]))
lp0.newNodalLoad(4,xc.Vector([0,F,0,0,0,0]))
#We add the load case to domain.
casos.addToDomain("0")



# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)


n1MedioElem= 0.0
n2MedioElem= 0.0
n12MedioElem= 0.0
#elementos= mdlr.getElementLoader
elem.getResistingForce()
mats= elem.getPhysicalProperties.getVectorMaterials #Materials at gauss points.
#Gauss points iterator
for m in mats:
  n1MedioElem= n1MedioElem+m.getStressResultantComponent("n1")
  n2MedioElem= n2MedioElem+m.getStressResultantComponent("n2")
  n12MedioElem= n12MedioElem+m.getStressResultantComponent("n12")


# print "G1= ",vectorG1
elem.getResistingForce()
n1MedioElem= n1MedioElem/4
n2MedioElem= n2MedioElem/4
n12MedioElem= n12MedioElem/4

n1MedioElemTeor= 0.0
n2MedioElemTeor= 2*F/L
n12MedioElemTeor= 0.0
ratio1= abs((n1MedioElem-n1MedioElemTeor))
ratio2= abs((n2MedioElem-n2MedioElemTeor)/n2MedioElemTeor)
ratio3= abs((n12MedioElem-n12MedioElemTeor))

''' 
print "n1MedioElem= ",n1MedioElem
print "n1MedioElemTeor= ",n1MedioElemTeor
print "ratio1= ",ratio1
print "n2MedioElem= ",n2MedioElem
print "n2MedioElemTeor= ",n2MedioElemTeor
print "ratio2= ",ratio2
print "n12MedioElem= ",n12MedioElem
print "n12MedioElemTeor= ",n12MedioElemTeor
print "ratio3= ",ratio3
   '''

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-14) & (abs(ratio2)<1e-14) & (abs(ratio3)<1e-14) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."