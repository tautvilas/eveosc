#include "ring.h"
#include "malloc.h"


ds_ring_t*
ds_ring_create()
{
    ds_ring_t*  pRing   = NEW(ds_ring_t);
    pRing->mSize        = 0;
    pRing->mpNode       = NULL;

    return pRing;
}


inline pointer_t
ds_ring_curr(const ds_ring_t* apRing)
{
    return apRing->mpNode->mpData;
}


void
ds_ring_destroy(ds_ring_t* apRing)
{
    while (ds_ring_curr(apRing))
        ds_ring_del(apRing);

    free(apRing);
}


bool_t
ds_ring_ins(ds_ring_t* apRing, const pointer_t apData)
{
    ds_ring_node_t* pNode   = NEW(ds_ring_node_t);
    if (!pNode)
        return FALSE;

    pNode->mpData   = apData;
    if (apRing->mpNode)
    {
        pNode->mpNext   = apRing->mpNode->mpNext;
        apRing->mpNode->mpNext  = pNode;
    }
    else
    {
        pNode->mpNext   = pNode;
        apRing->mpNode  = pNode;
    }
    apRing->mSize++;
    return TRUE;
}


void
ds_ring_del(ds_ring_t* apRing)
{
    if (!apRing->mpNode)
        return;

    ds_ring_node_t* pNode   = apRing->mpNode;
    if (apRing->mpNode == apRing->mpNode->mpNext)
        apRing->mpNode  = NULL;
    else
        apRing->mpNode  = apRing->mpNode->mpNext;

    free(pNode);
    apRing->mSize--;
}


inline pointer_t
ds_ring_next(ds_ring_t* apRing)
{
    if (!apRing->mpNode)
        return NULL;

    apRing->mpNode  = apRing->mpNode->mpNext;
    return apRing->mpNode->mpData;
}


pointer_t
ds_ring_find(ds_ring_t* apRing, const pointer_t apData)
{
    if (!apRing->mpNode)
        return NULL;

    const ds_ring_node_t* const pStart  = apRing->mpNode;
    do
    {
        if (apData == apRing->mpNode->mpData)
            return apData;

        apRing->mpNode  = apRing->mpNode->mpNext;
    } while (pStart != apRing->mpNode);

    return NULL;
}


bool_t
ds_ring_rem(ds_ring_t* apRing, const pointer_t apData)
{
    if (NULL == ds_ring_find(apRing, apData))
        return FALSE;

    ds_ring_del(apRing);
    return TRUE;
}


