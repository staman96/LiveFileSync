#include "list.h"


/*******************
 * CLASS LIST NODE *
 *******************/

/**
 * Constructor 
 */
List::ListNode::ListNode(DataType *dataptr): dataptr(dataptr), next(NULL) { }


/**************
 * CLASS LIST *
 **************/

/**
 * Constructor 
 */
List::List(): firstNode(NULL), lastNode(NULL), lsize(0) {    
}

/**
 * Constructor 
 */
List::~List(){
    ListNode* tempNode;     // A temp List Node.

    // Loop all the nodes 
    while ( (tempNode = this->firstNode) != NULL) {
        this->firstNode = tempNode->next;  // Store the next node after tempNode as the lists root node.
        delete tempNode;                   // Delete the temp node.
    }

    // Update size
    this->lsize = 0 ;    
}


/**
 * Pushes a DataType in the first place of this List.
 */
void List::pushFront(DataType* dataptr) {
    ListNode* newNode = new ListNode(dataptr);   // Create a new Node.

    // Handle the case where list is empty
    if (this->lastNode == NULL) {
        this->lastNode = newNode;
    }

    // Update the node's next pointer and the lists's firstNode.
    newNode->next = this->firstNode;
    this->firstNode = newNode;
    this->lsize++;
}

/**
 * Pushes a DataType in the last place of this List.
 */
void List::pushBack(DataType* dataptr) {
    ListNode* newNode = new ListNode(dataptr);   // Create a new Node.

    // Handle the case where list is empty
    if (this->firstNode == NULL) {
        this->firstNode = newNode;
    }
    else {
        this->lastNode->next = newNode;
    }

    // Update the node's next pointer and the lists's firstNode.        
    this->lastNode = newNode;
    this->lsize++;
}

/**
 * Pushes a DataType in the place indicated by parameter index.
 */
void List::push(DataType* dataptr, int index) {
    // Based on index push front, push back, or return
    if (index == 0) {
        this->pushFront(dataptr);
        return;
    } else if (index == this->lsize -1) {
        this->pushBack(dataptr);
        return;
    } else if (index >= this->lsize)
        return;

    // If non of the above apply then add dataptr based on parameter index.
    ListNode *newNode = new ListNode(dataptr);   // Create a new Node.
    ListNode *tempNode = this->firstNode;        // A temp node holding the current node in the while loop.    

    // Loop all the nodes 
    for (int idx = 0; idx < this->lsize; idx++) {

        // If we found the right index then return the data.
        if (idx == index) {
            // Place the new node             
            tempNode->next = newNode;            
            newNode->next = tempNode->next;
            
            // Update list size and return
            this->lsize++;            
            return; 
        }
        
        // Update the tempNode
        tempNode = tempNode-> next;
    }    
}

/**
 * Deletes a node with data equal to parameter data
 * Returns if the deletion was successfull.  
 */
bool List::deleteNode(DataType* dataptr) {
    ListNode *tempNode = this->firstNode;  // A temp node holding the current node in the while loop.
    ListNode *prevNode = NULL;          // A temp node holding the node located before the above temp node.
    
    // Loop all the nodes 
    while (tempNode != NULL) {

        // If we find the dataptr delete the node containing it.
        if (tempNode->dataptr == dataptr) {
                        
            // Update the node connected with the node to deleted.            
            if (tempNode == this->firstNode) {
                this->firstNode = tempNode->next;
            }
            else if (tempNode == this->lastNode) {
                this->lastNode = prevNode;
                prevNode->next = NULL;
            } else {
                prevNode->next = tempNode->next;
            }            

            // Delete the node and update the list size.
            delete(tempNode);
            this->lsize--;

            // Return true.
            return true;
        }

        // Update the tempNode and prevNode
        prevNode = tempNode;
        tempNode = tempNode-> next;
    }

    // Return false.
    return false;
}


/**
 * Deletes a node located in index equal to parameter index
 * Returns if the deletion was successfull.  
 */
bool List::deleteNode(int index) {
    // Return false in case index >= size
    if (index >= this->lsize) { return false; }

    ListNode *tempNode = this->firstNode;  // A temp node holding the current node in the while loop.
    ListNode *prevNode = NULL;          // A temp node holding the node located before the above temp node.    
    
    // Loop all the nodes 
    for (int idx = 0; idx < this->lsize; idx++) {

        // If we find the dataptr delete the node containing it.
        if (idx == index) {

            // Update the node connected with the node to deleted.
            if (tempNode == this->firstNode) {
                this->firstNode = tempNode->next;
            }
            else if (tempNode == this->lastNode) {
                this->lastNode = prevNode;
                prevNode->next = NULL;
            } else {
                prevNode->next = tempNode->next;
            }            

            // Delete the node and update the list size.
            delete(tempNode);
            this->lsize--;

            // Return true.
            return true;
        }

        // Update the tempNode and prevNode
        prevNode = tempNode;
        tempNode = tempNode-> next;
    }
    
    // Return false
    return false;
}


/**
 * Prints the list
 */
void List::print() {
    ListNode* tempNode = this->firstNode;     // A temp List Node.

    // Loop all the nodes 
    int count = 0;
    cout << "LIST\n-size: " << this->lsize << std::endl; 
    while (tempNode != NULL) {
        cout << "-Node[" << count << "]: " << *tempNode->dataptr << std::endl;
        tempNode = tempNode->next;
        count++;
    }
}

/**
 * Return the size of this list.
 */
int List::size(){
    return this->lsize;
}

/**
 * Return:
 * - In case idx <  size : the object located in place indicated by parameter index
 * - In case idx >= size : NULL
 */
DataType* List::get(int index) {
    // Return NULL in case index >= size
    if (index >= this->lsize) { return NULL; }

    ListNode* tempNode = this->firstNode;     // A temp List Node.
    int idx = 0;                              // An index to traverse the list.

    // Loop all the nodes
    while (tempNode != NULL) { 

        // If we found the right index then return the data.
        if (idx == index) {
            return tempNode->dataptr;
        }

        // Update the variables
        tempNode = tempNode->next;        
        idx++;
    }

    // Return null
    return NULL;
}


// void List::enhanceList(List& clist ) {
//     ListNode * nodeptr = clist.FLNode ;
//     Size = Size + clist.Size ;                  //prosthetw ta size twn duo list
//     while ( nodeptr != NULL ){                  //oso exei xtoixeia h clist akoma
//             pushfront( nodeptr->data ) ;        //ta kanw pushfront sthn lista mou, dld thn lista gia thn opoia kalesa ton cp constructor
//             nodeptr = nodeptr->next ;
//     }
// }



