
(DATABASE 
    (TYPES 
        (TYPE BahnhofKlasse 
            (SIMPLECLASS Bahnhof)) 
        (TYPE Bahnhof 
            (SIMPLEOBJ 
                (OID Bahnhof) 
                (TUPLE 
                    (Name STRING) 
                    (Position POINT)))) 
        (TYPE AbschnittKlasse 
            (LINKCLASS Abschnitt)) 
        (TYPE Abschnitt 
            (LINKOBJ 
                (OID Abschnitt) 
                (TUPLE 
                    (von STRING) 
                    (nach STRING) 
                    (Fahrzeit INT) 
                    (Entfernung REAL) 
                    (Verlauf LINE)) 
                (OID Bahnhof) 
                (OID Bahnhof))) 
        (TYPE LinienKlasse 
            (PATHCLASS Linie)) 
        (TYPE Linie 
            (PATHOBJ 
                (OID Linie) 
                (TUPLE 
                    (Nr INT) 
                    (AnzahlStationen INT) 
                    (Haeufigkeit INT)) 
                (PATH Abschnitt)))) 
    (OBJECTS 
        (OBJECT Bahnhoefe Bahnhofklasse 
            (SIMPLECLASS 
                (SIMPLEOBJ 
                    (OID Bahnhof 1) 
                    (TUPLE "Wallisellen" 
                        (POINT 26.000 24.000))) 
                (SIMPLEOBJ 
                    (OID Bahnhof 3) 
                    (TUPLE "Oerlikon" 
                        (POINT 7.000 24.000))) 
                (SIMPLEOBJ 
                    (OID Bahnhof 5) 
                    (TUPLE "Wipkingen" 
                        (POINT -3.000 9.000))) 
                (SIMPLEOBJ 
                    (OID Bahnhof 7) 
                    (TUPLE "Zuerich-HB" 
                        (POINT .000 .000))))) 
        (OBJECT Abschnitte AbschnittKlasse 
            (LINKCLASS 
                (LINKOBJ 
                    (OID Abschnitt 2) 
                    (TUPLE "Wallisellen" "Oerlikon" 5 1.900 
                        (LINE 
                            (point 26.000 24.000) 
                            (point 23.000 26.000) 
                            (point 12.000 26.000) 
                            (point 7.000 24.000))) 
                    (OID Bahnhof 1) 
                    (OID Bahnhof 3)) 
                (LINKOBJ 
                    (OID Abschnitt 4) 
                    (TUPLE "Oerlikon" "Wipkingen" 2 1.800 
                        (LINE 
                            (point 7.000 24.000) 
                            (point -1.000 16.000) 
                            (point -1.000 12.000) 
                            (point -3.000 9.000))) 
                    (OID Bahnhof 3) 
                    (OID Bahnhof 5)) 
                (LINKOBJ 
                    (OID Abschnitt 6) 
                    (TUPLE "Wipkingen" "Zuerich-HB" 6 .960 
                        (LINE 
                            (point -3.000 9.000) 
                            (point -8.000 4.000) 
                            (point -8.000 2.000) 
                            (point -7.000 1.000) 
                            (point -6.000 .000) 
                            (point .000 .000))) 
                    (OID Bahnhof 5) 
                    (OID Bahnhof 7)))) 
        (OBJECT Linien LinienKlasse 
            (PATHCLASS 
                (PATHOBJ 
                    (OID Linie 8) 
                    (TUPLE 8 4 30) 
                    (PATH 1 2 3 4 5 6 7))))))
