Globale Definitionen
<z> := 0,1,2,3,4,5,6,7,8,9
z(x) := x mal <z>
<sstr> := lenOfStr_str
<fileid> := <sstr>

Befehle, welcher der Datenknoten versteht
-----------------------------------------

Die ersten vier Zeichen zeigen den Befehl an
Ist ein Befehl kuerzer als vier Zeichen, so wird er mit Leerzeichen aufgefuellt

8dtn
Gibt alle Informationen des Knotens aus

echo$x
einfache Nachricht, die vom Server eine definierte Antwort erwartet
Ergebnis:0000$x

part<lcat><cat><lcontent><content>
Speichert eine gesamte Datei.
erste  0 bedeutet Befehle der Kategorie 0
zweite 0 bedeutet "Testengine", spaeter kommt Chunk support hinzu
<lcat> := <ziffer>(3) laenge der Kategorie
<cat> := c(<lcat>)
<lcontent> := <ziffer>(10)
<content> := char(<lcontent>)

010i
Initialisiert eine portionsweise Uebertragung einer Datei

010d
Empfaengt Daten

010e
Beendet die Uebertragung einer Datei

05ki
Gibt Informationen über den Knoten Preis
Ergebnis: 0000<data>
<data> := <name><port><webport>
<name> := <sstr> Name des Knotens
<port> := <sstr> Portnummer, auf der Knoten lauscht
<webport> := <sstr> Webportnummer, auf dem ein Webserver lauscht

Befehle, welcher der Index versteht.
------------------------------------
Die ersten vier Zeichen zeigen den Befehl an
Ist ein Befehl kuerzer als vier Zeichen, so wird er mit Leerzeichen aufgefuellt

echo$x
einfache Nachricht, die vom Server eine definierte Antwort erwartet
Ergebnis:0000$x

s   $size(12)$fileId
Gibt an, dass eine neue Datei gespeichert werden möchte.
Ergebnis: 0000<data>
<data> := <fileid><teilzahl><teile>
<teilzahl> := z(12)
<teile> := <teil>*
<teil> := <offsetinfile><size><partnumber><partstotal><datanodeCount><uri(datanodeCount)>
<offsetinfile> := z(12)
<partnumber> := z(12)
<partstotal> := z(12)
<datanodeCount> := z(12)
<uri> := <sstr>

<uris> := <uri>[uris]
<uri>  := <ziffer>(3)<string>(<ziffer>)
<anzahl> := <ziffer>(3)
Bsp: 0000002008meineuri008deineuri

d   $filePath
Löschen einer Datei
Ergebnis: 0000 für Erfolg

dfa
Löscht alle Dateien
Ergebnis: 0000 für Erfolg

?   $name
Prüft, ob ein Feature vorhanden ist
Ergebnis: 00001 für ja 00000 für nein

cf   
Gibt die Anzahl aller Dateien zurück

stat$filePath
Gibt Details zur angegebenen Datei zurück

list
$r = 0000<anzahl><dateiid*>
<anzahl> := <z>(12)
<dateiid> := <sstr>
Gibt eine Liste der DateiIDs zurück

1r  $url
registriert einen Datenknoten

1u  $url
deregistriert einen Datenknoten

authuserpass$username$password
Autorisiert einen Benutzer
Ergebnis:
-, falls nicht autorisierbar
$token - Token der Laenge 512Byte

9q  
beendet den index node sauber

8dtn
liefert alle informationen lesbar
dient zum debug ueber telnet