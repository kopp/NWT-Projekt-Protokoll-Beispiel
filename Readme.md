# Beispielprojekt: Temperatur und Luftfeuchte überwachen

Hier ist ein Beispiel, wie ein Protokoll für ein NWT-Projekt aussehen kann/soll.

Das Protokoll ist hier in
[Textform](Protokoll.md)
und hier
[als HTML](https://htmlpreview.github.io/?https://github.com/kopp/NWT-Projekt-Protokoll-Beispiel/blob/master/Protokoll.html)
zu finden.


# Wann was machen

Bevor ein Projekt begonnen wird, sollten die Abschnitte

- Einführung / Idee
- Anforderungen
- Planung
- Architektur
- Plan

ausgearbeitet oder zumindest umrissen haben.
Dann kann man sich klar werden, was man eigentlich vor hat und wie realistisch das ist.

Im Laufe des Projekts kommen dann die weiteren Abschnitte hinzu.
Sollte sich die Architektur im Laufe des Projekts ändern, dann wird sie angepasst und diese Änderungen werden im Protokoll vermerkt.


# Anmerkungen

- Während Doppelstunde 7 hat sich das Design geändert.
  Diese Änderungen sind dort dokumentiert/begrÜndet und in "Architektur" mit aufgenommen.
- Während Doppelstunde 9 wurde das "Nutzerinterface" konkret gemacht (also dass man `print all` tippen muss).
  Auch das ist sowohl in dem Protokoll als auch in der Architektur erwähnt.
- Weil sich hier ein Großteil der Entwicklung um Software gedreht hat, sind die Bilder in dem Protokoll etwas spärlich.
  Stattdessen wurde Code oder der Output vom _Serial Monitor_ hier abgebildet.
- Im Ausblick kann man während des Projektsimmer wieder Teile hinzufügen, wenn man merkt, dass man etwas besser machen könnte, wenn man mehr Zeit hätte.


# Hier verwendete Tools

- Das Protokoll ist in [Markdown](https://de.wikipedia.org/wiki/Markdown) geschrieben.
- Es wurde dann mit [pandoc](https://pandoc.org/) und [asciidoctor](https://asciidoctor.org/) formatiert.
- Die Aktivitätsdiagramme sind mit [Plantuml](https://plantuml.com/de/activity-diagram-beta) erstellt.
- Plots sind mit [Gnuplot](http://www.gnuplot.info/) erstellt.
