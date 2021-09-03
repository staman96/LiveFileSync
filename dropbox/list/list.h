#ifndef LISTS_H_INCLUDED
#define LISTS_H_INCLUDED

#include <iostream>

using std::cout;

// Change to whatever you want
typedef int DataType;

class List {
  private :

    class ListNode {
      public:
        DataType* dataptr;
        ListNode* next;

        /** Constructor */
        ListNode(DataType* dataptr); 
   };

    ListNode*  firstNode;    // Points to the first node of the list
    ListNode*  lastNode;     // Points to the last node of the list
    int        lsize;         // The size of the list.
    
  public :

    /** Constructor & Destructor */
    List();
    ~List();      

    /** Accessors */
    void print();
    int size();
    DataType* get(int index);

    /** Mutators */
    void pushFront(DataType* dataptr);
    void pushBack(DataType* dataptr);
    void push(DataType* dataptr, int index);
    bool deleteNode(DataType* dataptr);
    bool deleteNode(int index);
} ;



#endif // LISTS_H_INCLUDED
