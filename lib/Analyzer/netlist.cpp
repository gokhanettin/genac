#include "netlist.h"
#include "circuit.h"
#include "subcircuit.h"
#include "x.h"
#include "Primitive/y.h"
#include "Primitive/resistor.h"
#include "Primitive/capacitor.h"
#include "Primitive/inductor.h"
#include "Primitive/voltagesource.h"
#include "Primitive/currentsource.h"
#include "Primitive/cccs.h"
#include "Primitive/ccvs.h"
#include "Primitive/vcvs.h"
#include "Primitive/vccs.h"
#include <QtCore/QStringList>
#include <QtCore/QFile>
#include <QtCore/QIODevice>
#include <QtCore/QTextStream>
#include "dbg.h"



Netlist::Netlist()
    :m_circuit(new Circuit())
{}

void Netlist::parse(QString* err,const QString& filepath, const QString& libDir)
{
    QFile file(filepath);
    if(file.exists())
    {
       if(file.open(QIODevice::ReadOnly | QIODevice::Text))
       {
           QTextStream stream(&file);
           err->clear();
           parser(err, stream, true, 0, libDir);
       }
       if(err->isEmpty()){ //No error occured.
            m_circuit->do_map();
       }
    }
    else{
        *err = "Netlist file '" + filepath + "' not found!";
    }
}

Circuit* Netlist::circuit() const
{
   return m_circuit;
}

Netlist::OperatorType Netlist::opType(const QString& str)
{
    QStringList opList;
    opList << ".SUBCKT"
           << ".ENDS"
           << ".LIB"
           << ".TF"
           << ".END";

    int i = 0;
    for(i = 0; i<opList.size(); ++i)
    {
        if(str.compare(opList.at(i),Qt::CaseInsensitive) == 0)
        {
            break;
        }
    }

    return static_cast<OperatorType>(i);
}

void Netlist::parser(QString* err, QTextStream& netStream, bool parseFlag,
                     AbstractCircuit *ac,const QString& libDir)
{

    if(!ac)
        ac = m_circuit;


    if(netStream.atEnd())
    {
        if(parseFlag){
            if(m_stack.size() != 1 || m_stack.pop() != END)
            {
                qDebug() << "Netlist must end with .END";
            }
        }

            return;
    }
    else
    {
        QString line = netStream.readLine().trimmed();

        if(!line.isEmpty() && line.at(0) != '*')
        {
            QStringList tokens = line.split(" ",QString::SkipEmptyParts);
            switch(tokens.at(0).at(0).toUpper().toLatin1())
            {
            case 'Y':
                if(tokens.size() == 4)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString value = tokens[3];
                    ac->addComponent(new Y(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: Y expects 4 parameters!";
                }

                break;
            case 'R':
                if(tokens.size() == 4)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString value = tokens[3];
                    ac->addComponent(new Resistor(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: Resistor expects 4 parameters!";
                }

                break;
            case 'C':
                if(tokens.size() == 4)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString value = tokens[3];
                    ac->addComponent(new Capacitor(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: Capacitor expects 4 parameters!";
                }

                break;
            case 'L':
                if(tokens.size() == 4)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString value = tokens[3];
                    ac->addComponent(new Inductor(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: Inductor expects 4 parameters!";
                }

                break;
            case 'V':
                if(tokens.size() == 4)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString value = tokens[3];
                    ac->addComponent(new VoltageSource(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: Voltage Source expects 4 parameters!";
                }

                break;
            case 'I':
                if(tokens.size() == 4)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString value = tokens[3];
                    ac->addComponent(new CurrentSource(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: Current Source expects 4 parameters!";
                }

                break;
            case 'E':
                if(tokens.size() == 6)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2]<<tokens[3]<<tokens[4];
                    QString value = tokens[5];
                    ac->addComponent(new VCVS(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: E expects 6 parameters!";
                }
                break;
            case 'F':
                if(tokens.size() == 5)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString voltageName = tokens[3];
                    QString value = tokens[4];
                    ac->addComponent(new CCCS(name, nodes, voltageName, value));
                }
                else
                {
                    qDebug()<< "Error: F expects 5 parameters!";
                }
                break;
            case 'G':
                if(tokens.size() == 6)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2]<<tokens[3]<<tokens[4];
                    QString value = tokens[5];
                    ac->addComponent(new VCCS(name,nodes,value));
                }
                else
                {
                    qDebug()<< "Error: G expects 6 parameters!";
                }
                break;
            case 'H':
                if(tokens.size() == 5)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    nodes << tokens[1] << tokens[2];
                    QString voltageName = tokens[3];
                    QString value = tokens[4];
                    ac->addComponent(new CCVS(name, nodes, voltageName, value));
                }
                else
                {
                    qDebug()<< "Error: H expects 5 parameters!";
                }
                break;
            case 'X':
                if(tokens.size() > 3)
                {
                    QString name = tokens[0];
                    QStringList nodes;
                    for(int i = 1; i<tokens.size()-1; ++i)
                    {
                        nodes << tokens[i];
                    }
                    QString protoName = tokens[tokens.size()-1];
                    ac->addComponent(new X(name,nodes,protoName));
                }
                else
                {
                    qDebug() << "Error: Unexpected component with number of nodes smaller than 3!";
                }
                break;
            case '.':
            {
                OperatorType op = opType(tokens[0]);
                switch(op)
                {
                case SUBCKT:
                {
                    m_stack.push(op);
                    QString name = tokens.at(1);
                    QStringList nodes;
                    for(int i = 2; i<tokens.size(); ++i)
                    {
                        nodes << tokens.at(i);
                    }
                    Subcircuit* sub = new Subcircuit(name,nodes);
                    ac->addSubcircuit(sub);
                    parser(err, netStream, parseFlag, sub, libDir);
                }
                    break;
                case ENDS:
                    if(m_stack.pop() != SUBCKT)
                    {
                        qDebug() << "Error: .SUBCKT and .ENDS does not match!";
                    }
                    return;
                    break;
                case LIB:
                {
                    if(tokens.size() > 2) {
                        *err = ".LIB expects two tokens only.";
                        return;
                    }
                    if (libDir.isEmpty()) {
                        *err = "No Library path specified";
                        return;
                    }
                    QString libd = libDir;
                    if (libDir.at(libDir.size() - 1) !=  '/') {
                        libd += QString("/");
                    }

                    QFile libFile(libd + tokens[1]);
                    if(libFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        QTextStream libStream(&libFile);
                        parser(err, libStream, false, ac, libd);
                    }
                    else {
                        *err = libd + tokens[1] + " does not exist!";
                        return;
                    }
                }
                    break;
                case TF:
                {
                    QStringList l;
                    l<<tokens.at(1)<<tokens.at(2);
                    m_tfs.append(l);

                }
                break;

                case END:
                    m_stack.push(END);
                    break;
                }
            }

                break;
            default:
                qDebug() << "Error: UNknown Component Type!";
                break;
            }
        }

        parser(err, netStream, parseFlag, ac, libDir);
    }
}
