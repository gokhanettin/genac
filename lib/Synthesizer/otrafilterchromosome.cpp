#include "otrafilterchromosome.h"
#include "Analyzer/circuit.h"
#include "Analyzer/Primitive/cccs.h"
#include "Analyzer/Primitive/voltagesource.h"
#include "Analyzer/Primitive/capacitor.h"
#include "Analyzer/Primitive/resistor.h"
#include "utilities.h"
#include <QtCore/QStringList>

OtraFilterChromosome::OtraFilterChromosome(int resistor,int capacitor)
    :Chromosome(resistor,capacitor)
{
    m_type = Chromosome::OtraFilter;
}

/*virtual*/
QString OtraFilterChromosome::toNetlist() const
{
    CONST_SPLIT_IE(this);
    int nresistor = nresistors();
    int ncapacitor = ncapacitors();
    int i = 0;
    int halfi = 0;
    QString str = "Vs " + QString::number(_CONST_I(_CONST_E(3))) + " " + "0 Vs\n";
    int maxInnerNode = (nresistor + ncapacitor) * 2;
    QString name;
    for(i = 2; i <= maxInnerNode; i += 2) {
        halfi = i/2;
        if(halfi <= ncapacitor) {
            name = "C" + QString::number(halfi);
            str += name + " " +
                QString::number(_CONST_I(i-1)) + " " +
                QString::number(_CONST_I(i)) + " " + name +"\n";
        } else {
            name = "R" + QString::number(halfi - ncapacitor);
            str +=  name + " " +
                QString::number(_CONST_I(i-1)) + " " +
                QString::number(_CONST_I(i)) + " " + name +"\n";
        }
    }
    str += "X1 " + QString::number(_CONST_I(_CONST_E(1))) + " " +
        QString::number(_CONST_I(_CONST_E(0))) + " " +
        QString::number(_CONST_I(_CONST_E(2))) + " " + "0 OTRA\n";

    str += ".LIB OTRA\n.END";

    return str;
}

/*virtual*/
Circuit* OtraFilterChromosome::toCircuit() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    int i = 0;
    int halfi = 0;
    Circuit* cir = new Circuit();
    QStringList nodes;
    QString name;

    nodes << QString::number(_CONST_I(_CONST_E(3))) << "0";
    cir->addComponent(new VoltageSource("Vs",nodes,"Vs"));

    nodes.clear();

    nodes << QString::number(_CONST_I(_CONST_E(2))) << "0"
          << QString::number(_CONST_I(_CONST_E(1))) <<
        QString::number(_CONST_I(_CONST_E(0)));
    cir->addComponent(new CCCS("F1",nodes,"_inf"));

    for(i = 2; i < isize(); i += 2) {
        halfi = i/2;
        nodes.clear();
        nodes << QString::number(_CONST_I(i-1)) << QString::number(_CONST_I(i));
        if(halfi <= ncapacitor) {
            name = "C" + QString::number(halfi);
            cir->addComponent(new Capacitor(name,nodes,name));
        } else {
            name = "R" + QString::number(halfi - ncapacitor);
            cir->addComponent(new Resistor(name,nodes,name));
        }
    }

    cir->do_map();

    return cir;
}

/*virtual*/
int OtraFilterChromosome::esize() const
{
    return 4;
}

/*virtual*/
int OtraFilterChromosome::nShortCircuits() const
{
    CONST_SPLIT_IE(this);
    int n = Chromosome::nOpenCircuits();
    if (_CONST_I(_CONST_E(0)) == _CONST_I(_CONST_E(1))) {
        ++n;
    }
    return n;
}
