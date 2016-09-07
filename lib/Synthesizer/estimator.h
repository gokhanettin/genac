#ifndef ESTIMATOR_H
#define ESTIMATOR_H

#include <QtCore/QVector>
#include <ginac/ginac.h>

class Chromosome;
class Population;
class Analyzer;

class Estimator
{
public:
    Estimator(const QString& requirements = QString());
    ~Estimator();
    void setQuality(Chromosome *c);
    void setPopulationData(Population *p);
    void setRequirements(const QString& requirements);
    QVector<bool> requirements() {return m_requirements;}
private:
    Analyzer *m_analyzer;
    int m_nexception;
    QVector<int> m_sums;
    GiNaC::realsymbol m_s;
    GiNaC::realsymbol m_inf;
    QVector<bool> m_requirements;
};

#endif /* ESTIMATOR_H */
