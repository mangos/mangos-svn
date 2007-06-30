#ifndef _REFMANAGER_H
#define _REFMANAGER_H
//=====================================================

#include "LinkedList.h"
#include "Reference.h"

template <class TO, class FROM> class RefManager : public LinkedListHead
{
private:
    void clearReferences()
    {
        LinkedListElement* ref;
        while((ref = getFirst()) != NULL) {
            ((BaseReference*) ref)->invalidate();
            ref->delink(); // the delink might be already done by invalidate(), but doing it here again does not hurt and insures an empty list
        }
    }
public:
    RefManager() { }
    virtual ~RefManager() { clearReferences(); }

    Reference<TO, FROM>* getFirst() { return ((Reference<TO, FROM>* ) LinkedListHead::getFirst()); }
    Reference<TO, FROM>* getLast() { return ((Reference<TO, FROM>* ) LinkedListHead::getLast()); }
};

//=====================================================

#endif
