# -*- coding: utf-8 -*-
# Problem type

from materials.ehe import auxEHE


fck= 25e6
fcm= auxEHE.getFcm(fck)
ratio1= abs(fcm-33e6)/33e6
Ecm= auxEHE.getEcm(fck)
ratio2= abs(Ecm-27.3e9)/27.3e9

''' 
print "fck= ",fck
print "fcm= ",fcm
print "ratio1= ",ratio1
print "Ecm= ",Ecm
print "ratio2= ",ratio2
   '''

import os
fname= os.path.basename(__file__)
if((ratio1<1e-10) & (ratio2<2e-3)):
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."

