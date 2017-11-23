# Frac

##simple and mixed fractions

Calculates and simplifies with mixed fractions - Version: 20171117.202226

`Usage: frac [-h] [(integer|mixed|fraction) ((+|-|*|/) (integer|mixed|fraction))?]`

* Options:
	-h,  --help
		+ Show this help.
	-v,  --verbose
		+ Set verbose level (cumulative). (not implemented)

	- integer:
		+ a number composed of digits 0..9 (e.g. 123)
	- fraction:
		+ two integers separated by '/' with no spaces in between (e.g. 4/56)
	- mixed:
		+ a mixed fraction is a integer followed by a fraction, separated by a space (e.g. 12 3/4)

* Exit status:
	- 0 if ok.
	- a error code otherwise.

* Author:
	- Written by Ruben Carlo Benante <rcb@beco.cc>

# Automata:

## Tokens:

* i: integer (d+)
* f: fraction (d+/d+)
* o: operator ('+','-', '*', '/')


```

              i                               i
            ------> (( 1 ))                 -----> (( 4 ))
           /           |   \               /          |
          /            |    \   o         /           |
 --> ( 0 )           f |     ------> ( 3 )          f |
          \            |    /             \           |
           \    f      ∨   /               \  f       ∨
            ------> (( 2 ))                 -----> (( 5 ))

```








* By Dr. Beco
* Created: 2017-11-22

