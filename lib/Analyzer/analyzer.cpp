#include "analyzer.h"
#include "circuit.h"
#include "primitive.h"
#include <sstream>
#include "dbg.h"

Analyzer::Analyzer(Circuit* circuit)
    :m_circuit(0),m_A(0),m_X(0),m_Z(0),m_eqnCounter(0)
{
        if(circuit){
        stamp(circuit);
        }
}

Analyzer::~Analyzer()
{
    destroy();
}

uint& Analyzer::eqnCounter()
{
    return m_eqnCounter;
}

uint& Analyzer::N() const
{
    if(!m_circuit)
        qDebug() << "Error: NULL Circuit in Analyzer N!";

        return m_circuit->N();
}

uint& Analyzer::M() const
{
    if(!m_circuit)
        qDebug() << "Error: NULL Circuit in Analyzer M!";

    return m_circuit->M();
}

void Analyzer::stamp(Circuit* circuit)
{
    if(m_circuit)
    {
        destroy();
    }

    m_circuit = circuit;

    if(m_circuit)
    {
        uint n = N();
        uint m = M();
        m_A = new GiNaC::matrix(n+m,n+m);
        m_X = new GiNaC::matrix(n+m,1);
        m_Z = new GiNaC::matrix(n+m,1);

        for(uint i = 1; i<=N(); ++i)
        {
            this->X(i) = GiNaC::realsymbol("V("+ m_circuit->getIndexedNode(i).toStdString()+")");
        }


        m_circuit->do_stamp(this);
    }
    else
    {
        qDebug() << "error at stamp";
        return;
    }
}

void Analyzer::solve()
{
    m_result = m_A->solve(*m_X,*m_Z);
}

 void Analyzer::destroy()
 {
     delete m_A; m_A = 0;
     delete m_X; m_X = 0;
     delete m_Z; m_Z = 0;
     m_circuit = 0;
     m_eqnCounter = 0;
 }

 GiNaC::ex& Analyzer::A(uint i,uint j)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || j <1 || i> m+n || j > m+n)
        {
            qDebug() << "Index Error at A";
        }
        --i;
        --j;

        return (*m_A)(i,j);
 }

 GiNaC::ex& Analyzer::G(uint i,uint j)
 {
     uint n = N();

     if(i < 1 || j < 1 || i>n || j>n )
        {
            qDebug() << "Index Error at G";
        }
       --i;
       --j;

        return (*m_A)(i,j);
 }

 GiNaC::ex& Analyzer::C(uint i,uint j)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || j < 1 || i > m || j > n)
     {
         qDebug() << "Index Error at C";
     }

     i += (n-1);
   --j;

     return (*m_A)(i,j);
 }

 GiNaC::ex& Analyzer::B(uint i,uint j)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || j < 1 || i > n || j > m)
     {
         qDebug() << "Index Error at B";
     }
     --i;
       j += (n-1);
     return (*m_A)(i,j);
 }

 GiNaC::ex& Analyzer::D(uint i,uint j)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || j < 1 || i > m || j > m)
     {
         qDebug() << "Index Error at D";
     }

     i += (n-1);
     j += (n-1);

     return (*m_A)(i,j);
 }

 GiNaC::ex& Analyzer::X(uint i)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || i > m+n)
     {
         qDebug() << "Index Error at X";
     }

     --i;

     return (*m_X)(i,0);
 }

 GiNaC::ex& Analyzer::V(uint i)
 {
     uint n = N();

     if(i  < 1 || i > n)
     {
         qDebug() << "Index Error at V";
     }

     --i;

     return (*m_X)(i,0);
 }

 GiNaC::ex& Analyzer::J(uint i)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || i > m)
     {
         qDebug() << "Index Error at J";
     }

     i += (n-1);

     return (*m_X)(i,0);
 }

 GiNaC::ex& Analyzer::Z(uint i)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || i > m+n)
     {
         qDebug() << "Index Error at Z";
     }
     --i;

     return (*m_Z)(i,0);
 }

 GiNaC::ex& Analyzer::I(uint i)
 {
     uint n = N();

     if(i  < 1 || i > n)
     {
         qDebug() << "Index Error at I";
     }

     --i;

     return (*m_Z)(i,0);
 }

 GiNaC::ex& Analyzer::E(uint i)
 {
     uint n = N();
     uint m = M();

     if(i < 1 || i > m)
     {
         qDebug() << "Index Error at E";
     }

     i += (n-1);

     return (*m_Z)(i,0);
 }


 GiNaC::ex Analyzer::calcVoltage(const QString& exp, bool* found)
 {
     //find from X

     uint i = 0;
     if(found != 0)
        *found = false;
     for(i = 0; i < m_X->rows(); ++i)
     {
        std::stringstream stream;
         stream << (*m_X)(i,0);

         if(exp.toStdString() == stream.str())
         {
             break;
         }
     }

     if(i < m_X->rows()){
         if(found != 0)
            *found = true;

         return m_result(i,0);
     }

     return m_result(0,0);
 }

GiNaC::ex Analyzer::calcCurrent(const QString& exp,bool* found)
 {
     uint siz = exp.size();
   QString compName = exp.mid(2,siz-3);
   //find component
   Primitive* p = (Primitive*)m_circuit->getComponent(compName);


   if(found != 0)
        *found = true;
   if(!p){
       if(found != 0)
        *found = false;

       return GiNaC::ex();
   }
   switch(p->name().at(0).toUpper().toLatin1())
   {
    case 'V':
    case 'E':
    case 'H':
       {
           int i = m_X->rows()-1;
           while(i >= 0)
           {
               std::stringstream stream;
                stream << (*m_X)(i,0);

                if(exp.toStdString() == stream.str())
                {
                    return m_result(i,0);
                }

                --i;
           }
       }
   break;

   }

   Q_ASSERT(p->nodes().size() == 2);

   QString node1 = "V("+p->nodes().at(0)+")";
   QString node2 = "V("+p->nodes().at(1)+")";


   GiNaC::ex v1,v2;
   if(node1 != "V(0)")
         v1 = calcVoltage(node1);
   else
         v1 = 0;

   if(node2 != "V(0)")
         v2 = calcVoltage(node2);
   else
        v2 = 0;

   bool ok;
   double valDouble = p->value().toDouble(&ok);
   if(ok)
   {
       return (v1-v2)/valDouble;
   }

   GiNaC::realsymbol val;
   makeSymbol(val,p->value());

   return (v1-v2)/val;
 }

GiNaC::ex Analyzer::calcValue(const QString& exp)
{
    char ch = exp.at(0).toUpper().toLatin1();
    GiNaC::ex res;
    Q_ASSERT(ch == 'V' || ch == 'I');
    bool found = false;
    switch(ch)
    {
        case 'V':
            res = calcVoltage(exp,&found);
            break;
        case 'I':
            res = calcCurrent(exp,&found);
            break;
    }
    return res;
}

Analyzer::TransferFunction Analyzer::calcTF(const QString& out, const QString& in)
{
    using namespace std;
    GiNaC::ex num,den,num_den;
    TransferFunction tf;
    const GiNaC::realsymbol inf = m_symbols.getInf();
    num = calcValue(out);
    den = calcValue(in);


    tf.rhs = num/den;

    num_den = tf.rhs.numer_denom();

    int deg = num_den.op(0).degree(inf);

    num = num_den.op(0).expand().coeff(inf,deg);
    den = num_den.op(1).expand().coeff(inf,deg);
    if(den == 0){
        tf.rhs = inf;
    }
    else{
        tf.rhs = num/den;
    }

    tf.lhs = GiNaC::realsymbol(out.toStdString()) / GiNaC::realsymbol(in.toStdString());

    return tf;
}

QList<Analyzer::TransferFunction> Analyzer::calcTFs(const QList<QStringList>& tfs)
 {
     using namespace std;
    QList<TransferFunction> list;
    GiNaC::ex num,den,num_den;
    for(int i = 0; i<tfs.size(); ++i)
    {
        TransferFunction tf;
        num = calcValue(tfs.at(i).at(0));
        den = calcValue(tfs.at(i).at(1));

        tf.rhs = num/den;

        num_den = tf.rhs.numer_denom();

        const GiNaC::realsymbol inf = m_symbols.getInf();
        int deg = num_den.op(0).degree(inf);

        num = num_den.op(0).expand().coeff(inf,deg);
        den = num_den.op(1).expand().coeff(inf,deg);
        if(den == 0){
            tf.rhs = GiNaC::realsymbol("_inf");
        }
        else{
            tf.rhs = num/den;
        }


        tf.lhs = GiNaC::realsymbol(tfs.at(i).at(0).toStdString()) / GiNaC::realsymbol(tfs.at(i).at(1).toStdString());

        list.append(tf);
    }

    return list;
 }

