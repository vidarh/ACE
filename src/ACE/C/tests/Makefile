
all: clean regression.test ACEgadgets.test BezierLab.test events.test IFF.test sierp.test triscape.test

clean:
	rm -f *.s

%.s : %.b
	../ace $+

%.test : %.s
	diff -ub $+.expect $+

