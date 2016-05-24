# -*- coding: utf-8 -*-

__author__= "Luis C. Pérez Tato (LCPT) and Ana Ortega (AOO)"
__copyright__= "Copyright 2015, LCPT and AOO"
__license__= "GPL"
__version__= "3.0"
__email__= "l.pereztato@gmail.com" "ana.Ortega.Ort@gmail.com"

import xc_base
import geom
import xc

# Typical material definition.


def defElasticMaterial(preprocessor,name,E):
  '''Constructs an elastic uniaxial material.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            tangent in the stress-strain diagram
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_material",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  return retval

def defElasticPPMaterial(preprocessor,name,E,fyp,fyn):
  '''Constructs an elastic perfectly-plastic uniaxial material.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            tangent in the elastic zone of the stress-strain diagram
    fyp:          stress at which material reaches plastic state in tension
    fyn:          stress at which material reaches plastic state in compression
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elasticpp_material",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.fyp= fyp
  retval.fyn= fyn
  return retval

def defElastNoTensMaterial(preprocessor,name,E):
  '''Constructs a uniaxial elastic - no tension material.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            tangent in the elastic zone of the stress-strain diagram
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elast_no_trac_material",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  return retval

#Cable material.
def defCableMaterial(preprocessor,name,E,prestress,rho):
  '''Constructs a uniaxial bilinear prestressed material. 
  The stress strain ranges from slack (large strain at zero stress) 
  to taught (linear with modulus E).
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            elastic modulus
    prestress:    prestress
    rho:          effective self weight (gravity component of weight per 
                  volume transverse to the cable)
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("cable_material",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.prestress= prestress
  retval.rho= rho
  return retval


#Steel 01.
def defSteel01(preprocessor,name,E,fy,b):
  '''Constructs a uniaxial bilinear steel material object with kinematic hardening
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            initial elastic tangent 
    fy:           yield strength 
    b:            strain-hardening ratio: ratio between post-yield tangent
                  and initial elastic tangent
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("steel01",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.fy= fy
  retval.b= b
  return retval

#Steel 02.
def defSteel02(preprocessor,name,E,fy,b,initialStress):
  '''Constructs a uniaxial bilinear Giuffre-Menegotto-Pinto steel material with 
  isotropic strain hardening
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            initial elastic tangent 
    fy:           yield strength 
    b:            strain-hardening ratio: ratio between post-yield tangent
                  and initial elastic tangent
    initialStress:initial stress
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("steel02",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.fy= fy
  retval.b= b
  retval.initialStress= initialStress
  return retval

#Concrete 01.
def defConcrete01(preprocessor,name,epsc0,fpc,fpcu,epscu):
  '''Constructs a uniaxial Kent-Scott-Park concrete material object 
  with degraded linear unloading/reloading stiffness according to 
  the work of Karsan-Jirsa and no tensile strength
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    epsc0:        concrete strain at maximum strength 
    fpc:          concrete compressive strength at 28 days 
                  (compression is negative)
    fpcu:         concrete crushing strength 
    epscu:        concrete strain at crushing strength 
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("concrete01_material",name)
  retval= materiales.getMaterial(name)
  retval.epsc0= epsc0
  retval.fpc= fpc
  retval.fpcu= fpcu
  retval.epscu= epscu
  return retval


#Elastic section 2d.
def defElasticSection2d(preprocessor,name,A,E,I):
  '''Constructs an elastic section appropiate for 2D beam analysis.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    A:            cross-sectional area of the section
    E:            Young’s modulus of material
    I:            second moment of area about the local z-axis
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_section_2d",name)
  retval= materiales.getMaterial(name)
  retval.sectionProperties.A= A
  retval.sectionProperties.E= E
  retval.sectionProperties.I= I
  return retval

#Elastic shear section 2d.
def defElasticShearSection2d(preprocessor,name,A,E,G,I,alpha):
  '''Constructs an elastic section appropiate for 2D beam analysis, 
  including shear deformations.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    A:            cross-sectional area of the section
    E:            Young’s modulus of the material
    G:            Shear modulus of the material          
    I:            second moment of area about the local z-axis
    alpha:        shear shape factor
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elasticShearSection2d",name)
  retval= materiales.getMaterial(name)
  retval.sectionProperties.A= A
  retval.sectionProperties.E= E
  retval.sectionProperties.G= G
  retval.sectionProperties.I= I
  retval.sectionProperties.Alpha= alpha
  return retval

def defElasticSectionFromMechProp2d(preprocessor,name,mechProp2d):
  '''Constructs an elastic section appropiate for 2D beam analysis, 
  taking mechanical properties of the section form a MechProp2d object.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    mechProp2d:   object of type MechProp2d that contains the mechanical 
                  properties of the section
  '''  
  return defElasticSection2d(preprocessor,name,mechProp2d.A,mechProp2d.E,mechProp2d.I)

#Elastic section 3d.
def defElasticSection3d(preprocessor,name,A,E,G,Iz,Iy,J):
  '''Constructs an elastic section appropiate for 3D beam analysis
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    A:            cross-sectional area of the section
    E:            Young’s modulus of the material
    G:            Shear modulus of the material          
    Iz:           second moment of area about the local z-axis
    Iy:           second moment of area about the local y-axis
    J:            torsional moment of inertia of the section

  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_section_3d",name)
  retval= materiales.getMaterial(name)
  retval.sectionProperties.A= A
  retval.sectionProperties.E= E
  retval.sectionProperties.G= G
  retval.sectionProperties.Iz= Iz
  retval.sectionProperties.Iy= Iy
  retval.sectionProperties.J= J
  return retval

def defElasticSectionFromMechProp3d(preprocessor,name,mechProp3d):
  '''Constructs an elastic section appropiate for 3D beam analysis, 
  taking mechanical properties of the section form a MechProp3d object.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    mechProp2d:   instance of the class MechProp3d that contains the mechanical 
                  properties of the section
  '''  
  return defElasticSection3d(preprocessor,name,mechProp3d.A,mechProp3d.E,mechProp3d.G,mechProp3d.Iz,mechProp3d.Iy,mechProp3d.J)

#Elastic shear section 3d.
def defElasticShearSection3d(preprocessor,name,A,E,G,Iz,Iy,J,alpha):
  '''Constructs an elastic section appropiate for 3D beam analysis, 
  including shear deformations.
  Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    A:            cross-sectional area of the section
    E:            Young’s modulus of the material
    G:            shear modulus of the material          
    Iz:           second moment of area about the local z-axis
    Iy:           second moment of area about the local y-axis
    J:            torsional moment of inertia of the section
    alpha:        shear shape factor
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elasticShearSection3d",name)
  retval= materiales.getMaterial(name)
  retval.sectionProperties.A= A
  retval.sectionProperties.E= E
  retval.sectionProperties.G= G
  retval.sectionProperties.Iz= Iz
  retval.sectionProperties.Iy= Iy
  retval.sectionProperties.J= J
  retval.sectionProperties.Alpha= alpha
  return retval


#Linear elastic isotropic plane strain material.
def defElasticIsotropicPlaneStrain(preprocessor,name,E,nu,rho):
  '''Constructs an linear elastic isotropic plane-strain material.
 Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            Young’s modulus of the material
    nu:           Poisson’s ratio
    rho:          mass density, optional (defaults to 0.0)
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_isotropic_plane_strain_2d",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.nu= nu
  retval.rho= rho
  return retval

#Linear elastic isotropic plane stress material.
def defElasticIsotropicPlaneStress(preprocessor,name,E,nu,rho):
  '''Constructs an linear elastic isotropic plane-stress material.
 Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            Young’s modulus of the material
    nu:           Poisson’s ratio
    rho:          mass density, optional (defaults to 0.0)
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_isotropic_plane_stress_2d",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.nu= nu
  retval.rho= rho
  return retval

#Linear elastic isotropic 3d material.
def defElasticIsotropic3d(preprocessor,name,E,nu,rho):
  '''Constructs an linear elastic isotropic 3D material.
 Parameters:
    preprocessor: preprocessor name
    name:         name identifying the material
    E:            Young’s modulus of the material
    nu:           Poisson’s ratio
    rho:          mass density, optional (defaults to 0.0)
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_isotropic_3d",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.nu= nu
  retval.rho= rho
  return retval

#Elastic membrane plate section.
def defElasticPlateSection(preprocessor,name,E,nu,rho,h):
  '''Constructs an elastic isotropic section appropiate for plate analysis
 Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    E:            Young’s modulus of the material
    nu:           Poisson’s ratio
    rho:          mass density
    h:            overall depth of the section
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_membrane_plate_section",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.nu= nu
  retval.rho= rho
  retval.h= h
  return retval

#Elastic membrane plate section.
def defElasticMembranePlateSection(preprocessor,name,E,nu,rho,h):
  '''Constructs an elastic isotropic section appropiate for plate and shell analysis
 Parameters:
    preprocessor: preprocessor name
    name:         name identifying the section
    E:            Young’s modulus of the material
    nu:           Poisson’s ratio
    rho:          mass density
    h:            overall depth of the section
  '''
  materiales= preprocessor.getMaterialLoader
  materiales.newMaterial("elastic_membrane_plate_section",name)
  retval= materiales.getMaterial(name)
  retval.E= E
  retval.nu= nu
  retval.rho= rho
  retval.h= h
  return retval
