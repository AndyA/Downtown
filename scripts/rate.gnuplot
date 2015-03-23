set term png size 1920,1080
set bmargin 8
set output "rate.png"

plot "rate" using 1 with lines title 'rate'

