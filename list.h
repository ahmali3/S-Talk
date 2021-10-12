/* file: list.h
   Header file for the list concrete data structure*/



/*define the head-nodes*/

typedef struct {
  int next; /* pos of the next head node whether free or not */
  int prev; /* pos of the prev head node whether free or not */ 
  int position; /* pos of the head node */
 
  int first;     /* pos to the firstnode*/
  int current;    /* pos to the current node*/
  int last;   /* pos to the last node*/
  void* curitem; /* pointer to the current item*/
  int mynodes;          /* number of the nodes in the list*/
} LIST;

/*define the nodes of list*/

typedef struct {
  void* item ;  /* the item to be stored*/
  int next;   /* the pos to the next node either free or node in list*/
  int prev;    /* the pos to the prev node either free or node in list*/
} node;

LIST *ListCreate();
/* PRE: none
   POST: if space is available, an empty list is made and a pointer to it, is returned or returns 
   null.*/

int ListCount(LIST* somelist);
/* PRE: somelist is a valid list type structure.
   POST: returns the number of items in list.*/

void *ListFirst(LIST* somelist);
/*PRE: somelist is a valid list type structure.
  POST: returns a pointer to the first item in list and makes the first item the current item.*/

void *ListLast(LIST* somelist);
/*PRE: somelist is a valid list type structure.
  POST: returns a pointer to the last item in list and makes the last item the current item.*/

void *ListNext(LIST* somelist);
/*PRE: somelist is a valid list type structure.
  POST:  advances list's current item by one, and returns a pointer to the new current
         item. If this operation advances the current item beyond the end of the list, a NULL
         pointer is returned. */

void *ListPrev(LIST* somelist);
/*PRE: somelist is a valid list type structure.
  POST: backs up list's current item by one, and returns a pointer to the new current
        item. If this operation backs up the current item beyond the start of the list, a NULL
        pointer is returned. */

void *ListCurr(LIST* somelist);
/*PRE: somelist is a valid list type structure.
  POST: returns a pointer to the current item in list. */

int ListAdd(LIST* somelist, void* item);
/*PRE: somelist is a valid list type structure and item is valid also.
  POST: adds the new item to list directly after the current item, and makes item the current item.
        If the current pointer is before the start of the list, the item is added at the start.
        If the current pointer is beyond the end of the list, the item is added at the end.
        Returns 0 on success, -1 on failure. */

int ListInsert(LIST* somelist, void* item);
/*PRE: somelist is a valid list type structure and item is valid also.
  POST: adds item to list directly before the current item, and makes the new item
        the current one. If the current pointer is before the start of the list, the item is added at the 
        start. If the current pointer is beyond the end of the list, the item is added at the end.
        Returns 0 on success, -1 on failure. */

int ListAppend(LIST* somelist, void* item);
/*PRE: somelist is a valid list type structure and item is valid also.
  POST: adds item to the end of list, and makes the new item the current one.
  Returns 0 on success, -1 on failure. */

int ListPrepend(LIST* somelist, void* item);
/*PRE: somelist is a valid list type structure and item is valid also.
  POST: adds item to the front of list, and makes the new item the current one.
  Returns 0 on success, -1 on failure. */

void *ListRemove(LIST* somelist);
/*PRE: list is a valid list type structure.
  POST: Return current item and take it out of list. Make the next item the current one.*/

void ListConcat(LIST* somelist, LIST* someotherlist);
/*PRE: somelist and someotherlist are valid list type structures.
  POST: adds list2 to the end of list1. The current pointer is set to the current
  pointer of list1. List2 no longer exists after the operation. */

void ListFree(LIST* somelist, void (*itemfree)() );
/*PRE: somelist is a valid list type structure and item is also valid.
  POST: delete list. */

void *ListTrim(LIST* somelist);
/*PRE: somelist is a valid list type structure
  POST: Return last item and take it out of list. Make the new last item the current one. */

void *ListSearch(LIST* somelist, int (*comparator)() , void* comparisonArg);
/*PRE: somelist is a valid list type structure and also the comparator and comparisonArg are valid types.
  POST: searches list starting at the current item until the end is reached or a match is found. */
