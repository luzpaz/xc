# -*- coding: utf-8 -*-
# Comprobación de secciones de hormigón frente a tensiones normales.

import os
from materials.xLamina import extrae_combinaciones as ec
from materials.xLamina import modelo
from materials.xLamina import calculo_comb
import geom
import math
from postprocess.reports import common_formats as fmt
import scipy

def even(number):
   return number % 2 == 0
def odd(number):
   return number % 2 != 0

# Ejecuta el análisis y la comprobación frente a tensiones normales
def xLaminaCompruebaTNComb(mdlr, nmbDiagIntSec1, nmbDiagIntSec2):
  listaCombinaciones= []
  cargas= mdlr.getLoadLoader
  casos= cargas.getLoadPatterns
  ts= casos.newTimeSeries("constant_ts","ts")
  casos.currentTimeSeries= "ts"
  execfile("/tmp/cargas.xci")
  listaCombinaciones= cargas.listaNombresLoadPatterns()

  for comb in listaCombinaciones:
    print("Resolviendo para acción: ",listaCombinaciones[i],"\n")
    resuelveCombEstatLin(comb)
    trataResultsComb(comb,nmbDiagIntSec1,nmbDiagIntSec2)

  os.system("rm -f "+"/tmp/acciones.xci")
  os.system("rm -f "+"/tmp/cargas.xci")
  xLaminaPrintTN(nmbArch) # XXX Sacar de aquí la impresión de result.

def trataResultsCombTN(mdlr, nmbComb):
  #print "Postproceso combinación: ",nmbComb
  elements= mdlr.getSets["total"].getElements
  for e in elements:
    e.getResistingForce()
    TagTmp= e.tag
    scc= e.getSection()
    Ntmp= scc.getStressResultantComponent("N")
    MyTmp= scc.getStressResultantComponent("My")
    MzTmp= scc.getStressResultantComponent("Mz")
    posEsf= geom.Pos3d(Ntmp,MyTmp,MzTmp)
    diagInt= e.getProp("diagInt")
    FCtmp= diagInt.getCapacityFactor(posEsf)
    if(FCtmp>e.getProp("FCCP")):
      e.setProp("FCCP",FCtmp) # Caso pésimo
      e.setProp("HIPCP",nmbComb)
      e.setProp("NCP",Ntmp)
      e.setProp("MyCP",MyTmp)
      e.setProp("MzCP",MzTmp)

# Imprime los resultados de la comprobación frente a tensiones normales
def xLaminaPrintTNAnsys(mdlr,nmbArchSalida, nmbSeccion1, nmbSeccion2):
  texOutput1= open("/tmp/texOutput1.tmp","w")
  texOutput1.write("Section 1\n")
  texOutput2= open("/tmp/texOutput2.tmp","w")
  texOutput2.write("Section 2\n")
  ansysOutput1= open(nmbArchSalida+".mac","w")
  ansysOutput2= open(nmbArchSalida+"esf.mac","w")
  #printCabeceraListadoFactorCapacidad("texOutput1","1 ("+ nmbSeccion1 +")")
  #printCabeceraListadoFactorCapacidad("texOutput2","2 ("+ nmbSeccion2 +")")
  fcs1= [] #Capacity factors at section 1.
  fcs2= [] #Capacity factors at section 2.
  elementos= mdlr.getSets["total"].getElements
  for e in elementos:
    eTag= e.getProp("idElem")
    FCCP= e.getProp("FCCP")
    HIPCP= e.getProp("HIPCP")
    NCP= e.getProp("NCP")
    MyCP= e.getProp("MyCP")
    MzCP= e.getProp("MzCP")
    outStr= str(eTag)+" & "+HIPCP+" & "+fmt.Esf.format(NCP/1e3)+" & "+fmt.Esf.format(MyCP/1e3)+" & "+fmt.Esf.format(MzCP/1e3)+" & "+fmt.Esf.format(FCCP)+"\\\\\n"
    ansOutStr1= "detab,"+str(math.floor(e.tag/10))+",FC1,"+str(FCCP)+"\n"
    ansOutStr2= "detab,"+str(math.floor(e.tag/10))+",N1,"+str(NCP/1e3)+"\n"
    ansOutStr3= "detab,"+str(math.floor(e.tag/10))+",My1,"+str(MyCP/1e3)+"\n"
    ansOutStr4= "detab,"+str(math.floor(e.tag/10))+",Mz1,"+str(MzCP/1e3)+"\n"
    if(odd(e.tag)):
      fcs1.append(FCCP)
      texOutput1.write(outStr)
      ansysOutput1.write(ansOutStr1)
      ansysOutput1.write(ansOutStr2)
      ansysOutput1.write(ansOutStr3)
      ansysOutput1.write(ansOutStr4)
    else:
      fcs2.append(FCCP)
      texOutput2.write(outStr)
      ansysOutput2.write(ansOutStr1)
      ansysOutput2.write(ansOutStr2)
      ansysOutput2.write(ansOutStr3)
      ansysOutput2.write(ansOutStr4)
  
  #printCierreListadoFactorCapacidad("texOutput1")
  #printCierreListadoFactorCapacidad("texOutput2")
  texOutput1.close()
  texOutput2.close()
  ansysOutput1.close()
  ansysOutput2.close()
    
  os.system("cat /tmp/texOutput1.tmp /tmp/texOutput2.tmp > "+nmbArchSalida+".tex")
    
  # os.system("rm -f "+"/tmp/acciones.xci")
  # os.system("rm -f "+"/tmp/cargas.xci")
  # os.system("rm -f "+"/tmp/elementos.xci")
  os.system("rm -f "+"/tmp/texOutput1.tmp")
  os.system("rm -f "+"/tmp/texOutput2.tmp")
  retval= [scipy.mean(fcs1),scipy.mean(fcs2)]
  return retval

def strElementProp(eTag,nmbProp,vProp):
  retval= "mdlr.getElementLoader.getElement("
  retval+= str(eTag)
  retval+= ").setProp("
  retval+= '"' + nmbProp + '"'
  retval+= ',' + str(vProp) + ")\n"
  return retval

# Imprime los resultados de la comprobación frente a tensiones normales
def xLaminaPrintTN(mdlr,nmbArchSalida):
  texOutput1= open("/tmp/texOutput1.tmp","w")
  texOutput1.write("Section 1\n")
  texOutput2= open("/tmp/texOutput2.tmp","w")
  texOutput2.write("Section 2\n")
  xcOutput= open(nmbArchSalida+".py","w")
  #printCabeceraListadoFactorCapacidad("texOutput1","1 ("+ nmbSeccion1 +")")
  #printCabeceraListadoFactorCapacidad("texOutput2","2 ("+ nmbSeccion2 +")")
  fcs1= [] #Capacity factors at section 1.
  fcs2= [] #Capacity factors at section 2.
  elementos= mdlr.getSets["total"].getElements
  for e in elementos:
    eTag= e.getProp("idElem")
    idSection= e.getProp("idSection")
    FCCP= e.getProp("FCCP")
    HIPCP= e.getProp("HIPCP")
    NCP= e.getProp("NCP")
    MyCP= e.getProp("MyCP")
    MzCP= e.getProp("MzCP")
    strOut= str(eTag)+" & "+idSection+" & "+HIPCP+" & "+fmt.Esf.format(NCP/1e3)+" & "+fmt.Esf.format(MyCP/1e3)+" & "+fmt.Esf.format(MzCP/1e3)+" & "+fmt.Esf.format(FCCP)+"\\\\\n"
    if(e.getProp("dir")==1):
      fcs1.append(FCCP)
      texOutput1.write(strOut)
      xcOutput.write(strElementProp(eTag,"FCCP1",FCCP))
      xcOutput.write(strElementProp(eTag,"NCP1",NCP/1e3))
      xcOutput.write(strElementProp(eTag,"MyCP1",MyCP/1e3))
      xcOutput.write(strElementProp(eTag,"MzCP1",MzCP/1e3))
    else:
      fcs2.append(FCCP)
      texOutput2.write(strOut)
      xcOutput.write(strElementProp(eTag,"FCCP2",FCCP))
      xcOutput.write(strElementProp(eTag,"NCP2",NCP/1e3))
      xcOutput.write(strElementProp(eTag,"MyCP2",MyCP/1e3))
      xcOutput.write(strElementProp(eTag,"MzCP2",MzCP/1e3))

  #printCierreListadoFactorCapacidad("texOutput1")
  #printCierreListadoFactorCapacidad("texOutput2")
  texOutput1.close()
  texOutput2.close()
  xcOutput.close()
    
  os.system("cat /tmp/texOutput1.tmp /tmp/texOutput2.tmp > "+nmbArchSalida+".tex")
    
  # os.system("rm -f "+"/tmp/acciones.xci")
  # os.system("rm -f "+"/tmp/cargas.xci")
  # os.system("rm -f "+"/tmp/elementos.xci")
  os.system("rm -f "+"/tmp/texOutput1.tmp")
  os.system("rm -f "+"/tmp/texOutput2.tmp")
  retval= [scipy.mean(fcs1),scipy.mean(fcs2)]
  return retval

'''
 Lanza la comprobación de tensiones normales en una lámina
    cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
    con los materiales que se definen en el archivo nmbArchMateriales,
    las características de las secciones que se definen en los registros
    datosScc1 y datosScc2, las combinaciones definidas en el archivo
    nmbArchDefHipELU e imprime los resultados en archivos con
    el nombre nmbArchTN.*
'''
def lanzaCalculoTNFromAnnsysData(nmbArch, datosScc1, datosScc2, nmbArchDefHipELU):
  extraeDatosLST(nmbArch+".lst") 
  xLaminaConstruyeModeloFicticio(datosScc1,datosScc2)
  nmbDiagIntSec1= "diagInt"+datosScc1.nmbSeccion
  nmbDiagIntSec2= "diagInt"+datosScc2.nmbSeccion
  xLaminaCalculaCombEstatLin(nmbArchDefHipELU,nmbDiagIntSec1,nmbDiagIntSec2)
  meanFCs= xLaminaPrintTN(mdlr,nmbArch+"TN",datosScc1.nmbSeccion,datosScc2.nmbSeccion)
  return meanFCs

'''
 Lanza la comprobación de tensiones normales en una lámina
    cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
    con los materiales que se definen en el archivo nmbArchMateriales,
    las características de las secciones que se definen en los registros
    datosScc1 y datosScc2, las combinaciones definidas en el archivo
    nmbArchDefHipELU e imprime los resultados en archivos con
    el nombre nmbArchTN.*
'''
def lanzaCalculoTNFromXCData(mdlr,analysis,nmbArchCsv,nmbArchSalida, mapSectionsForEveryElement,mapSectionsDefinition, mapInteractionDiagrams):
  ec.extraeDatos(mdlr,nmbArchCsv, mapSectionsForEveryElement,mapSectionsDefinition, mapInteractionDiagrams)
  #modelo.xLaminaConstruyeModeloFicticio(mdlr,datosScc1,datosScc2)
  #nmbDiagIntSec1= "diagInt"+datosScc1.nmbSeccion
  #nmbDiagIntSec2= "diagInt"+datosScc2.nmbSeccion
  calculo_comb.xLaminaCalculaCombEstatLin(mdlr,analysis,trataResultsCombTN)
  meanFCs= xLaminaPrintTN(mdlr,nmbArchSalida)
  return meanFCs

