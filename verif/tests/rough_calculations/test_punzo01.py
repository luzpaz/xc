# -*- coding: utf-8 -*-
import rough_calculations.ng_punzonamiento


#CASO 1

qk=2e3      #N/m2
A=5*5     #m2

Vd= rough_calculations.ng_punzonamiento.esfuerzoPunzonamiento(qk,A)
#Da como resultado Vd=80 kN

fck=25e6
d=0.20
a=0.25
b=0.25

Vdmax= rough_calculations.ng_punzonamiento.punzMaximo(fck,d,a,b)
#Da como resultado Vdmax=387 kN

h=0.25
fyd=500e6/1.15

As= rough_calculations.ng_punzonamiento.armaduraPunz(Vd,fck,d,a,b,h,fyd)
#Da como resultado Vd<Vcu -> No necesita armadura de punzonamiento


ratio1= (Vd-80e3)/80e3
ratio2= (Vdmax-387.298334621e3)/387.298334621e3

#print "Vd= ",Vd/1e3," kN"
#print "Vdmax= ",Vdmax/1e3," kN"
#print "As= ", As," cm2/m"
#print "ratio1= ",ratio1
#print "ratio2= ",ratio2

import os
fname= os.path.basename(__file__)
if abs(ratio1)<1e-5 and abs(ratio2)<1e-5 and abs(As)<1e-15:
  print "test ",fname,": ok."
else:
  print "test ",fname,": ERROR."
