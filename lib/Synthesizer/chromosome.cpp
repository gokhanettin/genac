#include "chromosome.h"
#include "opampfilterchromosome.h"
#include "otrafilterchromosome.h"
#include "utilities.h"
#include <QtCore/QMap>
#include <cmath>
#include "dbg.h"

#define PENALIZED(f, p) (f * powf(M_E, (-1.0f*p)))

Chromosome::Chromosome(int ncapacitors, int nresistors)
    :m_type{OpampFilter}, m_ncapacitor{ncapacitors}, m_nresistor{nresistors},
     m_penalty(0), m_quality(0.0f), m_fitness(0.0f)
{
}

int Chromosome::inputNode() const
{
    CONST_SPLIT_IE(this);
    return _CONST_I(_CONST_E(esize() - 1));
}

int Chromosome::outputNode() const
{
    CONST_SPLIT_IE(this);
    return _CONST_I(_CONST_E(esize() - 2));
}


int Chromosome::size() const
{
    return isize() + esize();
}

void Chromosome::fill()
{
    m_IE.resize(size());
}


float Chromosome::fitness() const
{
    return  PENALIZED(m_fitness, m_penalty);
}

/*virtual*/
int Chromosome::nOpenCircuits() const
{
    CONST_SPLIT_IE(this);
    int cnt = 0;
    int imx = imax();
    int *temp = new int[imx + 1];

    for(int i = 0; i <= imx; ++i) {
        temp[i] = 0;
    }
    for(int i = 0; i < size(); ++i) {
        if(i < isize()) {
            ++temp[at(i)];
        } else {
            ++temp[_CONST_I(at(i))];
        }
    }
    for(int i = 0; i <= imx; ++i) {
        if(temp[i] == 1) {
            ++cnt;
        }
    }

    delete[] temp;
    return cnt;
}

/*virtual*/
int Chromosome::nShortCircuits() const
{
    CONST_SPLIT_IE(this);
    int cnt = 0;
    for(int i = 1; i < isize(); ++i) {
        if(i%2 == 0 && _CONST_I(i) == _CONST_I(i-1)) {
            ++cnt;
        }
    }

    // Short-circuit Vs
    if (_CONST_I(_CONST_E(3)) == 0) {
        ++cnt;
    }

    if(inputNode() == 0) {
        ++cnt;
    }

    if(outputNode() == 0) {
        ++cnt;
    }

    if(inputNode() == outputNode()) {
        ++cnt;
    }

    return cnt;
}

int Chromosome::hammingDistance(const Chromosome& other) const
{
    int d = 0;
    int len = size();

    for (int i = 0; i < len; ++i) {
        if (at(i) != other.at(i)) {
            ++d;
        }
    }
    return d;
}

int Chromosome::similarity(const Chromosome& other) const
{
    return size() - hammingDistance(other);
}

QString Chromosome::toPrintable() const
{
    QString str = "";
    int len = size();

    for (int i = 0; i < len; ++i) {
        str += QString::number(at(i));
        if (i < len - 1) {
            str += " ";
        } else {
            str + "\n";
        }
    }
    return str;
}

QString Chromosome::toCanonicalPrintable() const
{
    Chromosome *c = Chromosome::canonicalize(*this);
    int len = size();
    QString canonical;
    for (int i = 0; i < len; ++i) {
        canonical += QString::number(c->at(i));
        if (i < len - 1) {
            canonical += " ";
        } else {
            canonical + "\n";
        }
    }
    delete c;
    return canonical;
}

Chromosome* Chromosome::create(Type type, int ncapacitors,
                               int nresistors, bool random)
{
    Chromosome *c = nullptr;
    switch (type) {
    case Chromosome::OpampFilter:
        c = new OpampFilterChromosome(ncapacitors, nresistors);
        break;
    case Chromosome::OtraFilter:
        c = new OtraFilterChromosome(ncapacitors, nresistors);
        break;
    }

    c->fill();
    SPLIT_IE(c);
    if (random){
        // Random generate
        for (int i = c->emin(); i < c->esize(); ++i) {
            _E(i) = randomInt(c->emin(), c->emax());
        }
        // _I(0) is always Ground
        for(int i = 1; i <= c->isize(); ++i) {
            _I(i) = randomInt(c->imin(), c->imax());
            if(i%2 == 0 && _I(i) == _I(i-1)) {
                if(_I(i) > 0) {
                    _I(i) -= 1;
                } else if(_I(i) < c->imax()) {
                    _I(i) += 1;
                }
            }
        }
    }
    return c;
}

Chromosome* Chromosome::create(Type type, int ncapacitors,
                               int nresistors, const QVector<int>& IE)
{
    Chromosome *c = nullptr;
    switch (type) {
    case Chromosome::OpampFilter:
        c = new OpampFilterChromosome(ncapacitors, nresistors);
        break;
    case Chromosome::OtraFilter:
        c = new OtraFilterChromosome(ncapacitors, nresistors);
        break;
    }

    Q_ASSERT_X(c->size() == IE.size(),
               "Chromosome::create",
               "IE vector and chromosome size don't agree");

    c->fill();
    for (int i = 0; i < IE.size(); ++i) {
        (*c)[i] = IE[i];
    }
    return c;
}

Chromosome* Chromosome::clone(const Chromosome& other)
{
    Chromosome *c = nullptr;
    Type type = other.type();
    int ncapacitors = other.ncapacitors();
    int nresistors  = other.nresistors();
    switch (type) {
    case Chromosome::OpampFilter:
        c = new OpampFilterChromosome(ncapacitors, nresistors);
        break;
    case Chromosome::OtraFilter:
        c = new OtraFilterChromosome(ncapacitors, nresistors);
        break;
    }

    c->fill();
    for (int i = 0; i < other.size(); ++i) {
        (*c)[i] = other[i];
    }
    return c;
}


Chromosome* Chromosome::canonicalize(const Chromosome& other)
{
    int ncap = other.ncapacitors();
    int nres = other.nresistors();
    int len = other.size();
    int ilen = other.isize();
    int temp = 0;
    Chromosome *c = Chromosome::clone(other);
    QMap<int, int> nodes;
    int node = 0;
    for (int i = 0; i < ilen; ++i) {
        if (!nodes.contains(other.at(i))) {
            nodes.insert(other.at(i), node++);
        }
    }

    for (int i = 0; i < len; ++i) {
        if (i < ilen) {
            (*c)[i] = nodes.value(other.at(i));
        } else {
            (*c)[i] = nodes.value(other.at(other.at(i)));
        }
    }

    for(int i = 2; i < ilen; i+=2) {
        if(c->at(i) < c->at(i-1)) {
            temp = c->at(i);
            (*c)[i] = c->at(i-1);
            (*c)[i-1] = temp;
        }
    }

    for (int i = 1; i < ncap; ++i) {
        for (int j = 2; j < ncap*2; j+=2) {
            if (c->at(j-1)*ilen + c->at(j) > c->at(j+1)*ilen + c->at(j+2)) {
                temp = c->at(j+2);
                (*c)[j+2] = c->at(j);
                (*c)[j] = temp;

                temp = c->at(j+1);
                (*c)[j+1] = c->at(j-1);
                (*c)[j-1] = temp;
            }
        }
    }
    for (int i = 1; i < nres; ++i) {
        for (int j = ncap*2+2; j < (ncap+nres)*2; j+=2) {
            if (c->at(j-1)*ilen + c->at(j) > c->at(j+1)*ilen + c->at(j+2)) {
                temp = c->at(j+2);
                (*c)[j+2] = c->at(j);
                (*c)[j] = temp;

                temp = c->at(j+1);
                (*c)[j+1] = c->at(j-1);
                (*c)[j-1] = temp;
            }
        }
    }
    return c;
}
