# -*- coding: utf-8 -*-
# home made test
# Referencia:  Cálculo de estructuras por el método de los elementos finitos. E. Oñate, pg. 165, ejemplo 5.3


L= 1.0 # Bar length (m)
E= 2.1e6*9.81/1e-4 # Módulo elástico
nu= 0.3 # Coeficiente de Poisson
G= E/(2.0*(1+nu)) # Módulo de elasticidad a cortante
alpha= 1.2e-5 # Coeficiente de dilatación del acero
A= 4e-4 # Área de la barra expresada en metros cuadrados
I= (2e-2)**4/12.0 # Momento de inercia de la sección expresada en m4
AT= 10 # Incremento de temperatura expresado en grados centígrados

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from materials import typical_materials

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales2D(nodos)

nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXY(0.0,0.0)
nod= nodos.newNodeXY(L,0.0)


# Definimos transformaciones geométricas
trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf2d("lin")

# Materials definition
seccion= typical_materials.defElasticShearSection2d(mdlr,"seccion",A,E,G,I,1.0)

# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin" # Transformación de coordenadas para los nuevos elementos
elementos.dimElem= 2
elementos.defaultMaterial= "seccion"
elementos.defaultTag= 1
beam= elementos.newElement("force_beam_column_2d",xc.ID([1,2]));
    
# Constraints
coacciones= mdlr.getConstraintLoader

#
spc= coacciones.newSPConstraint(1,0,0.0)
spc= coacciones.newSPConstraint(1,1,0.0)
spc= coacciones.newSPConstraint(1,2,0.0)
spc= coacciones.newSPConstraint(2,0,0.0)
spc= coacciones.newSPConstraint(2,1,0.0)
spc= coacciones.newSPConstraint(2,2,0.0)

# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
ts= casos.newTimeSeries("linear_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
#casos.currentLoadPattern= "0"
eleLoad= lp0.newElementalLoad("beam_strain_load")
eleLoad.elementTags= xc.ID([1])
defTermica= xc.PlanoDeformacion(alpha*AT)
eleLoad.planoDeformacionDorsal= defTermica
eleLoad.planoDeformacionFrontal= defTermica

#We add the load case to domain.
casos.addToDomain("0")

analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)

elem1= elementos.getElement(1)
elem1.getResistingForce()
scc0= elem1.getSections()[0]

axil= scc0.getStressResultantComponent("N")
momento= scc0.getStressResultantComponent("Mz")
cortante= scc0.getStressResultantComponent("Vy")



N= (-E*A*alpha*AT)
ratio= ((axil-N)/N)

''' 
print "N= ",N
print "axil= ",axil
print "ratio= ",ratio
print "momento= ",momento
print "cortante= ",cortante
   '''

import os
fname= os.path.basename(__file__)
if (abs(ratio)<1e-10) & (abs(momento)<1e-10) & (abs(cortante)<1e-10) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."