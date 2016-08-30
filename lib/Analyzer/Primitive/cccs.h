#ifndef CCCS_H
#define CCCS_H

#include "primitive.h"
#include "mapper.h"

class CCCS : public Primitive
{
public:
    CCCS(const QString& name = QString(),
              const QStringList& nodes = QStringList(),
              const QString& value = QString())
        :Primitive(name,nodes,value){}

    virtual void stamp(Analyzer *a);
};

#endif // CCCS_H
