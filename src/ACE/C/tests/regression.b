'..
'..
'.. This "test" is meant as a simple regression test particularly for the
'.. parser to narrow down places where parses are failing due to refactoring
'.. work
'..


gadget 1,ON

gadget 2,OFF,"12345",(10,75)-(200,88)

gadget 2,OFF,"12345",(10,75)-(200,88),STRING

gadget mod 2,25

bevelbox (10,50)-(150,60), 10

window 1,"Gadgets",(0,0)-(640,200),31

