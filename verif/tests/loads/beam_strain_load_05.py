# -*- coding: utf-8 -*-
# home made test
# Referencia:  Cálculo de estructuras por el método de los elementos finitos. E. Oñate, pg. 165, ejemplo 5.3

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_3dof
from materials import typical_materials

L= 1.0 # Bar length (m)
E= 2.1e6*9.81/1e-4 # Módulo elástico
nu= 0.3 # Coeficiente de Poisson
G= E/(2*(1+nu)) # Módulo de elasticidad a cortante
alpha= 1.2e-5 # Coeficiente de dilatación del acero
A= 4e-4 # Área de la barra expresada en metros cuadrados
Iy= 80.1e-8 # Momento de inercia de la sección expresada en m4
Iz= 8.49e-8 # Momento de inercia de la sección expresada en m4
J= 0.721e-8 # Momento de inercia a torsión expresado en m4
AT= 10 # Incremento de temperatura expresado en grados centígrados

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales2D(nodos)

nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXY(0.0,0.0)
nod= nodos.newNodeXY(L/2,0.0)
nod= nodos.newNodeXY(L,0.0)


# Definimos transformaciones geométricas
trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf2d("lin")

# Materials definition
seccion= typical_materials.defElasticShearSection2d(mdlr,"seccion",A,E,G,Iz,1.0)

# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin" # Transformación de coordenadas para los nuevos elementos
elementos.defaultMaterial= "seccion"
elementos.defaultTag= 1
beam1= elementos.newElement("force_beam_column_2d",xc.ID([1,2]));
beam2= elementos.newElement("force_beam_column_2d",xc.ID([2,3]));

    
# Constraints
coacciones= mdlr.getConstraintLoader
fix_node_3dof.fixNode000(coacciones,1)
fix_node_3dof.fixNode000(coacciones,3)

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

# Procedimiento de solución
analisis= predefined_solutions.simple_static_modified_newton(prueba)
result= analisis.analyze(1)


nod2= nodos.getNode(2)
dX= nod2.getDisp[0] 
dY= nod2.getDisp[1]   

elem1= elementos.getElement(1)
elem1.getResistingForce()
scc0= elem1.getSections()[0]

axil= scc0.getStressResultantComponent("N")
momento= scc0.getStressResultantComponent("Mz")
cortante= scc0.getStressResultantComponent("Vy")



N= (-E*A*alpha*AT)
ratio= ((axil-N)/N)

''' 
print "dX= ",dX
print "dY= ",dY
print "N= ",N
print "axil= ",axil
print "ratio= ",ratio
print "momento= ",momento
print "cortante= ",cortante
   '''

import os
fname= os.path.basename(__file__)
if (abs(dX)<1e-10) & (abs(ratio)<1e-10) & (abs(momento)<1e-10) & (abs(cortante)<1e-10) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."