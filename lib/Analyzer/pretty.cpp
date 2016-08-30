#include "pretty.h"
#include <sstream>

QString pretty(const GiNaC::ex &expr, bool complex, bool islhs)
{
    std::stringstream stream_num;
    std::stringstream stream_den;
    GiNaC::ex num_den = expr.numer_denom();
    stream_num << num_den.op(0);
    if(num_den.op(1).is_equal(1)){
        stream_num << "\n";
        return "*  "+ QString::fromStdString(stream_num.str());
    }
    stream_den << num_den.op(1);
    QString num = QString::fromStdString(stream_num.str());
    QString den = QString::fromStdString(stream_den.str());
    int max = 0;
    int min = 0;
    if(num.size() > den.size()){
        max = num.size();
        min = den.size();
    }
    else{
        max = den.size();
        min = num.size();
    }
    QString str(max+1,QChar('-'));
    if(islhs){
        str.append(" =");
    }
    str.append("\n");
    num.append("\n");
    den.append("\n");

    QString pre((max-min)/2, ' ');
    if(num.size() < den.size()){
        num.prepend(pre);
    }
    else{
        den.prepend(pre);
    }

    if(complex){
        den.replace(QChar('I'),QChar('j'));
        num.replace(QChar('I'),QChar('j'));
        //den.replace(QChar('w'),QChar(0x03c9)); // Does not work!
        //num.replace(QChar('w'),QChar(0x03c9)); //replace with small omega
    }

    return "*  " + num + "*  " + str + "*  " + den;
}

QString pretty(const Equation& eq, bool complex)
{
    return pretty(eq.lhs,complex,true) + pretty(eq.rhs,complex,false);
}

