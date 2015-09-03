# -*- coding: utf-8 -*-

#Based on sXML-master projet on gitHub

import Container as ctr
import TableNode as tb
import Header as hdr
import HeaderItem as hi
import Object as obj
import ObjectItem as oI
import Row as rw
import NodeLoadProperties as nlp
import LoadCaseContainer as lcc
import NodeContainer as nc
import LoadComponentBase as lcb

idNodeLoadContainer= "{F8371A21-F459-11D4-94D3-000000000000}"
tNodeLoadContainer= nlp.tbName
idNodeLoadContainerTb= "18E5DA6A-A48A-4522-A210-E961AF296D93"
tNodeLoadContainerTb= nlp.tbName
nodeLoadPrefix= 'F'

class NodeLoadComponent(lcb.LoadComponentBase):
  ''' Each of the components (X, Z or Z) of a punctual load '''
  counter= 0
  memberType= nc.idNodeContainer
  memberTypeName= nc.progIdNodes
  def __init__(self, nodeId, loadCaseId, loadCaseName, direction, value):
    super(NodeLoadComponent,self).__init__(loadCaseId, loadCaseName, direction, value)
    NodeLoadComponent.counter+=1
    self.nodeLoadCompId= NodeLoadComponent.counter 
    self.nodeId= nodeId

  def getNodeReferenceItem(self):
    retval= oI.ObjectItem()
    nodeId= str(self.nodeId)
    name= nc.nodePrefix+nodeId
    row= rw.RowP012("0", oI.ObjectItem(nc.idNodeContainer, None, None, None,None,None), oI.ObjectItem(nc.progIdNodes, None, None, None,None,None), oI.ObjectItem(name, None, None, None,None,None))
    retval.rows.append(row)
    return retval
    
  def getObject(self):
    retval= obj.Object()
    loadCompId= str(self.nodeLoadCompId)
    retval.setId(loadCompId)
    name= nodeLoadPrefix+loadCompId
    retval.setNm(name)
    nodeId= str(self.nodeId) # Load name.
    retval.setP0(oI.ObjectItem(name)) #Name
    retval.setP1(self.getLoadCaseReferenceItem()) #Reference to load case.
    retval.setP2(self.getNodeReferenceItem()) #Reference to node.
    retval.setP3(self.getDirectionObjectItem()) #Direction X, Y or Z
    retval.setP4(oI.ObjectItem('0','','','Force')) #Type 0 -> Force.
    retval.setP5(self.getValueObjectItem()) #Value
    retval.setP6(self.getSystemItem()) #System 0 -> GCS, 1 -> LCS
    return retval


def getNodeLoadComponents(nodeLoad):
  '''get NodeLoadComponent objects from the 3D nodeLoad argument.'''
  retval= []
  vDir= nodeLoad.vDir
  nodeId= nodeLoad.tag
  loadCaseId= nodeLoad.loadCaseId
  loadCaseName= nodeLoad.loadCaseName
  value= nodeLoad.value
  if(vDir[0]!=0.0):
    retval.append(NodeLoadComponent(nodeId, loadCaseId, loadCaseName, 'X',value*vDir[0])) #X component.
  if(vDir[1]!=0.0):
    retval.append(NodeLoadComponent(nodeId, loadCaseId, loadCaseName, 'Y',value*vDir[1])) #Y component.
  if(vDir[2]!=0.0):
    retval.append(NodeLoadComponent(nodeId, loadCaseId, loadCaseName, 'Z',value*vDir[2])) #Z component.
  return retval
  
def getNodeLoadObjects(nl):
  components= getNodeLoadComponents(nl)
  retval= []
  for c in components:
    retval.append(c.getObject())
  return retval

class NodeLoadContainer(ctr.Container):
  def __init__(self,nodeLoadsDict):
    super(NodeLoadContainer,self).__init__(idNodeLoadContainer,tNodeLoadContainer)
    nodeLoads=[]
    for nl in nodeLoadsDict:
      compObjects= getNodeLoadObjects(nl)
      for c in compObjects:
        nodeLoads.append(c)
    self.table= tb.TableNode(idNodeLoadContainerTb,tNodeLoadContainerTb, 'Point forces in node', None,nodeLoads)
  
