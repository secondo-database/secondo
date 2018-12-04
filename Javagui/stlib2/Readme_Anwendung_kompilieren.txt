Bibliothek kompilieren
========================================================
Makefile im CVS-Pfad: secondo/Javagui/stlib/makefile

Einfaches Übersetzen:
Aufruf "make" unter "secondo/Javagui/stlib"

Erstellen eines jar-Files der MovingObjectsLibrary:
Aufruf "make lib" unter "secondo/Javagui/stlib"



GUI kompilieren (inklusive stlib und MMDB-Integration)
========================================================

Makefile secondo/Javagui/makefile anpassen:
- Unter Eintrag "makedirs" "make -C stlib all" vor "make -C mmdb all" hinzufügen
- Unter Eintrag "clean" "make -C stlib clean" hinzufügen

GUI neu kompilieren: Aufruf "make javagui" unter "secondo"


MapViewer kompilieren
=================================================
Benötigte Bibliotheken sind im CVS-Pfad: secondo/Javagui/stlib2/MapViewer/lib 
Makefile im CVS-Pfad: secondo/Javagui/stlib2/MapViewer/makefile

Anwendung MapViewer kompilieren und jar erzeugen: Aufruf "make" unter "secondo/Javagui/stlib2/MapViewer"
Ausführbares jar-File liegt danach im Ordner "secondo/Javagui/stlib2/MapViewer/jar" und kann durch doppelklick gestartet werden

