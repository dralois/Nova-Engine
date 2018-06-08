Wichtig: Der /resources Ordner muss in den /Game Ordner kopiert werden,
sonst funktioniert die Solution nicht. Alternativ kann auch die .exe im Debug Ordner ausgeführt werden.
Mit dem Hotkey 'C' kann man die Bewegung aktivieren/deaktivieren.

Frage 1:
Die benötigte Matrix lautet:

[ 1  0  0 ]
[ 0  0  0 ]
[ 0 1/6 1 ]

Das liegt daran, dass die Projektion über die dritte Zeile gesteuert wird.
Die "Kamera" ist am Punkt [0, -6], daher ist der Parameter (-1/-6) = 1/6.

Frage 2:
Sei die Achse n = (1,1,0) = (u, v, w)
Zunächst normalisiert man n => n = (1/sqrt(2), 1/sqrt(2), 0)

Um die Drehung auszuführen muss man die Achse in die Z-Achse drehen.
Dazu dreht man die Achse n um die x-Achse in die x/z Ebene und dann um die y-Achse in die y/z Ebene.
Dort kann man nun die eigentliche Drehung um die z-Achse um den Winkel 90* ausführen.
Dann führt man die Schritte umgekehrt erneut durch.

Die resultierende Matrixoperationen: Rx(-a) * Ry(-b) * Rz(90*) * Ry(b) * rx(a), mit cos(a) = w / sqrt(v^2 + w^2) und sin(b) = -u

cos(a) = 0/.. = 0	=> a = 90*
sin(b) = -sqrt(2)/2 	=> b =-45*

Die Operationen sind daher (sin und cos in den Drehmatrizen wurden bereits aufgelöst,
sin(90*) = 1, cos(90*) = 0, sin(45*) ~ 0.707.., cos(45*) ~ 0.707 usw.) :

[ 1	0	0	0 ]
[ 0	0	1	0 ]
[ 0	-1	0	0 ]
[ 0	0	0	1 ]
*
[ 0.707	0	0.707	0 ]
[ 0	1	0	0 ]
[ -0.70 0	0.707	0 ]
[ 0	0	0	1 ]
*
[ 0	-1	0	0 ]
[ 1	0	0	0 ]
[ 0	0	1	0 ]
[ 0	0	0	1 ]
*
[ -0.70	0	-0.707	0 ]
[ 0	1	0	0 ]
[ 0.707	0	-0.707	0 ]
[ 0	0	0	1 ]
*
[ 1	0	0	0 ]
[ 0	0	-1	0 ]
[ 0	1	0	0 ]
[ 0	0	0	1 ]
=
[ 0.5	-0.5	0.707	0 ]
[ 0.5	-0.5	-0.707	0 ]
[ 0.707	0.707	0	0 ]
[ 0	0	0	1 ]