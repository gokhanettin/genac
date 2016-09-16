# GENAC

## Build

Minimum Qt 5.5.0 required.
```
sudo apt-get install pkg-config libginac-dev qt5-default cmake
cd to/this/direectory
mkdir build && cd build
cmake ..
make
```

## Run

Run the analyzer:

```
src/genac analyze -n ../samples/circuit1.nl -l ../samples/LIB
*  V(1)
*  ----- =
*  V(4)
*                            R1
*  -------------------------------------------------------
*  R2*s*R3*C2+R2*s*C2*R1+s*R3*C2*R1+C1*R2*s^2*R3*C2*R1+R1

```

Run genetic algorithm based synthesizer:

```
src/genac genetic-synthesize \
--numerator 001   \
--denominator 111 \
--ncapacitors 2   \
--nresistors 3    \
--active-element OPAMP \
--population-size 25 \
--ngenerations 500 \
--selection-type ROULETTEWHEEL \
--crossover-type ONEPOINT \
--crossover-probability 0.80 \
--mutation-probability 0.01 # if not provided, defaults to adaptive mutation probability.
```
