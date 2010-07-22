/* Derived mathematical functions.
 
  Taken from Appendix E of the AmigaBASIC Manual.

  Date: 27th December 1993 */

'..SECANT
DEF SEC(X)=1/COS(X)

'..COSECANT
DEF CSC(X)=1/SIN(X)

'..COTANGENT
DEF COT(X)=1/TAN(X)

'..INVERSE SINE
DEF ARCSIN(X)=ATN(X/SQR(-X*X+1))

'..INVERSE COSINE
DEF ARCCOS(X)=-ATN(X/SQR(-X*X+1))+1.5708

'..INVERSE SECANT
DEF ARCSEC(X)=ATN(X/SQR(X*X-1))+SGN(SGN(X)-1)*1.5708

'..INVERSE COSECANT
DEF ARCCSC(X)=ATN(X/SQR(X*X-1))+(SGN(X)-1)*1.5708
  
'..INVERSE COTANGENT
DEF ARCCOT(X)=ATN(X)+1.5708

'..HYPERBOLIC SINE
DEF SINH(X)=(EXP(X)-EXP(-X))/2

'..HYPERBOLIC COSINE
DEF COSH(X)=(EXP(X)+EXP(-X))/2

'..HYPERBOLIC TANGENT
DEF TANH(X)=(EXP(-X)/EXP(X)+EXP(-X))*2+1

'..HYPERBOLIC SECANT
DEF SECH(X)=2/(EXP(X)+EXP(-X))

'..HYPERBOLIC COSECANT
DEF CSCH(X)=2/(EXP(X)-EXP(-X))

'..HYPERBOLIC COTANGENT
DEF COTH(X)=EXP(-X)/(EXP(X)-EXP(-X))*2+1

'..INVERSE HYPERBOLIC SINE
DEF ARCSINH(X)=LOG(X+SQR(X*X+1))

'..INVERSE HYPERBOLIC COSINE
DEF ARCCOSH(X)=LOG(X+SQR(X*X-1))

'..INVERSE HYPERBOLIC TANGENT
DEF ARCTANH(X)=LOG((1+X)/(1-X))/2

'..INVERSE HYPERBOLIC SECANT
DEF ARCSECH(X)=LOG((SQR(-X*X+1)+1)/X)

'..INVERSE HYPERBOLIC COSECANT
DEF ARCCSCH(X)=LOG(SGN(X)*SQR(X*X+1)+1)/X

'..INVERSE HYPERBOLIC COTANGENT
DEF ARCCOTH(X)=LOG((X+1)/(X-1))/2
