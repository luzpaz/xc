# -*- coding: utf-8 -*-
# home made test
KX= 1000 # Constante del muelle
KY= 2000 # Constante del muelle
l= 1e-7 # Distancia entre nodos
FX= 1 # Magnitud de la fuerza
FY= 2 # Magnitud de la fuerza

import xc_base
import geom
import xc
from solution import predefined_solutions
from model import predefined_spaces
from model import fix_node_3dof
from materials import typical_materials

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
nodos= mdlr.getNodeLoader
predefined_spaces.gdls_elasticidad2D(nodos)

# Model definition

nodos.defaultTag= 1 #First node number.
nod= nodos.newNodeXY(1,1)
nod= nodos.newNodeXY(1,1)

# Materials definition
kx= typical_materials.defElasticMaterial(mdlr,"kx",KX)
ky= typical_materials.defElasticMaterial(mdlr,"ky",KY)


''' Se definen nodos en los puntos de aplicación de
    la carga. Puesto que no se van a determinar tensiones
    se emplea una sección arbitraria de área unidad '''
    
# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultMaterial= "kx"
elementos.dimElem= 2
elementos.defaultTag= 1
elem= elementos.newElement("zero_length",xc.ID([1,2]))
elem.clearMaterials()
elem.setMaterial(0,"kx")
elem.setMaterial(1,"ky")

''' 
print "Tipo elemento: ",getTipoElemento
print "Núm. materiales: ",getNumMateriales
print "Vector x local: ",getVectorI
print "Vector y local: ",getVectorJ
print "Vector z local: ",getVectorK
print "dimensión: ",getDim
print "filas trf: ",getTrf.nrows
print "cols. trf: ",getTrf.ncols
print "trf[0]: ","getTrf[0,0]"," ","getTrf[0,1]"," ","getTrf[0,2]"
print "trf[1]: ","getTrf[1,0]"," ","getTrf[1,1]"," ","getTrf[1,2]"
print "trf[2]: ","getTrf[2,0]"," ","getTrf[2,1]"," ","getTrf[2,2]"
   '''

    
# Constraints
coacciones= mdlr.getConstraintLoader
#
spc= coacciones.newSPConstraint(1,0,0.0) # Nodo 1
spc= coacciones.newSPConstraint(1,1,0.0)


# Loads definition
cargas= mdlr.getLoadLoader
casos= cargas.getLoadPatterns
#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(2,xc.Vector([FX,FY]))
#We add the load case to domain.
casos.addToDomain("0")

# Solution
analisis= predefined_solutions.simple_static_linear(prueba)
result= analisis.analyze(1)



nodos.calculateNodalReactions(True)
nodos= mdlr.getNodeLoader
nod2= nodos.getNode(2)
deltax= nod2.getDisp[0]
deltay= nod2.getDisp[1] 
nod1= nodos.getNode(1)
RX= nod1.getReaction[0]
RY= nod1.getReaction[1] 

elementos= mdlr.getElementLoader

elem1= elementos.getElement(1)
elem1.getResistingForce()

ratio1= -RX/FX
ratio2= (KX*deltax)/FX
ratio3= -RY/FY
ratio4= (KY*deltay)/FY

''' 
print "RX= ",RX
print "dx= ",deltax
print "RY= ",RY
print "dy= ",deltay
print "ratio1= ",(ratio1)
print "ratio2= ",(ratio2)
print "ratio3= ",(ratio3)
print "ratio4= ",(ratio4)
   '''
  
import os
fname= os.path.basename(__file__)
if (abs(ratio1-1.0)<1e-5) & (abs(ratio2-1.0)<1e-5) & (abs(ratio3-1.0)<1e-5) & (abs(ratio4-1.0)<1e-5) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."