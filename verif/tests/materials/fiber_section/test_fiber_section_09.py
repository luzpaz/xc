# -*- coding: utf-8 -*-
''' Test de funcionamiento de una sección de hormigón armado.
   los resultados se comparan con los obtenidos del prontuario.
   informático del hormigón estructural (Cátedra de hormigón de la ETSICCP-IECA
   UPM). '''

# prueba.logFileName= "/tmp/borrar.log"  #Para no imprimir mensajes de advertencia.
# Macros
import xc_base
import geom
import xc
from materials.ehe import auxEHE
from misc import banco_pruebas_scc3d


from materials.ehe import hormigonesEHE
from materials.ehe import EHE_reinforcing_steel
from materials.fiber_section import creaSetsFibras
from model import fix_node_6dof
from solution import predefined_solutions

# Coeficientes de seguridad.
gammac= 1.5 # Coeficiente de minoración de la resistencia del hormigón.
gammas= 1.15 # Coeficiente de minoración de la resistencia del acero.

MzDato= 20e3
NDato= -1345e3
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador
# Materials definition
tag= EHE_reinforcing_steel.B500S.defDiagD(mdlr)
tag= hormigonesEHE.HA25.defDiagD(mdlr)

import os
pth= os.path.dirname(__file__)
if(not pth):
  pth= "."
#print "pth= ", pth
execfile(pth+"/secc_hormigon_01.py")
materiales= mdlr.getMaterialLoader
secHA= materiales.newMaterial("fiber_section_3d","secHA")
fiberSectionRepr= secHA.getFiberSectionRepr()
fiberSectionRepr.setGeomNamed("geomSecHormigon01")
secHA.setupFibers()

banco_pruebas_scc3d.modeloSecc3d(mdlr,"secHA")

# Constraints
coacciones= mdlr.getConstraintLoader

fix_node_6dof.fixNode6DOF(coacciones,1)
fix_node_6dof.Nodo6DOFMovXGiroZLibres(coacciones,2)

# Loads definition
cargas= mdlr.getLoadLoader

casos= cargas.getLoadPatterns

#Load modulation.
ts= casos.newTimeSeries("constant_ts","ts")
casos.currentTimeSeries= "ts"
#Load case definition
lp0= casos.newLoadPattern("default","0")
lp0.newNodalLoad(2,xc.Vector([NDato,0,0,0,0,MzDato]))

#We add the load case to domain.
casos.addToDomain("0")


# Procedimiento de solución
analisis= predefined_solutions.simple_newton_raphson(prueba)
analOk= analisis.analyze(10)

nodos= mdlr.getNodeLoader
nodos.calculateNodalReactions(True)

RN= nodos.getNode(1).getReaction[0] 
RM= nodos.getNode(1).getReaction[5] 
RN2= nodos.getNode(2).getReaction[0] 

elementos= mdlr.getElementLoader
ele1= elementos.getElement(1)
scc= ele1.getSection()
esfN= scc.getStressResultantComponent("N")
esfMy= scc.getStressResultantComponent("My")
esfMz= scc.getStressResultantComponent("Mz")
defMz= scc.getSectionDeformationByName("defMz")
defN= scc.getSectionDeformationByName("defN")
fibrasHormigon= creaSetsFibras.FiberSet(scc,"hormigon",hormigonesEHE.HA25.tagDiagD)
fibraCEpsMin= fibrasHormigon.getFiberWithMinStrain()
epsCMin= fibraCEpsMin.getMaterial().getStrain() # Deformación mínima en el hormigón.
fibraCEpsMax= fibrasHormigon.getFiberWithMaxStrain()
epsCMax= fibraCEpsMax.getMaterial().getStrain() # Deformación máxima en el hormigón.

fibrasArmadura= creaSetsFibras.FiberSet(scc,"armadura",EHE_reinforcing_steel.B500S.tagDiagD)
fibraSEpsMax= fibrasArmadura.getFiberWithMaxStrain()
epsSMax= fibraSEpsMax.getMaterial().getStrain() # Deformación máxima en el acero.



from materials import regimenSeccion
from materials.ehe import comprobTnEHE
tipoSolic= regimenSeccion.tipoSolicitacion(epsCMin,epsSMax)
strTipoSolic= regimenSeccion.strTipoSolicitacion(tipoSolic)
cumpleFT= comprobTnEHE.cumpleCompresionCompuesta(epsCMin,epsCMax)
aprovSecc= comprobTnEHE.aprovCompresionCompuesta(epsCMin,epsSMax)

ratio1= (RM+MzDato)/MzDato
ratio2= (esfMz-MzDato)/MzDato
ratio3= (esfN-NDato)
ratio4= (cumpleFT-1)
ratio5= (RN+NDato)/NDato
epsSMaxTeor= 1.2e-3
ratio6= (epsSMax+epsSMaxTeor)/epsSMaxTeor

'''
print "ratio1= ", ratio1
print "ratio2= ", ratio2
print "ratio3= ", ratio3
print "ratio4= ", ratio4
print "ratio5= ", ratio5

print "Deformación mínima en el hormigón: ",(epsCMin)
print "Deformación máxima en el hormigón: ",(epsCMax)
print "Deformación máxima en la armadura: ",(epsSMax)
print "Tipo solicitación: ",strTipoSolic," (",(tipoSolic),") \n"
print "Cumple a ",strTipoSolic,": ",(cumpleFT)
print "Aprovechamiento a ",strTipoSolic,": ",(aprovSecc)
print "RN= ",(RN/1e3)
print "RN2= ",(RN2/1e3)
print "N= ",(esfN/1e3)
print "My= ",(esfMy/1e3)
print "Mz= ",(esfMz/1e3)
print "defMz= ",(defMz)
print "defN= ",(defN)

print "               Cumple ratio1: ",abs(ratio1)<1e-9
print "               Cumple ratio2: ",abs(ratio2)<1e-9
print "               Cumple ratio3: ",abs(ratio3)<1e-9
print "               Cumple ratio4: ",abs(ratio4)<1e-6
print "               Cumple ratio5: ",abs(ratio5)<1e-9
print "               Cumple RN2: ",abs(RN2)<1e-9
print "               Cumple esfMy: ",abs(esfMy)<1e-10
print "               Cumple tipoSolic: ",(tipoSolic == 3)
print "epsSMax= ",epsSMax
print "epsSMaxTeor= ",epsSMaxTeor
print "ratio6= ", ratio6
print "               Cumple ratio6: ",abs(ratio6)<0.01
 '''

import os
fname= os.path.basename(__file__)
if (abs(ratio1)<1e-9) & (abs(ratio2)<1e-9) & (abs(ratio3)<1e-9) & (abs(ratio5)<1e-9) & (abs(RN2)<1e-9) & (abs(esfMy)<1e-10) & (tipoSolic == 3) & (abs(ratio4)<1e-6) & (abs(ratio6)<0.01) & (analOk == 0.0) :
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."