Frage 1:
Der Vertexbuffer müsste größer sein (nämlich so groß wie aktuell der Indexbuffer),
außerdem müsste die Reihenfolge der Vertices den Indices entsprechen (also z.B. [v0,v1,v2,v2,v1,v3,..])

Frage 2:
cP = a1*cA + a2*cB + a3*cC
=> a1 = PBC / ABC = 6.4 / 19.21 ~ 0.33
=> a2 = PCA / ABC = 9.6 / 19.21 ~ 0.5
=> a3 = PAB / ABC = 3.2 / 19.21 ~ 0.16
cP = 0.33*cA + 0.5*cB + 0.16*cC = (0.5, 0.5, 0.66)