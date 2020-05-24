#include "cciifilterchromosome.h"
#include "Analyzer/circuit.h"
#include "Analyzer/Primitive/cccs.h"
#include "Analyzer/Primitive/vcvs.h"
#include "Analyzer/Primitive/currentsource.h"
#include "Analyzer/Primitive/voltagesource.h"
#include "Analyzer/Primitive/capacitor.h"
#include "Analyzer/Primitive/resistor.h"
#include "utilities.h"
#include <QtCore/QStringList>

CciiFilterChromosome::CciiFilterChromosome(int ncapacitors, int nresistors)
    :Chromosome(ncapacitors, nresistors)
{
    m_type = Chromosome::CciiFilter;
}

/*virtual*/
QString CciiFilterChromosome::input() const
{
    CONST_SPLIT_IE(this);
    return "I(V1)";
}

/*virtual*/
QString CciiFilterChromosome::output() const
{
    CONST_SPLIT_IE(this);
    return "I(V2)";
}

/*virtual*/
QString CciiFilterChromosome::toNetlist() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    QString str = "Is " + QString::number(isize()) + " "
            + QString::number(_CONST_I(_CONST_E(3))) + " Is\n";
    for(int i = 2; i <= isize(); i += 2) {
        int halfi = i / 2;
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
        QString::number(_CONST_I(_CONST_E(2))) + " " + "CCII\n";

    str += "V1 " + QString::number(isize()) + " 0 0\n";
    str += "V2 " + QString::number(_CONST_I(_CONST_E(2))) + " 0 0\n";

    str += ".LIB CCII\n";
    str += QString(".TF %1 %2\n").arg(output(), input());
    str += ".END";

    return str;
}

/*virtual*/
Circuit* CciiFilterChromosome::toCircuit() const
{
    CONST_SPLIT_IE(this);
    int ncapacitor = ncapacitors();
    Circuit* cir = new Circuit();
    QStringList nodes;

    nodes << QString::number(isize()) << QString::number(_CONST_I(_CONST_E(3)));
    cir->addComponent(new CurrentSource("Is", nodes, "Is"));

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
    nodes << QString::number(_CONST_I(_CONST_E(1))) << "0";
    cir->addComponent(new Resistor("R_Y", nodes, "_inf"));

    nodes.clear();
    nodes << QString::number(_CONST_I(_CONST_E(0))) << "0"
          << QString::number(_CONST_I(_CONST_E(1))) << "0";
    cir->addComponent(new VCVS("E_X", nodes, "1"));

    nodes.clear();
    nodes << "0" << QString::number(_CONST_I(_CONST_E(2)));
    cir->addComponent(new CCCS("F_Z", nodes, "E_X", "1"));

    nodes.clear();
    nodes << QString::number(isize()) << "0";
    cir->addComponent(new VoltageSource("V1", nodes, "0"));

    nodes.clear();
    nodes << QString::number(_CONST_I(_CONST_E(2))) << "0";
    cir->addComponent(new VoltageSource("V2", nodes, "0"));

    cir->do_map();

    return cir;
}

/*virtual*/
int CciiFilterChromosome::esize() const
{
    return 4;
}

/*virtual*/
int CciiFilterChromosome::nShortCircuits() const
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
