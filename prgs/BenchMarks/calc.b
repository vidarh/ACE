REM $EVENT OFF
REM $OPTION a-o-x-n-

10 PRINT "BYTE CALC Benchmark see May 85 issue"
t=timer
20 NR=5000
30 A=2.718281828459045
40 B=3.141592653589794
50 C=1
60 FOR I=1 TO NR
70 C=C*A
80 C=C*B
90 C=C/A
100 C=C/B
110 NEXT I
t=timer-t
120 PRINT "ERROR=";C-1
print "Took";t;"seconds"
