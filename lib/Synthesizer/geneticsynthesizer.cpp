#include "geneticsynthesizer.h"
#include "estimator.h"
#include "chromosome.h"
#include "population.h"
#include "selection.h"
#include "crossover.h"
#include "mutation.h"
#include "utilities.h"
#include <QtCore/QString>
#include <cmath>
#include "dbg.h"

#define RECORDFILE "GARecords.txt"
#define SAVEFILE   "GABests.txt"

#define DIVERSITY_TO_MUTATION_P(d) (powf(M_E, (-10.0f*d)))

#define setChromosomeType(t, x)                    \
    do {                                           \
        if ((x) == "OPAMP") {                      \
            t = Chromosome::OpampFilter;           \
        } else if ((x) == "OTRA") {                \
            t = Chromosome::OtraFilter;            \
        }                                          \
    } while(0)

#define setSelectionType(t, x)                                    \
    do {                                                          \
        if ((x) == "ROULETTEWHEEL") {                             \
            t = GeneticSynthesizer::SelectionType::RouletteWheel; \
        } else if ((x) == "RANK") {                               \
            t = GeneticSynthesizer::SelectionType::Rank;          \
        } else if ((x) == "TOURNAMENT") {                         \
            t = GeneticSynthesizer::SelectionType::Tournament;    \
        }                                                         \
    } while(0)

#define setCrossoverType(t, x)                                    \
    do {                                                          \
        if ((x) == "ONEPOINT") {                                  \
            t = GeneticSynthesizer::CrossoverType::OnePoint;      \
        } else if ((x) == "TWOPOINT") {                           \
            t = GeneticSynthesizer::CrossoverType::TwoPoint;      \
        } else if ((x) == "UNIFORM") {                            \
            t = GeneticSynthesizer::CrossoverType::Uniform;       \
        } else if ((x) == "SINGLEGENE") {                         \
            t = GeneticSynthesizer::CrossoverType::SingleGene;    \
        }                                                         \
    } while(0)


static bool lessThan(Chromosome *c1, Chromosome *c2)
{
    return c1->fitness() < c2->fitness();
}

GeneticSynthesizer::GeneticSynthesizer()
    :m_estimator(new Estimator),
     m_selectionType(RouletteWheel),
     m_crossoverType(OnePoint),
     m_population(nullptr),
     m_recordFile(RECORDFILE),
     m_saveFile(SAVEFILE)
{
    if (m_saveFile.open(QFile::WriteOnly | QFile::Truncate)) {
        m_saveStream.setDevice(&m_saveFile);
    }
    if (m_recordFile.open(QFile::WriteOnly | QFile::Truncate)) {
        m_recordStream.setDevice(&m_recordFile);
        m_recordStream.setRealNumberNotation(QTextStream::FixedNotation);
        m_recordStream.setNumberFlags(QTextStream::ForcePoint);
        m_recordStream.setRealNumberPrecision(4);
        m_recordStream << "Generation;Avergage Quality;MaxQuality;"
                       << "Average Fitness;Max Fitness;"
                       << "Diversity\n";
    }
}

GeneticSynthesizer::~GeneticSynthesizer()
{
    delete m_estimator;
}

void GeneticSynthesizer::run(const QString& nreq, const QString& dreq,
         const QString& active, int psize, int generations,
         const QString& selection, const QString& xover,
         float xp, float mp, int cc, int rc, bool adaptivem)
{
    m_estimator->setRequirements(nreq + dreq);
    Chromosome::Type ctype = Chromosome::OpampFilter;
    setChromosomeType(ctype, active);
    setSelectionType(m_selectionType, selection);
    setCrossoverType(m_crossoverType, xover);
    m_population = new Population;
    m_population->initialize(psize, ctype, cc, rc, true);
    Population *population = nullptr;
    int n = 1;
    while (true) {
        population = new Population;
        population->reserve(psize);
        m_estimator->setPopulationData(m_population);
        std::sort(m_population->begin(), m_population->end(), lessThan);
        Chromosome *best = Chromosome::clone(*m_population->last());
        record(n);
        Chromosome *c = nullptr;
        qDebug() << "Generation: " << n;
        for (int i = 0; i < m_population->size(); ++i) {
            c = (*m_population)[i];
            if (c->quality() == 1.0f) {
                save(c, n);
            }
            qDebug() << c->toPrintable() << " ---> " << c->fitness();
            // qDebug().nospace().noquote() << c->transferFunction();
        }
        if (n == generations) {
            delete m_population;
            break;
        }
        while (true) {
            Chromosome *parent1 = nullptr;
            Chromosome *parent2 = nullptr;
            Chromosome *child1 = nullptr;
            Chromosome *child2 = nullptr;
            if (m_population->size() == 1) {
                population->append(m_population->takeFirst());
                break;
            }
            select(&parent1, &parent2);
            if (flip(xp)) {
                crossover(parent1, parent2, &child1, &child2);
            } else {
                child1 = Chromosome::clone(*parent1);
                child2 = Chromosome::clone(*parent2);
            }
            float mutp = adaptivem ? DIVERSITY_TO_MUTATION_P(m_population->diversity()) : mp;
            if (flip(mutp)) {
                mutate(child1);
                mutate(child2);
            }
            if (population->size() < psize) {
                population->append(child1);
            } else {
                break;
            }
            if (population->size() < psize) {
                population->append(child2);
            } else {
                break;
            }
        }

        int j = 0;
        c = (*population)[j];
        for (int i = 1; i < population->size(); ++i) {
            if (lessThan((*population)[i], c)) {
                j = i;
                c = (*population)[j];
            }
        }
        delete c;
        (*population)[j] = best;
        delete m_population;
        m_population = population;
        ++n;
    }
}

void GeneticSynthesizer::select(Chromosome **parent1, Chromosome **parent2)
{
    switch (m_selectionType) {
    case RouletteWheel:
        Selection::rouletteWheel(m_population, parent1);
        (*parent1)->penalize();
        Selection::rouletteWheel(m_population, parent2);
        (*parent2)->penalize();
        break;
    case Rank:
        Selection::rank(m_population, parent1);
        (*parent1)->penalize();
        std::sort(m_population->begin(), m_population->end(), lessThan);
        Selection::rank(m_population, parent2);
        (*parent2)->penalize();
        std::sort(m_population->begin(), m_population->end(), lessThan);
        break;
    case Tournament:
        Selection::tournament(m_population, parent1);
        (*parent1)->penalize();
        Selection::tournament(m_population, parent2);
        (*parent2)->penalize();
        break;
    }
}

void GeneticSynthesizer::crossover(const Chromosome *parent1, const Chromosome *parent2,
               Chromosome **child1, Chromosome **child2)
{
    switch (m_crossoverType) {
    case OnePoint:
        Crossover::onePoint(parent1, parent2, child1, child2);
        break;
    case TwoPoint:
        Crossover::twoPoint(parent1, parent2, child1, child2);
        break;
    case Uniform:
        Crossover::uniform(parent1, parent2, child1, child2);
        break;
    case SingleGene:
        Crossover::singleGene(parent1, parent2, child1, child2);
        break;
    }
}

void GeneticSynthesizer::mutate(Chromosome *c)
{
    if (flip(0.5)) {
        Mutation::shortCircuit(c);
    } else if (flip(0.5)) {
        Mutation::swap(c);
    } else {
        Mutation::fullyRandomized(c);
    }
}

void GeneticSynthesizer::save(const Chromosome *c, int gen)
{
    QString canonical = c->toCanonicalPrintable();
    if (!m_bests.contains(canonical)) {
        m_saveStream << "********** " << gen << " **********\n"
                     << "Canonical form: " << canonical << "\n"
                     << c->toNetlist() << "\n" << c->transferFunction() << "\n";
        m_saveStream.flush();
        m_bests.insert(canonical);
    }
}

void GeneticSynthesizer::record(int gen)
{
    m_recordStream << gen
                   << ";" << m_population->averageQuality() << ";" << m_population->maxQuality()
                   << ";" << m_population->averageFitness() << ";" << m_population->maxFitness()
                   << ";" << m_population->diversity() << "\n";
    m_recordStream.flush();
}
