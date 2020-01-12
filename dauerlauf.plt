#!/usr/bin/gnuplot

set title "Langzeitmessung von Temperatur und Luftfeuchte in einer Küche"

# date --date="2020-01-11 22:30 CET" +%s
timestamp_measurement_start = 1578778200 + 60*60
# measurement startet at ca 22.30



set key top left

# how to interpret the data as time stamps
set timefmt "%s"
set xdata time
# how to display the time on the x axis
set format x "%H:%M"

set y2tics

set xlabel "Uhrzeit"
set ylabel "Temperatur in Grad C"
set y2label "Relative Luftfeuchte in %"

set yrange [20.6:22.5]
set y2range [50:70]


set label ". Raum benutzt" at 1578812400, 20.75
set label ". Fenster auf" at 1578818400, 20.7
set label ". Fenster zu" at 1578819180, 20.65
set label ". Kochen beginnt" at 1578845400, 20.75
set label ". Backofen an" at 1578849000, 20.7
set label ". Backofen aus" at 1578852000, 20.65


set terminal png size 1000, 800
set output "dauerlauf.png"

plot 'dauerlauf.dat' u (timestamp_measurement_start + $1):3 w lp title "Temperatur <-" \
   , '' u (timestamp_measurement_start + $1):5 w lp title "Luftfeuchte ->" axis x1y2

