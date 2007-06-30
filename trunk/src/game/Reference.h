#ifndef _REFERENCE_H
#define _REFERENCE_H

#include "LinkedList.h"

//=====================================================

class BaseReference : public LinkedListElement {
public:
    virtual void invalidate() = 0;
};

//=====================================================

template <class TO, class FROM> class Reference : public BaseReference
{
private:
     TO* iRefTo;
     FROM* iRefFrom;
private:
    void clear() { delink(); iRefTo = NULL; iRefFrom = NULL; }
protected:
    // Tell our refTo (target) object that we have a link
    virtual void targetObjectBuildLink() = 0;

    // Tell our refTo (taget) object, that the link is cut
    virtual void targetObjectDestroyLink() = 0;

    // Tell our refFrom (source) object, that the link is cut (Target destroyed)
    virtual void sourceObjectDestroyLink() = 0;
public:
    Reference() { iRefTo = NULL; iRefFrom = NULL; }

    // Create new link
    inline void link(TO* toObj, FROM* fromObj) { 
        if(isValid())
            unlink();
        if(toObj != NULL) {
            iRefTo = toObj;
            iRefFrom = fromObj;
            targetObjectBuildLink();
        }
    }

    // We don't need the reference anymore. Call comes from the refFrom object
    // Tell our refTo object, that the link is cut
    inline void unlink() { targetObjectDestroyLink(); clear(); }

    // Link is invalid due to destruction of referenced traget object. Call comes from the refTo object
    // Tell our refFrom object, that the link is cut
    inline void invalidate() { sourceObjectDestroyLink(); clear(); }

    inline bool isValid() const { return iRefTo != NULL; }


    Reference<TO,FROM>* next() { return((Reference<TO,FROM>*)LinkedListElement::next()); }
    Reference<TO,FROM>* prev() { return((Reference<TO,FROM>*)LinkedListElement::prev()); }

    inline TO* operator ->() const { return iRefTo; }
    inline TO* getTarget() const { return iRefTo; }

    inline FROM* getSource() const { return iRefFrom; }
};

//=====================================================
#endif
