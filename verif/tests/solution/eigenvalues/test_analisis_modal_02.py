# -*- coding: utf-8 -*-
''' Test para comprobar el análisis modal espectral
tomado de la publicación de Andrés Sáez Pérez: «Estructuras III»
 E.T.S. de Arquitectura de Sevilla (España). '''
from __future__ import division
import xc_base
import geom
import xc

from model import fix_node_3dof
from model import predefined_spaces
from solution import predefined_solutions
from materials import typical_materials
import math

masaPorPlanta= 134.4e3
matrizMasasNodo= xc.Matrix([[masaPorPlanta,0,0],[0,0,0],[0,0,0]])
Ehorm= 200000*1e5 # Módulo elástico del hormigón.

Bbaja= 0.45 # Escuadría de los pilares.
Ibaja= 1/12.0*Bbaja**4 # Inercia de la sección.
Hbaja= 4 # Altura de la planta baja.
B1a= 0.40 # Escuadría de los pilares.
I1a= 1/12.0*B1a**4 # Inercia de la sección.
H= 3 # Altura del resto de plantas.
B3a= 0.35 # Escuadría de los pilares.
I3a= 1/12.0*B3a**4 # Inercia de la sección.


kPlBaja= 20*12*Ehorm*Ibaja/(Hbaja**3)
kPl1a= 20*12*Ehorm*I1a/(H**3)
kPl2a= kPl1a
kPl3a= 20*12*Ehorm*I3a/(H**3)
kPl4a= kPl3a

# Problem type
prueba= xc.ProblemaEF()
mdlr= prueba.getModelador

nodos= mdlr.getNodeLoader
predefined_spaces.gdls_resist_materiales2D(nodos)
nodos.defaultTag= 0;
nod0= nodos.newNodeXY(0,0)
nod0.mass= matrizMasasNodo
nod0.setProp("gdlsCoartados",xc.ID([0,1,2]))
nod1= nodos.newNodeXY(0,4) 
nod1.mass= matrizMasasNodo
nod1.setProp("gdlsCoartados",xc.ID([1,2]))
nod2= nodos.newNodeXY(0,4+3) 
nod2.mass= matrizMasasNodo
nod2.setProp("gdlsCoartados",xc.ID([1,2]))
nod3= nodos.newNodeXY(0,4+3+3) 
nod3.mass= matrizMasasNodo
nod3.setProp("gdlsCoartados",xc.ID([1,2]))
nod4= nodos.newNodeXY(0,4+3+3+3) 
nod4.mass= matrizMasasNodo
nod4.setProp("gdlsCoartados",xc.ID([1,2]))
nod5= nodos.newNodeXY(0,4+3+3+3+3) 
nod5.mass= matrizMasasNodo
nod5.setProp("gdlsCoartados",xc.ID([1,2]))
setTotal= mdlr.getSets.getSet("total")
nodos= setTotal.getNodes
for n in nodos:
  n.fix(n.getProp("gdlsCoartados"),xc.Vector([0,0,0]))

# Materials definition
sccPlBaja= typical_materials.defElasticSection2d(mdlr,"sccPlBaja",20*Bbaja*Bbaja,Ehorm,20*Ibaja)
sccPl1a= typical_materials.defElasticSection2d(mdlr,"sccPl1a",20*B1a*B1a,Ehorm,20*I1a) 
sccPl2a= typical_materials.defElasticSection2d(mdlr,"sccPl2a",20*B1a*B1a,Ehorm,20*I1a) 
sccPl3a= typical_materials.defElasticSection2d(mdlr,"sccPl3a",20*B3a*B3a,Ehorm,20*I3a) 
sccPl4a= typical_materials.defElasticSection2d(mdlr,"sccPl4a",20*B3a*B3a,Ehorm,20*I3a)


# Definimos transformaciones geométricas
trfs= mdlr.getTransfCooLoader
lin= trfs.newLinearCrdTransf2d("lin")


# Elements definition
elementos= mdlr.getElementLoader
elementos.defaultTransformation= "lin"
elementos.defaultMaterial= "sccPlBaja"
elementos.defaultTag= 1 #Tag for next element.
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([0,1]))
beam2d.h= Bbaja
elementos.defaultMaterial= "sccPl1a" 
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([1,2]))
beam2d.h= B1a
elementos.defaultMaterial= "sccPl2a" 
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([2,3]))
beam2d.h= B1a
elementos.defaultMaterial= "sccPl3a" 
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([3,4]))
beam2d.h= B3a
elementos.defaultMaterial= "sccPl4a" 
beam2d= elementos.newElement("elastic_beam_2d",xc.ID([4,5]))
beam2d.h= B3a




beta= 0.29 # Ductilidad alta.
Ki= [1.0,1.33,1.88]

# Procedimiento de solución
solu= prueba.getSoluProc
solCtrl= solu.getSoluControl


solModels= solCtrl.getModelWrapperContainer
sm= solModels.newModelWrapper("sm")


cHandler= sm.newConstraintHandler("transformation_constraint_handler")

numberer= sm.newNumberer("default_numberer")
numberer.useAlgorithm("rcm")

solMethods= solCtrl.getSoluMethodContainer
smt= solMethods.newSoluMethod("smt","sm")
solAlgo= smt.newSolutionAlgorithm("frequency_soln_algo")
integ= smt.newIntegrator("eigen_integrator",xc.Vector([1.0,1,1.0,1.0]))

soe= smt.newSystemOfEqn("sym_band_eigen_soe")
solver= soe.newSolver("sym_band_eigen_solver")

analysis= solu.newAnalysis("modal_analysis","smt","")
ac= 0.69 # Aceleración de cálculo.
T0= 0.24
T1= 0.68
meseta= 2.28

spectrum= geom.FunctionGraph1D()

spectrum.append(0.0,1.0)
spectrum.append(T0,meseta)
t=T1
while(t<2.0):
  spectrum.append(t,meseta*T1/t)
  t+=1

spectrum*=(ac)
analysis.spectrum= spectrum

analOk= analysis.analyze(5)          
periodos= analysis.getPeriods()
aceleraciones= []
sz= periodos.size()
for i in range(0,sz):
  T= periodos[i]
  aceleraciones.append(analysis.spectrum(T))

modos= analysis.getNormalizedEigenvectors()
factoresDistribucion= analysis.getDistributionFactors()



factoresParticipacionModal= analysis.getModalParticipationFactors()
masasModalesEfectivas= analysis.getEffectiveModalMasses()
masaTotal= analysis.getTotalMass()

cargaModo1= xc.Vector([0,0,0])
cargaModo2= xc.Vector([0,0,0])
cargaModo3= xc.Vector([0,0,0])


setTotal= mdlr.getSets["total"]
nodos= setTotal.getNodes
for n in nodos:
  if(n.tag>0): 
    cargaModo1+= beta*Ki[0]*n.getEquivalentStaticLoad(1,aceleraciones[0])
    cargaModo2+= beta*Ki[1]*n.getEquivalentStaticLoad(2,aceleraciones[1])
    cargaModo3+= beta*Ki[2]*n.getEquivalentStaticLoad(3,aceleraciones[2])
 

periodosTeor= xc.Vector([0.468,0.177,0.105,0.085,0.065])
ratio1= (periodos-periodosTeor).Norm()
modosEjemplo= xc.Matrix([[0.323,-0.764,0.946,0.897,-0.623],
                         [0.521,-0.941,0.378,-0.251,1.000],
                         [0.685,-0.700,-0.672,-0.907,-0.658],
                         [0.891,0.241,-1.000,1.000,0.195],
                         [1.000,1.000,0.849,-0.427,-0.042]])
resta= (modos-modosEjemplo)
ratio2= resta.Norm()
ratio3= abs(masaTotal-5*masaPorPlanta)/5/masaPorPlanta
''' Los valores de los 3 primeros factores de distribución (3 primeras columnas)
   se tomaron del ejemplo de referencia. Los otros dos (que no se dan en el
   ejemplo) son los que se obtienen del programa. ''' 
factoresDistribEjemplo= xc.Matrix([[0.419,0.295,0.148,0.0966714,0.0429946],
                         [0.676,0.363,0.059,-0.0270432,-0.0689994],
                         [0.889,0.27,-0.105,-0.0978747,0.0453662],
                         [1.157,-0.093,-0.156,0.1078,-0.0134259],
                         [1.298,-0.386,0.133,-0.0461473,0.00292086]])
resta= factoresDistribucion-factoresDistribEjemplo
ratio4= resta.Norm()
ratio5= math.sqrt((cargaModo1[0]-273523)**2+(cargaModo2[0]-31341)**2+(cargaModo3[0]-6214)**2)/273523.0

'''
print "kPlBaja= ",kPlBaja
print "kPl1a= ",kPl1a
print "kPl3a= ",kPl3a
print "periodos: ",periodos
print "aceleraciones= ",aceleraciones          
print "ratio1= ",ratio1
print "modos: ",modos
print "resta: ",resta
print "ratio2= ",ratio2
print "factoresParticipacionModal: ",factoresParticipacionModal
print "masasModalesEfectivas: ",masasModalesEfectivas
print "masaTotal: ",masaTotal
print "ratio3= ",ratio3
print "factoresDistribucion: ",factoresDistribucion
print "ratio4= ",ratio4
print "\n  carga estática equivalente modo 1: ",cargaModo1
print "  carga estática equivalente modo 2: ",cargaModo2
print "  carga estática equivalente modo 3: ",cargaModo3
print "ratio5= ",ratio5
'''


import os
fname= os.path.basename(__file__)
if((ratio1<1e-3) & (ratio2<5e-2) & (ratio3<1e-12) & (ratio4<5e-2) & (ratio5<5e-2)):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
