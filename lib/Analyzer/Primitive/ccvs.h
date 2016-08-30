#ifndef CCVS_H
#define CCVS_H

#include "primitive.h"
#include "mapper.h"
class CCVS : public Primitive
{
public:
    CCVS(const QString& name = QString(),
              const QStringList& nodes = QStringList(),
              const QString& value = QString())
        :Primitive(name,nodes,value){}

    virtual void stamp(Analyzer *a);
};

#endif // CCVS_H
