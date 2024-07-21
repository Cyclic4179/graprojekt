Die Aufgabenstellung bestand daraus, zwei dünnbesetzte, im Ellpack-Format übergebene Matrizen performant miteinander zu multiplizieren, fehlerhafte Eingaben abzufangen und die Performanz der verschiedenen Implementierungen zu dokumentieren und beurteilen.

Die Multiplikation wurde in sechs Versionen Implementiert. Version eins führt eine klassische Matrixmultiplikation aus. Hauptversion implementiert den Gustavson-Algorithmus, wodurch die Suche zu multiplizierender Werte wegfällt. Die zweite Version transponiert die rechte Matrix für bessere Cachefreundlichkeit und einfachere Suche nach zu multiplizierenden Werten. Version drei arbeitet auf dichten Matrizen, wodurch die suche zu multiplizierender Werte wegfällt. Version vier vereinigt Versionen zwei und drei und arbeitet zudem mit Parallelregistern. Version fünf speichert die Zeilenindizes von jeder Spalte der rechten Matrix um diese mit den Zeilen der linken Matrix zu multiplizieren

Getestet wurde auf einem System mit einem AMD Ryzen 7 5825U Prozessor, 4.546GHz,
16 GB Arbeitsspeicher, NixOS 24.05.20240427.58a1abd (Uakari), 64 Bit, Linux-Kernel 6.8.7. Kompiliert
wurde mit GCC 13.2.0 mit der Option -O2

Florians Beitrag zum Projekt:
Florian kümmerte sich mit Simon viel um die Organisation. Auch hat er das Lesen und Validieren der Ellpack-Dateien implementiert. Er erstellte zusätzlich ein Python-Skript zum Generieren von Beispiel-Matrizen für Tests. Florian schrieb des Weiteren die Struktur des Projekts mit den Funktionsköpfen und header-Dateien und kümmerte sich um die Formatierung und IDE-Einstellungen.

Pierres Beitrag zum Projekt:
Pierres Arbeit umfasst die Erstellung des Makefiles, das Schreiben von Skripten zum automatischen Testen von Ein- und Ausgabe sowie Multiplikation, Generieren von Eingaben und Plotten der Benchmarkings. Er schrieb C-Makros zum Debuggen und Testen auf Nicht-Nullpointer sowie die Funktion um das ELLPACK Format auszugeben. Zudem hat Pierre den Großteil der Mainmethode geschrieben, in welcher Komandozeilenargumente gelesen und verarbeitet werden und das Benchmarking stattfindet.

Simons Beitrag zum Projekt:
Simon hat die Hauptarbeit bei der Implementierung der verschiedenen Multiplikationen übernommen, diese mit verschiedenen Inputs grob getestet, Hilfs- und Debugfunktionen geschrieben und selbst und von den anderen gefundene Bugs identifiziert und behoben. Er hat sich zusammen mit Flo um die Strukturierung des Projekts, z.B. in den Bereichen Aufgabenteilung, Fristen und Einhaltung der Aufgabenstellung, gekümmert.
