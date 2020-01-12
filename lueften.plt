#!/usr/bin/gnuplot

set xlabel "Zeit seit Messbeginn in s"
set ylabel "Temperatur in Grad C"
set y2label "rel. Luftfeuchte in %"
set y2tics

set yrange [20: 23.5]


set terminal png
set output "lueften.png"

set key box

# Tür zu bei ca 0
# Tür auf bei 1950
# Tür zu bei 2500

set arrow from 1000, 23.25 to 0, 23 head
set label "Tür zu" at 1000, 23.25

set arrow from 3000, 23.25 to 1950, 23 head
set label "Tür auf" at 3000, 23.25

set arrow from 4000, 21 to 2500, 20.5 head
set label "Tür zu" at 4000, 21

set arrow from 5500, 21.25 to 8000, 22.5 head
set label "Mehr Leute im Raum" at 5500, 21.25

plot 'lueften.dat' u 2:5 w lp axis x1y1 title "Temperatur" \
     , '' u 2:8 w lp axis x1y2 title "Luftfeuchte"

