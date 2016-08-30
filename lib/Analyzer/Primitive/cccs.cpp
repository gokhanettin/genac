#include "cccs.h"
#include "analyzer.h"
#include "voltagesource.h"
#include "dbg.h"

void CCCS::stamp(Analyzer* a)
{
    int len = nodes().size();

    if(len != 4)
        qDebug() << "Error: Number of CCCS nodes is inconsistent!";

    uint idx[2];
    bool ok = true;
    for(int i = 0; i<2; ++i)
    {
        idx[i] = parent()->index(nodes().at(i),&ok);
        if(!ok)
        {
            qDebug() << "Error: Unknown Node!";
            return;
        }
    }

    QStringList nlist;
    nlist << nodes().at(2)<<nodes().at(3);
    VoltageSource* v = new VoltageSource(name(),nlist,"0");
    v->setParent(parent());
    v->stamp(a);
    delete v;

    double val = value().toDouble(&ok);
    if(ok)
    {
        if(idx[0] != 0)
            a->B(idx[0],a->eqnCounter()) = val;
        if(idx[1] != 0)
            a->B(idx[1],a->eqnCounter()) = -1*val;
    }
    else
    {
        GiNaC::realsymbol sym;
        a->makeSymbol(sym,value());
        if(idx[0] != 0)
            a->B(idx[0],a->eqnCounter()) = sym;
        if(idx[1] != 0)
            a->B(idx[1],a->eqnCounter()) = -1*sym;
    }

}
