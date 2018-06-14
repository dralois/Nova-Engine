Wichtig: Der /resources Ordner muss in den /Game Ordner kopiert werden,
sonst funktioniert die Solution nicht. Alternativ kann auch die .exe im Debug Ordner ausgeführt werden.
Mit dem Hotkey 'C' kann man die Bewegung aktivieren/deaktivieren.

Frage 1:
A|D
-P-	-> P = (1-x)(1-y)A + (1-x)yB + xyC + x(1-y)D
B|C
(0.5/0.5):
-> A = (1/1/1), B = (0/0/0), C = (1/1/1), D = (0/0/0), x = 0.5, y = 0.5 => P = (0.5,0.5,0.5)
(0.46875/0.46875):
-> A = (1/1/1), B = (0/0/0), C = (1/1/1), D = (0/0/0), x = 0.46875, y = 0.46875 => P ~= (0.502/0.502/0.502)
Frage 2:
Cd = Buffer Farbe, As = akt. Alpha, Cs = akt. Farbe
Cd = As*Cs + (1 - As) * Cd
Cd ist zu Beginn (0/0/0)
Cd = 	
=> 0.5 * (0.5/0/0) + (1 - 0.5) * (0/0/0) = (0.25/0/0)
=> 0 * (1/1/1) + (1 - 0) * (0.25/0/0) = (0.25/0/0)
=> 0.5 * (0.25/0/0) + (1 - 0.5) * (0.25/0/0) =
(0.25/0/0)