#ifndef _RING_H_
#define _RING_H_

#include "global.h"

// Internal data types.
// Do not access them directly
typedef struct ds_ring_node_s {
        pointer_t               mpData;
        struct ds_ring_node_s*  mpNext;
    } ds_ring_node_t;

typedef struct {
        size_t          mSize;
        ds_ring_node_t* mpNode;
    } ds_ring_t;


/**
 *  Creates new ring.
 *
 *  @return Pointer to new ring.
 */
ds_ring_t*
ds_ring_create();


/**
 *  Destroys ring previously created with ds_ring_create().
 *
 *  Data items are not freed. It must be done by the caller.
 *
 *  @param  apRing  Pointer to ring.
 */
void
ds_ring_destroy(ds_ring_t* apRing);


/**
 *  Inserts data item after current item.
 *
 *  @param  apRing  Pointer to ring.
 *  @param  apData  Pointer of data item.
 *  @return TRUE on success, FALSE otherwise.
 */
bool_t
ds_ring_ins(ds_ring_t* apRing, const pointer_t apData);


/**
 *  Deletes current data item.
 *
 *  After deletion current ring data item is next item in the ring. If there are
 *  no more items current data item is NULL.
 *
 *  Item data is not freed. It mus be done by the caller.
 *
 *  @param  apRing  Pointer to ring.
 */
void
ds_ring_del(ds_ring_t* apRing);


/**
 *  Changes corrent ring item to the next item in ring.
 *
 *  @param  apRing  Pointer to ring.
 *  @return Pointer of next data item or NULL if ring is empty.
 */
pointer_t
ds_ring_next(ds_ring_t* apRing);


/**
 *  Returns pointer of current data item.
 *
 *  @param  apRing  Pointer to ring.
 *  @return Pointer of current data item or NULL if ring is empty.
 */
pointer_t
ds_ring_curr(const ds_ring_t* apRing);


/**
 *  Searches for data item.
 *
 *  After search current ring data item points to item that was found or to the
 *  same item if item was not found.
 *
 *  @param  apRing  Pointer to ring.
 *  @param  apData  Pointer of data item to find.
 *  @return Pointer of found data item (that is equal to apData) if
 *                  data item was found or NULL if it was not.
 */
pointer_t
ds_ring_find(ds_ring_t* apRing, const pointer_t apData);


/**
 *  Removes specified data item.
 *
 *  Searches for specified data item and deletes it from ring.
 *
 *  After deletion current ring data item is next item in the ring. If there are
 *  no more items current data item is NULL.
 *
 *  Item data is not freed. It mus be done by the caller.
 *
 *  @param  apRing  Pointer to ring.
 *  @param  apData  Pointer of data item to be removed.
 *  @return TRUE if data item was found and deleted or FALSE otherwise.
 */
bool_t
ds_ring_rem(ds_ring_t* apRing, const pointer_t apData);

#endif // _RING_H_
