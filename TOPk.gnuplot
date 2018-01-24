set term png
set output "TOPK_time.png"
set xtics 5
set key top left
plot 'TOPk.dat' using 1:2 with linespoints title 'TPO',\
 'TOPk.dat' using 1:5 with linespoints title 'Unguided BFS',\
 'TOPk.dat' using 1:8 with linespoints title 'Backbone'
