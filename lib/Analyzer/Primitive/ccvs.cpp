#include "ccvs.h"
#include "analyzer.h"
#include "voltagesource.h"
#include "dbg.h"

void CCVS::stamp(Analyzer* a)
{
    if(nodes().size() != 4)
        qDebug() << "Error: Number of CCVS nodes is inconsistent!";

    QStringList dnlist;
    dnlist << nodes().at(2) << nodes().at(3);
    VoltageSource* dv = new VoltageSource("d_"+name(),dnlist,"0");
    dv->setParent(parent());
    dv->stamp(a);
    delete dv;

    uint idx = a->eqnCounter();

    QStringList nlist;
    nlist << nodes().at(0) << nodes().at(1);
    VoltageSource* v = new VoltageSource(name(),nlist,"0");
    v->setParent(parent());
    v->stamp(a);
    delete v;

    bool ok;
    double val = value().toDouble(&ok);
    if(ok)
        a->D(a->eqnCounter(),idx) -= val;
    else
    {
        GiNaC::realsymbol sym;
        a->makeSymbol(sym,value());
        a->D(a->eqnCounter(),idx) -= sym;
    }
}


