#include "opampfilterchromosome.h"
#include "Analyzer/circuit.h"
#include "Analyzer/Primitive/vcvs.h"
#include "Analyzer/Primitive/voltagesource.h"
#include "Analyzer/Primitive/capacitor.h"
#include "Analyzer/Primitive/resistor.h"
#include "utilities.h"
#include <QtCore/QStringList>

OpampFilterChromosome::OpampFilterChromosome(int ncapacitors, int nresistors)
    :Chromosome(ncapacitors, nresistors)
{
    m_type = Chromosome::OpampFilter;
}

/*virtual*/
QString OpampFilterChromosome::input() const
{
    CONST_SPLIT_IE(this);
    return QString("V(%1)").arg(_CONST_I(_CONST_E(3)));
}

/*virtual*/
QString OpampFilterChromosome::output() const
{
    CONST_SPLIT_IE(this);
    return QString("V(%1)").arg(_CONST_I(_CONST_E(2)));
}

/*virtual*/
QString OpampFilterChromosome::toNetlist() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    QString str = "Vs " + QString::number(_CONST_I(_CONST_E(3))) + " " + "0 Vs\n";
    for(int i = 2; i <= isize(); i += 2) {
        int halfi = i/2;
        if(halfi <= ncapacitor) {
            QString name = "C" + QString::number(halfi);
            str += name + " " +
                QString::number(_CONST_I(i-1)) + " " +
                QString::number(_CONST_I(i)) + " " + name +"\n";
        } else {
            QString name = "R" + QString::number(halfi - ncapacitor);
            str +=  name + " " +
                QString::number(_CONST_I(i-1)) + " " +
                QString::number(_CONST_I(i)) + " " + name + "\n";
        }
    }
    str += "X1 " + QString::number(_CONST_I(_CONST_E(0))) + " " +
        QString::number(_CONST_I(_CONST_E(1))) + " " +
        QString::number(_CONST_I(_CONST_E(2))) + " " + "OPAMP\n";

    str += ".LIB OPAMP\n";
    str += QString(".TF %1 %2\n").arg(output(), input());
    str += ".END";
    return str;
}

/*virtual*/
Circuit* OpampFilterChromosome::toCircuit() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    Circuit* cir = new Circuit();
    QStringList nodes;

    nodes << QString::number(_CONST_I(_CONST_E(3))) << "0";
    cir->addComponent(new VoltageSource("Vs", nodes, "Vs"));
    nodes.clear();

    nodes << QString::number(_CONST_I(_CONST_E((2)))) << "0"
          << QString::number(_CONST_I(_CONST_E((0))))
          << QString::number(_CONST_I(_CONST_E(1)));
    cir->addComponent(new VCVS("E_X",nodes,"_inf"));

    for(int i = 2; i < isize(); i += 2) {
        int halfi = i/2;
        nodes.clear();
        nodes << QString::number(_CONST_I(i-1)) << QString::number(_CONST_I(i));
        if(halfi <= ncapacitor) {
            QString name = "C" + QString::number(halfi);
            cir->addComponent(new Capacitor(name, nodes, name));
        } else {
            QString name = "R" + QString::number(halfi - ncapacitor);
            cir->addComponent(new Resistor(name, nodes, name));
        }
    }

    cir->do_map();

    return cir;
}

/*virtual*/
int OpampFilterChromosome::esize() const
{
    return 4;
}

/*virtual*/
int OpampFilterChromosome::nShortCircuits() const
{
    CONST_SPLIT_IE(this);
    int n = Chromosome::nShortCircuits();
    if (_CONST_I(_CONST_E(0)) == _CONST_I(_CONST_E(1))) {
        ++n;
    }
    if (_CONST_I(_CONST_E(0)) == _CONST_I(_CONST_E(3))) {
        ++n;
    }
    if (_CONST_I(_CONST_E(1)) == _CONST_I(_CONST_E(3))) {
        ++n;
    }
    if (_CONST_I(_CONST_E(2)) == _CONST_I(_CONST_E(3))) {
        ++n;
    }

    for (int i = 0; i < esize(); ++i) {
        if (_CONST_I(_CONST_E(i)) == 0) {
            ++n;
        }
    }
    return n;
}
