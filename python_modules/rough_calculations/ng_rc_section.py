# -*- coding: utf-8 -*-

__author__= "Luis C. Pérez Tato (LCPT)"
__copyright__= "Copyright 2016, LCPT"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com"

from rough_calculations import ng_rebar_def
from materials.sia262 import concreteSIA262
from materials.sia262 import steelSIA262
from materials.sia262 import minimal_reinforcement
from postprocess.reports import common_formats as fmt
from miscUtils import LogMessages as lmsg


class RCSection(object):
  tensionRebars= None
  beton= concreteSIA262.c25_30
  exigeanceFisuration= "B"
  b= 0.25
  h= 0.25
  def __init__(self,tensionRebars,beton,exigFis,b,h):
    self.tensionRebars= tensionRebars
    self.beton= beton
    self.exigeanceFisuration= exigFis
    self.b= b
    self.h= h
    self.stressLimitUnderPermanentLoads= 230e6
  def setArmature(self,tensionRebars):
    self.tensionRebars= tensionRebars
  def getAsMinFlexion(self):
    return self.tensionRebars.getAsMinFlexion(self.beton,self.exigeanceFisuration,self.h)
  def getAsMinTraction(self):
    return self.tensionRebars.getAsMinTraction(self.beton,self.exigeanceFisuration,self.h)
  def getMR(self):
    return self.tensionRebars.getMR(self.beton,self.b,self.h)
  def getVR(self,Nd,Md):
    return self.tensionRebars.getVR(self.beton,Nd,Md,self.b,self.h)
  def writeResultFlexion(self,outputFile,Nd,Md,Vd):
    famArm= self.tensionRebars
    beton= self.beton
    AsMin= self.getAsMinFlexion()
    ancrage= famArm.getBasicAnchorageLength(beton)
    outputFile.write("  Dimensions coupe; b= "+ fmt.Longs.format(self.b)+ "m, h= "+ fmt.Longs.format(self.h)+ "m\\\\\n")
    ng_rebar_def.writeRebars(outputFile,beton,famArm,AsMin)
    if(abs(Md)>0):
      MR= self.getMR()
      outputFile.write("  Verif. en flexion: Md= "+ fmt.Esf.format(Md/1e3)+ " kN m, MR= "+ fmt.Esf.format(MR/1e3)+ "kN m")
      ng_rebar_def.writeF(outputFile,"  F(M)", MR/Md)
    if(abs(Vd)>0):
      VR= self.getVR(Nd,Md)
      outputFile.write("  Vérif. eff. tranchant: Vd= "+ fmt.Esf.format(Vd/1e3)+ "kN,  VR= "+ fmt.Esf.format(VR/1e3)+ "kN")
      ng_rebar_def.writeF(outputFile,"  F(V)",VR/Vd)
  def writeResultTraction(self,outputFile,Nd):
    famArm= self.tensionRebars
    beton= self.beton
    AsMin= self.getAsMinTraction()/2
    ancrage= famArm.getBasicAnchorageLength(beton)
    ng_rebar_def.writeRebars(outputFile,beton,famArm,AsMin)
    if(abs(Nd)>0):
      lmsg.error("ERROR; tension not implemented.")
  def writeResultCompression(self,outputFile,Nd,AsTrsv):
    ''' Results for compressed rebars.

    :param AsTrsv: Rebar area in transverse direction.
     '''
    famArm= self.tensionRebars #Even if they're not in tension...
    beton= self.beton
    AsMin= 0.2*AsTrsv # 20% of the transversal area.
    ng_rebar_def.writeRebars(outputFile,beton,famArm,AsMin)
    if(abs(Nd)!=0.0):
      lmsg.error("ERROR; not implemented.")
      
  def writeResultStress(self,outputFile,M):
    '''Cheking of stresses under permanent loads (SIA 262 fig. 31)'''
    beton= self.beton
    if(abs(M)>0):
      stress= M/(0.9*self.h*self.tensionRebars.getAs())
      outputFile.write("  Verif. contraintes: M= "+ fmt.Esf.format(M/1e3)+ " kN m, $\sigma_s$= "+ fmt.Esf.format(stress/1e6)+ " MPa\\\\\n")
      outputFile.write("    $\sigma_{lim}$= "+ fmt.Esf.format(self.stressLimitUnderPermanentLoads/1e6)+ " MPa")
      ng_rebar_def.writeF(outputFile,"  F($\sigma_s$)", self.stressLimitUnderPermanentLoads/stress)
