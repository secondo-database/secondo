Bibliothek kompilieren
=================================================
Erstellen eines jar-Files der MovingObjectsLibrary

Makefile im CVS-Pfad: secondo/Javagui/stlib/MovingObjectsLibrary/src/stlib/makefile
Aufruf "make lib" unter "secondo/Javagui/stlib/MovingObjectsLibrary/src/stlib"



MMDB-Integration + GUI kompilieren
=================================================

Inhalt aus CVS unter "secondo/Javagui/stlib/MovingObjectsLibrary/src/stlib" muss nach "secondo/Javagui/stlib" kopiert werden.
Ordner "data", "operator" und "streamprocessing" aus CVS unter "secondo/Javagui/stlib/mmdb" müssen nach "secondo/Javagui/mmdb/" kopiert werden.

Makefile secondo/Javagui/makefile anpassen:
- Unter Eintrag "makedirs" "make -C stlib all" vor "make -C mmdb all" hinzufügen
- Unter Eintrag "clean" "make -C stlib clean" hinzufügen

GUI neu kompilieren: Aufruf "make javagui" unter "secondo"


MapViewer kompilieren
=================================================
Benötigte Bibliotheken sind im CVS-Pfad: secondo/Javagui/stlib/MapViewer/lib 
Makefile im CVS-Pfad: secondo/Javagui/stlib/MapViewer/makefile

Anwendung MapViewer kompilieren und jar erzeugen: Aufruf "make jar" unter "secondo/Javagui/MapViewer"
Ausführbares jar-File liegt danach im Ordner "secondo/Javagui/MapViewer/jar" und kann durch doppelklick gestartet werden

