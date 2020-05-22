#include "otrafilterchromosome.h"
#include "Analyzer/circuit.h"
#include "Analyzer/Primitive/cccs.h"
#include "Analyzer/Primitive/vcvs.h"
#include "Analyzer/Primitive/voltagesource.h"
#include "Analyzer/Primitive/capacitor.h"
#include "Analyzer/Primitive/resistor.h"
#include "utilities.h"
#include <QtCore/QStringList>

OtraFilterChromosome::OtraFilterChromosome(int ncapacitors, int nresistors)
    :Chromosome(ncapacitors, nresistors)
{
    m_type = Chromosome::OtraFilter;
}

/*virtual*/
QString OtraFilterChromosome::toNetlist() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    QString str = "Vs " + QString::number(_CONST_I(_CONST_E(3))) + " 0 Vs\n";
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
                QString::number(_CONST_I(i)) + " " + name +"\n";
        }
    }
    str += "X1 " + QString::number(_CONST_I(_CONST_E(0))) + " " +
        QString::number(_CONST_I(_CONST_E(1))) + " " +
        QString::number(_CONST_I(_CONST_E(2))) + " " + "OTRA\n";

    str += ".LIB OTRA\n";
    str += QString(".TF V(%1) V(%2)\n")
        .arg(QString::number(outputNode()), QString::number(inputNode()));
    str += ".END";

    return str;
}

/*virtual*/
Circuit* OtraFilterChromosome::toCircuit() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    Circuit* cir = new Circuit();
    QStringList nodes;

    nodes << QString::number(_CONST_I(_CONST_E(3))) << "0";
    cir->addComponent(new VoltageSource("Vs", nodes, "Vs"));

    for(int i = 2; i <= isize(); i += 2) {
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

    nodes.clear();
    nodes << QString::number(_CONST_I(_CONST_E(0))) << "0";
    cir->addComponent(new VoltageSource("V_X" ,nodes, "0"));

    nodes.clear();
    nodes << QString::number(_CONST_I(_CONST_E(1))) << "0";
    cir->addComponent(new VoltageSource("V_Y" ,nodes, "0"));

    nodes.clear();
    nodes << "0" << "hidden";
    cir->addComponent(new CCCS("F_X", nodes, "V_X", "1"));
    cir->addComponent(new CCCS("F_Y", nodes, "V_Y", "-1"));
    cir->addComponent(new Resistor("R_Z", nodes, "_inf"));

    nodes.clear();
    nodes << QString::number(_CONST_I(_CONST_E((2)))) << "0" << "hidden" << "0";
    cir->addComponent(new VCVS("E_Z", nodes, "1"));

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
    int n = Chromosome::nShortCircuits();
    if (_CONST_I(_CONST_E(0)) == _CONST_I(_CONST_E(1))) {
        ++n;
    }
    for (int i = 0; i < 3; ++i) {
        if (_CONST_I(_CONST_E(i)) == _CONST_I(_CONST_E(3))) {
            ++n;
        }
        if (_CONST_I(_CONST_E(i)) == 0) {
            ++n;
        }
    }
    return n;
}
