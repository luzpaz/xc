# -*- coding: utf-8 -*-
# Comprobación de secciones de hormigón frente a cortante.

from materials.xLamina import extrae_combinaciones as ec
from materials.xLamina import calculo_comb
from materials.ehe import comprobVEHE08
from materials.ehe import cortanteEHE
from materials.ehe import torsionEHE
from materials.sia262 import shearSIA262
import geom
import math
from postprocess.reports import common_formats as fmt
import scipy
import os

 
def xLaminaPrintVAnsys(nmbArchSalida, nmbSeccion1, nmbSeccion2):
  # Imprime los resultados de la comprobación frente a cortante
  texOutput1= open("/tmp/texOutput1.tmp","w")
  texOutput2= open("/tmp/texOutput2.tmp","w")
  ansysOutput1= open(nmbArchSalida+".mac","w")
  ansysOutput2= open(nmbArchSalida+"esf.mac","w")
  printCabeceraListadoCortante("texOutput1","1 ("+ nmbSeccion1 +")")
  printCabeceraListadoCortante("texOutput2","2 ("+ nmbSeccion2 +")")
  for e in elementos:
    if(odd(e.tag)):
      texOutput1.write(floor(tag/10)," & ",HIPCP," & ",format(NCP/1e3,"%5.2f")," & ",format(VyCP/1e3,"%5.2f")," & ",format(VzCP/1e3,"%5.2f")," & ",format(Vu1CP/1e3,"%5.2f")," & ",format(rad2deg(thetaCP),"%4.1f")," & ",format(VcuCP/1e3,"%5.2f")," & ",format(VsuCP/1e3,"%5.2f")," & ",format(Vu2CP/1e3,"%5.2f")," & ",format(VuCP/1e3,"%5.2f")," & ",format(FCCP,"%3.2f"),"\\\\\n")
      ansysOutput1.write("detab,",floor(tag/10),",FCV1,",FCCP,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",NV1,",NCP/1e3,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",Vy1,",VyCP/1e3,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",Vz1,",VzCP/1e3,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",Vu1,",VuCP/1e3,"\n")
    else:
      texOutput2.write(floor(tag/10)," & ",HIPCP," & ",format(NCP/1e3,"%5.2f")," & ",format(VyCP/1e3,"%5.2f")," & ",format(VzCP/1e3,"%5.2f")," & ",format(Vu1CP/1e3,"%5.2f")," & ",format(rad2deg(thetaCP),"%4.1f")," & ",format(VcuCP/1e3,"%5.2f")," & ",format(VsuCP/1e3,"%5.2f")," & ",format(Vu2CP/1e3,"%5.2f")," & ",format(VuCP/1e3,"%5.2f")," & ",format(FCCP,"%3.2f"),"\\\\\n")
      ansysOutput1.write("detab,",floor(tag/10),",FCV2,",FCCP,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",NV2,",NCP/1e3,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",Vy2,",VyCP/1e3,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",Vz2,",VzCP/1e3,"\n")
      ansysOutput2.write("detab,",floor(tag/10),",Vu2,",VuCP/1e3,"\n")

  printCierreListadoCortante("texOutput1")
  printCierreListadoCortante("texOutput2")
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

def strElementProp(eTag,nmbProp,vProp):
  retval= "mdlr.getElementLoader.getElement("
  retval+= str(eTag)
  retval+= ").setProp("
  retval+= '"' + nmbProp + '"'
  retval+= ',' + str(vProp) + ")\n"
  return retval

def xLaminaPrintV(mdlr, nmbArchSalida):
  # Imprime los resultados de la comprobación frente a cortante
  texOutput1= open("/tmp/texOutput1.tmp","w")
  texOutput2= open("/tmp/texOutput2.tmp","w")
  xcOutput= open(nmbArchSalida+".py","w")
  #printCabeceraListadoCortante("texOutput1","1 ("+ nmbSeccion1 +")")
  #printCabeceraListadoCortante("texOutput2","2 ("+ nmbSeccion2 +")")
  #strHeader= "eTag & idSection & HIPCP & NCP kN & MyCP kN & MzCP kN & Vu1CP kN & VsuCP kN & Vu2CP kN & VuCP kN & FCCP \\\\\n"
  strHeader= "eTag & idSection & HIPCP & NCP kN & MyCP kN m/m & MzCP kN m/m & MuCP kN m/m & VyCP kN & VzCP kN & VuCP kN & FCCP \\\\\n"
  texOutput1.write(strHeader)
  texOutput2.write(strHeader)
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
    MuCP= e.getProp("MuCP")
    VyCP= e.getProp("VyCP")
    VzCP= e.getProp("VzCP")
    VcuCP= e.getProp("VcuCP")
    VsuCP= e.getProp("VsuCP")
    VuCP= e.getProp("VuCP")
    #strOut= str(eTag)+" & "+idSection+" & "+HIPCP+" & "+fmt.Esf.format(NCP/1e3)+" & "+fmt.Esf.format(MyCP/1e3)+" & "+fmt.Esf.format(MzCP/1e3)+" & "+fmt.Esf.format(Vu1CP/1e3)+" & "+fmt.Esf.format(VsuCP/1e3)+" & "+fmt.Esf.format(Vu2CP/1e3)+" & "+fmt.Esf.format(VuCP/1e3)+" & "+fmt.Esf.format(FCCP)+"\\\\\n"
    strOut= str(eTag)+" & " +idSection+" & "+ HIPCP +" & "+fmt.Esf.format(NCP/1e3)+" & "+fmt.Esf.format(MyCP/1e3)+" & "+fmt.Esf.format(MzCP/1e3)+" & "+fmt.Esf.format(MuCP/1e3)+" & "+fmt.Esf.format(VyCP/1e3)+" & "+fmt.Esf.format(VzCP/1e3)+" & "+fmt.Esf.format(VuCP/1e3)+" & "+fmt.Esf.format(FCCP)+"\\\\\n"
    if(e.getProp("dir")==1):
      fcs1.append(FCCP)
      texOutput1.write(strOut)
      xcOutput.write(strElementProp(eTag,"FCCP1",FCCP))
      xcOutput.write(strElementProp(eTag,"NCP1",NCP/1e3))
      xcOutput.write(strElementProp(eTag,"MyCP1",MyCP/1e3))
      xcOutput.write(strElementProp(eTag,"MzCP1",MzCP/1e3))
      xcOutput.write(strElementProp(eTag,"MuCP1",MuCP/1e3))
      xcOutput.write(strElementProp(eTag,"VyCP1",VyCP/1e3))
      xcOutput.write(strElementProp(eTag,"VzCP1",VzCP/1e3))
      xcOutput.write(strElementProp(eTag,"VcuCP1",VcuCP/1e3))
      xcOutput.write(strElementProp(eTag,"VsuCP1",VsuCP/1e3))
      xcOutput.write(strElementProp(eTag,"VuCP1",VuCP/1e3))
    else:
      fcs2.append(FCCP)
      texOutput2.write(strOut)
      xcOutput.write(strElementProp(eTag,"FCCP2",FCCP))
      xcOutput.write(strElementProp(eTag,"NCP2",NCP/1e3))
      xcOutput.write(strElementProp(eTag,"MyCP2",MyCP/1e3))
      xcOutput.write(strElementProp(eTag,"MzCP2",MzCP/1e3))
      xcOutput.write(strElementProp(eTag,"MuCP2",MuCP/1e3))
      xcOutput.write(strElementProp(eTag,"VyCP2",VyCP/1e3))
      xcOutput.write(strElementProp(eTag,"VzCP2",VzCP/1e3))
      xcOutput.write(strElementProp(eTag,"VcuCP2",VcuCP/1e3))
      xcOutput.write(strElementProp(eTag,"VsuCP2",VsuCP/1e3))
      xcOutput.write(strElementProp(eTag,"VuCP2",VuCP/1e3))
  #printCierreListadoCortante("texOutput1")
  #printCierreListadoCortante("texOutput2")
  texOutput1.close()
  texOutput2.close()
  xcOutput.close()
    
  os.system("cat /tmp/texOutput1.tmp /tmp/texOutput2.tmp > "+nmbArchSalida+".tex")
    
  # os.sys("rm -f "+"/tmp/acciones.xci")
  # os.sys("rm -f "+"/tmp/cargas.xci")
  # os.sys("rm -f "+"/tmp/elementos.xci")
  os.system("rm -f "+"/tmp/texOutput1.tmp")
  os.system("rm -f "+"/tmp/texOutput2.tmp")
  retval= [scipy.mean(fcs1),scipy.mean(fcs2)]
  return retval


def lanzaCalculoVFromAnsysData(nmbArch, nmbRegDatosScc1, nmbRegDatosScc2, nmbArchDefHipELU):
  '''
   Lanza la comprobación de cortante en una lámina
      cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
      con los materiales que se definen en el archivo nmbArchMateriales,
      las características de las secciones que se definen en los registros
      datosScc1 y datosScc2, las combinaciones definidas en el archivo
      nmbArchDefHipELU e imprime los resultados en archivos con
      el nombre nmbArchTN.*
  '''
  extraeDatosLST(nmbArch+".lst")
  xLaminaConstruyeModeloFibras(nmbRegDatosScc1,nmbRegDatosScc2)
  xLaminaCalculaCombEstatNoLin(nmbArchDefHipELU)
  xLaminaPrintV(nmbArch+"V",deref(nmbRegDatosScc1).nmbSeccion,deref(nmbRegDatosScc2).nmbSeccion)

def lanzaCalculoV(mdlr,analysis,nmbArchCsv,nmbArchSalida, mapSectionsForEveryElement,mapSectionsDefinition, mapInteractionDiagrams,trataResultsCombV):
  '''
   Lanza la comprobación de cortante en una lámina
      cuyos esfuerzos se dan en el archivo de nombre nmbArch.lst
      con los materiales que se definen en el archivo nmbArchMateriales,
      las características de las secciones que se definen en los registros
      datosScc1 y datosScc2, las combinaciones definidas en el archivo
      nmbArchDefHipELU e imprime los resultados en archivos con
      el nombre nmbArchTN.*
  '''
  elems= ec.extraeDatos(mdlr,nmbArchCsv, mapSectionsForEveryElement,mapSectionsDefinition, mapInteractionDiagrams)
  #cortanteEHE.defVarsControlVEHE(elems)
  shearSIA262.defVarsControlVSIA262(elems)
  calculo_comb.xLaminaCalculaComb(mdlr,analysis,trataResultsCombV)
  meanFCs= xLaminaPrintV(mdlr,nmbArchSalida)
  return meanFCs
