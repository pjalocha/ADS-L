set datafile separator ","
# set key outside right top

set title "Effect of error correction on packet reception rates for ADS-L and OGN"

set term png size 1024,640
set output "PER.png"


set grid

set xlabel "Received signal level[dBm]"
set xrange [-118:-105]
set xtics  2
set mxtics 2

set ylabel "Packet Error Rate [%]"
set yrange [-10:110]
set ytics  20
set mytics  2

set y2label "Number of [bits] corrected per packet and received SNR [dB]"
set y2range [-2:22]
set y2tics  10

set grid  xtics  ytics lc rgb "#666666" lw 1.5
set grid mxtics mytics lc rgb "#aaaaaa" lw 1.0 dt 2


# pt: 7=filled circle, 5=filled square, 9=filled triangle, 13 = filled diamond
plot \
    "adsl.csv"    using 1:4 with linespoints lw 2.0 pt 5  ps 1   lt rgb "red"  title "ADS-L with error correction (CRC)", \
    "ogn1.csv"    using 1:4 with linespoints lw 2.0 pt 5  ps 1   lt rgb "blue" title "OGN with error correction (LDPC FEC)", \
    "adsl_nc.csv" using 1:4 with linespoints lw 1.5 pt 7  ps 1   lt rgb "red"  title "ADS-L without error correction", \
    "ogn1_nc.csv" using 1:4 with linespoints lw 1.5 pt 7  ps 1   lt rgb "blue" title "OGN without error correction", \
    "adsl.csv"    using 1:($1>=(-115.75)?$3:1/0) with linespoints lw 1   pt 9  ps 0.5 lt rgb "red"  axis x1y2 title "Corrected bits per ADS-L packet", \
    "ogn1.csv"    using 1:($1>=(-117.75)?$3:1/0) with linespoints lw 1   pt 9  ps 0.5 lt rgb "blue" axis x1y2 title "Corrected bits per OGN packet", \
    "adsl.csv"    using 1:($1>=(-115.75)?$2:1/0) with linespoints lw 1   pt 13 ps 0.5 lt rgb "red"  axis x1y2 title "Received SNR for ADS-L packets", \
    "ogn1.csv"    using 1:($1>=(-117.75)?$2:1/0) with linespoints lw 1   pt 13 ps 0.5 lt rgb "blue" axis x1y2 title "Received SNR for OGN packets"
