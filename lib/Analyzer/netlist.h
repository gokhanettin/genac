#ifndef NETLIST_H
#define NETLIST_H

#include <QtCore/QStack>
#include <QtCore/QStringList>

class QTextStream;
class Circuit;
class AbstractCircuit;


class Netlist
{
public:
    enum OperatorType{
        SUBCKT,
        ENDS,
        LIB,
        TF,
        END
    };
    Netlist();
    void parse(QString* err,const QString& filepath,const QString& libDir = QString());
    Circuit* circuit() const;
    const QList<QStringList>& tfs() const{return m_tfs;}
private:
  void parser(QString* err, QTextStream& netStream, bool parseFlag,
              AbstractCircuit *ac = 0,const QString& libDir = QString());
    OperatorType opType(const QString& str);
    Circuit* m_circuit;
    QList<QStringList> m_tfs;
    QStack<OperatorType> m_stack;
};

#endif // NETLIST_H
