# -*- coding: utf-8 -*-
''' Test de funcionamiento de la comprobación a fisuración de una sección de hormigón armado.
   Los resultados se comparan con los obtenidos del «Prontuario informático del hormigón armado». '''

# Macros
import xc_base
import geom
import xc
from materials.ehe import auxEHE
from misc import banco_pruebas_scc3d
from solution import predefined_solutions # Procedimiento de solución


from materials.ehe import hormigonesEHE
from materials.ehe import EHE_reinforcing_steel
from materials.ehe import fisuracionEHE
from model import fix_node_6dof

ancho= 0.25 # Ancho de la sección expresado en metros.
canto= 0.25 # Canto de la sección expresado en metros.
recub= 0.025 # Recubrimiento de la sección expresado en metros.
areaFi12=1.13e-4

NDato= -100.0 # Axil para comprobar fisuración.
MyDato= 0.0 # Momento para comprobar fisuración.

prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
# Materials definition
tagHA25= hormigonesEHE.HA25.defDiagK(mdlr)
tagB500S= EHE_reinforcing_steel.B500S.defDiagK(mdlr)

geomSecHA= mdlr.getMaterialLoader.newSectionGeometry("geomSecHA")
regiones= geomSecHA.getRegions
hormigon= regiones.newQuadRegion(hormigonesEHE.HA25.nmbDiagK)
hormigon.nDivIJ= 10
hormigon.nDivJK= 10
hormigon.pMin= geom.Pos2d(-ancho/2.0,-canto/2.0)
hormigon.pMax= geom.Pos2d(ancho/2.0,canto/2.0)
armaduras= geomSecHA.getReinfLayers
armaduraInf= armaduras.newStraightReinfLayer(EHE_reinforcing_steel.B500S.nmbDiagK)
armaduraInf.numReinfBars= 2
armaduraInf.barArea= areaFi12
armaduraInf.p1= geom.Pos2d(recub-ancho/2.0,recub-canto/2.0) # Armadura inferior.
armaduraInf.p2= geom.Pos2d(ancho/2.0-recub,recub-canto/2.0)
armaduraSup= armaduras.newStraightReinfLayer(EHE_reinforcing_steel.B500S.nmbDiagK)
armaduraSup.numReinfBars= 2
armaduraSup.barArea= areaFi12
armaduraSup.p1= geom.Pos2d(recub-ancho/2.0,canto/2.0-recub) # Armadura superior.
armaduraSup.p2= geom.Pos2d(ancho/2.0-recub,canto/2.0-recub)

materiales= mdlr.getMaterialLoader
secHA= materiales.newMaterial("fiber_section_3d","secHA")
fiberSectionRepr= secHA.getFiberSectionRepr()
fiberSectionRepr.setGeomNamed("geomSecHA")
secHA.setupFibers()

banco_pruebas_scc3d.modeloSecc3d(mdlr,"secHA")

# Constraints
coacciones= mdlr.getConstraintLoader

fix_node_6dof.fixNode6DOF(coacciones,1)
fix_node_6dof.Nodo6DOFMovXGiroYLibres(coacciones,2)

# Loads definition
cargas= mdlr.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(2,xc.Vector([NDato,0,0,0,MyDato,0]))

#We add the load case to domain.
casos.addToDomain("0")



# Procedimiento de solución
analisis= predefined_solutions.simple_newton_raphson(prueba)
analOk= analisis.analyze(10)

secHAParamsFis= fisuracionEHE.ParamsFisuracionEHE()


elementos= mdlr.getElementLoader
ele1= elementos.getElement(1)
scc= ele1.getSection()
secHAParamsFis.calcApertCaracFis(scc,hormigonesEHE.HA25.tagDiagK,EHE_reinforcing_steel.B500S.tagDiagK,hormigonesEHE.HA25.fctm())

ratio1= secHAParamsFis.Wk


''' 
    print "ratio1= ",ratio1
'''

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-5):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."