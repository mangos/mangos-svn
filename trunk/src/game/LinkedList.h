#ifndef _LINKEDLIST
#define _LINKEDLIST

#include "Common.h"

//============================================
class LinkedListHead;

class LinkedListElement {
private:
    friend class LinkedListHead;

    LinkedListElement* iNext;
    LinkedListElement* iPrev;
public:
    LinkedListElement() { iNext = NULL; iPrev = NULL; }
    ~LinkedListElement() { delink(); }

    bool hasNext() { return(iNext->iNext != NULL); }
    bool hasPrev() { return(iPrev->iPrev != NULL); }
    bool isInList() { return(iNext != NULL && iPrev != NULL); }

    LinkedListElement* next() { return hasNext() ? iNext : NULL; }
    LinkedListElement* prev() { return hasPrev() ? iPrev : NULL; }

    void delink() { if(isInList()) { iNext->iPrev = iPrev; iPrev->iNext = iNext; iNext = NULL; iPrev = NULL; } }

    void insertBefore(LinkedListElement* pElem)
    {
        pElem->iNext = this;
        pElem->iPrev = iPrev;
        iPrev->iNext = pElem;
        iPrev = pElem;
    }

    void insertAfter(LinkedListElement* pElem)
    {
        pElem->iPrev = this;
        pElem->iNext = iNext;
        iNext->iPrev = pElem;
        iNext = pElem;
    }
};

//============================================

class LinkedListHead {
private:
    LinkedListElement iFirst;
    LinkedListElement iLast;
public:
    LinkedListHead()
    {
        // create empty list

        iFirst.iNext = &iLast;
        iLast.iPrev = &iFirst;
    }

    bool isEmpty() { return(!iFirst.iNext->isInList()); }

    LinkedListElement* getFirst() { return(isEmpty() ? NULL : iFirst.iNext); }

    LinkedListElement* getLast() { return(isEmpty() ? NULL : iLast.iPrev); }

    void insertFirst(LinkedListElement* pElem)
    {
        iFirst.insertAfter(pElem);
    }

    void insertLast(LinkedListElement* pElem)
    {
        iLast.insertBefore(pElem);
    }

    uint32 getSize()
    {
        uint32 result = 0;
        LinkedListElement* e = getFirst();
        while(e) {
            ++result;
            e = e->next();
        }
        return result;
    }
};

//============================================

#endif
