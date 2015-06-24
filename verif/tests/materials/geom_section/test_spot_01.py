# -*- coding: utf-8 -*-
# Home made test

import xc_base
import geom
import xc
from model import predefined_spaces
from materials import typical_materials
import math

fy= 2600 # Tensión de cedencia del material expresada en kp/cm2.
E= 2.1e6 # Módulo de Young del material en kp/cm2.
xA= 1/3.0
yA= 3/4.0
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
epp= typical_materials.defElasticPPMaterial(mdlr,"epp",E,fy,-fy)
geomPrueba= mdlr.getMaterialLoader.newSectionGeometry("geomPrueba")
geomPrueba.tagSpot= 1
spot1= geomPrueba.newSpot(geom.Pos2d(xA,yA))
x1= spot1.pos.x
y1= spot1.pos.y
spot2= geomPrueba.newSpot(geom.Pos2d(0,0))
dist= geomPrueba.distSpots(1,2)

''' 
             \for_each_spot

                 print codigo,",",pos.x,",",pos.y
'''
ratio1= ((xA-x1)/xA)
ratio2= ((yA-y1)/yA)
ratio3= (dist-math.sqrt((x1)**2+(y1)**2))
''' 
print "ratio1= ",ratio1
print "ratio2= ",ratio2
print "ratio3= ",ratio3
 '''

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-12) & (abs(ratio2)<1e-12) & (abs(ratio3)<1e-12):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."