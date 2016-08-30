#include "circuit.h"
#include "component.h"
#include "dbg.h"

void Circuit::do_map()
{
    for(int i = 0; i<m_components.size(); ++i)
        m_components[i]->map(this);
}

void Circuit::do_stamp(Analyzer* a)
{
    for(int i = 0; i<m_components.size(); ++i)
    {
        m_components[i]->stamp(a);
    }
}

#ifdef PRINTABLE_MAPABLES
void Circuit::do_print() const
{
    for(int i = 0; i<m_components.size(); ++i)
        m_components[i]->print();
}
#endif
