#ifndef OTRAFILTERCHROMOSOME_H
#define OTRAFILTERCHROMOSOME_H
#include "chromosome.h"

/* E[0]  : -
 * E[1]  : +
 * E[2]  : OUT
 * E[3]  : Vs
 * I[0]  : 0 (always GND)
 * 0     : GND
 */

class OtraFilterChromosome : public Chromosome
{
public:
    OtraFilterChromosome(int ncapacitor, int nresistor);

    virtual ~OtraFilterChromosome() =default;

    virtual QString toNetlist() const;
    virtual Circuit* toCircuit() const;
    virtual int esize() const;
    virtual int nShortCircuits() const;
};

#endif // OTRAFILTERCHROMOSOME_H
