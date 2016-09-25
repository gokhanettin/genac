#include "estimator.h"
#include "chromosome.h"
#include "population.h"
#include "Analyzer/analyzer.h"
#include "Analyzer/circuit.h"
#include "Analyzer/pretty.h"
#include <cfloat>
#include <cmath>
#include "dbg.h"

# define ALPHA (0.60f)
# define BETA  (0.40f)

#define VALID_FUNC(cost)   (BETA + ALPHA / (1.0f + (float)(cost)))
#define INVALID_FUNC(cost) (BETA / (1.0f + (float)(cost)))

Estimator::Estimator(const QString& requirements)
    :m_analyzer(new Analyzer),
     m_nexception(0)
{
    m_analyzer->makeSymbol(m_s,"s");
    m_analyzer->makeSymbol(m_inf,"_inf");
    setRequirements(requirements);
}

Estimator::~Estimator()
{
    delete m_analyzer;
}

void Estimator::setRequirements(const QString& requirements)
{
    m_requirements.resize(requirements.size());
    for (int i = 0; i < requirements.size(); ++i) {
        m_requirements[i] = (requirements[i] == '1');
    }
    m_sums.resize(requirements.size());
}

void Estimator::setQuality(Chromosome *c)
{
    if (!c->transferFunction().isEmpty()) {
        return;
    }
    int reqsize = m_requirements.size();
    int cost = c->nShortCircuits() + c->nOpenCircuits();
    if (cost > 0) {
        c->setQuality(INVALID_FUNC(cost));
        c->setTransferFunction("Has Open or Short Elements");
        return;
    }
    Circuit *circuit = c->toCircuit();
    m_analyzer->stamp(circuit);
    Analyzer::TransferFunction tf;
    try {
        m_analyzer->solve();
        delete circuit;
        circuit = nullptr;
        QString out = QString("V(%1)").arg(QString::number(c->outputNode()));
        QString in = QString("V(%1)").arg(QString::number(c->inputNode()));
        tf = m_analyzer->calcTF(out, in);
        c->setTransferFunction(pretty(tf));
    } catch (const std::exception &e) {
        delete circuit;
        circuit = nullptr;
        ++m_nexception;
        qDebug() << QString::fromStdString(e.what())
                 << " exception: " << m_nexception
                 << " " << c->toPrintable();
        c->setQuality(FLT_EPSILON);
        c->setTransferFunction("Makes an inconsistent matrix");
        return;
    }

    // if (tf.rhs.is_zero() || tf.rhs.is_equal(m_inf) || tf.rhs.is_equal(1)) {
    //     c->setQuality(BETA);
    //     return;
    // }

    int degree = reqsize / 2 - 1;
    GiNaC::ex numer, denom, coeff;
    bool exists = false;
    QVector<bool> v;
    v.resize(reqsize);
    numer = tf.rhs.numer();
    denom = tf.rhs.denom();
    cost = 0;
    for (int i = degree; i >= 0; --i) {
        coeff = numer.coeff(m_s, i);
        exists = coeff.is_zero() == m_requirements[degree - i];
        v[degree - i] = exists;
        cost += exists;

    }
    for (int i = degree; i >= 0; --i) {
        coeff = denom.coeff(m_s, i);
        exists = coeff.is_zero() == m_requirements[2*degree - i + 1];
        v[2*degree - i + 1] =  exists;
        cost += exists;
    }
    c->setImperfections(v);
    c->setQuality(VALID_FUNC(cost));
}

void Estimator::setPopulationData(Population *p)
{
    Q_CHECK_PTR(p);
    const int M = p->size();
    Q_ASSERT(M > 0);
    int L = p->at(0)->size();
    const int R = m_requirements.size();
    float diversity = 0.0f;
    Chromosome *ci = nullptr;
    Chromosome *cj = nullptr;
    for (int i = 0; i < M-1; ++i) {
        ci = (*p)[i];
        for (int j = i + 1; j < M; ++j) {
            cj = (*p)[j];
            diversity += ci->hammingDistance(*cj);
        }
        setQuality(ci);
        if (ci->isValid()) {
            for (int k = 0; k < R; ++k) {
                    m_sums[k] += ci->hasImperfection(k);
            }
        }
    }
    ci = (*p)[M-1];
    setQuality(ci);
    for (int k = 0; k < R; ++k) {
        if (ci->isValid()) {
            m_sums[k] += ci->hasImperfection(k);
        }
    }

    qDebug() << "Imperfection sums: " << m_sums;

    diversity /= (L * M * (M - 1)/2.0f);
    p->setDiversity(diversity);
    float cost = 0.0f;
    float avrgq = 0.0f;
    float maxq = 0.0f;
    float q = 0.0f;
    for (int i = 0; i < M; ++i) {
        ci = (*p)[i];
        q = ci->quality();
        if (q > maxq) {
            maxq = q;
        }
        cost = 0;
        avrgq += q;
        if (ci->isValid()) {
            for (int k = 0; k < R; ++k) {
                cost += m_sums[k] * m_sums[k] * ci->hasImperfection(k);
            }
            cost /= (float)R;
            cost = sqrtf(cost);
            ci->setFitness(VALID_FUNC(cost));
        } else {
            ci->setFitness(q);
        }
    }

    avrgq /= M;
    p->setAverageQuality(avrgq);
    p->setMaxQuality(maxq);

    float maxf = 0.0f;
    float avrgf = 0.0f;
    float f = 0.0f;
    for (int i = 0; i < M; ++i) {
        ci = (*p)[i];
        f = ci->fitness();
        if (f > maxf) {
            maxf = f;
        }
        avrgf += f;
    }
    avrgf /= M;
    p->setAverageFitness(avrgf);
    p->setMaxFitness(maxf);
}
