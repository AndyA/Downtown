set term png size 1920,1080
set bmargin 8
set output "rms.png"

plot "rms" using 1 with lines title 'rms'

