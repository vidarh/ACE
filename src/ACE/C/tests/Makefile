
REGRESSION_TESTS= _test_len.test
all: clean $(REGRESSION_TESTS) \
    array_via_pointers.test ehb.test flower.test ham.test HopNet.test ifs.test kx.test linkedlist.test pattern2.test \
	sierp.test spiro.test tree.test welcome.test array_of_structs.test  BezierLab.test boxit.test dragon.test fact.test FractalTree.test \
	hanoi.test hi.test IFF.test land.test pattern.test shuttle.test snowflake.test torus.test tri.test \
	ttt.test bst.test henon.test lorenz.test\
	triscape.test boot.test screenprint.test SubTask.test image.test stun.test 7seg.test

# 7seg has random (a5) ofsets

# boot.s excluded because of segfault if missing library.
# screenprint.s, SubTask.s, image.s excluded because of segfault if missing library bmap.
# stun.b gives syntax error.

clean:
	rm -f *.s

%.s : %.b
	../ace $+

# _tempstring0
%.test : %.s
	diff -ub -I".*lea.*_tempstring[0-9],a[01].*" -I".*move.w.*_bg,d[01].*" $+.expect $+
