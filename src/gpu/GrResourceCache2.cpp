
/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#include "GrResourceCache2.h"
#include "GrGpuResource.h"  
#include "SkRefCnt.h"

GrResourceCache2::~GrResourceCache2() {
    this->releaseAll();
}

void GrResourceCache2::insertResource(GrGpuResource* resource) {
    SkASSERT(resource);
    SkASSERT(!resource->wasDestroyed());
    SkASSERT(!this->isInCache(resource));
    fResources.addToHead(resource);
    ++fCount;
    if (!resource->getScratchKey().isNullScratch()) {
        fScratchMap.insert(resource->getScratchKey(), resource);
    }
}

void GrResourceCache2::removeResource(GrGpuResource* resource) {
    SkASSERT(this->isInCache(resource));
    fResources.remove(resource);    
    if (!resource->getScratchKey().isNullScratch()) {
        fScratchMap.remove(resource->getScratchKey(), resource);
    }
    --fCount;
}

void GrResourceCache2::abandonAll() {
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->abandon();
        // abandon should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fCount);
}

void GrResourceCache2::releaseAll() {
    while (GrGpuResource* head = fResources.head()) {
        SkASSERT(!head->wasDestroyed());
        head->release();
        // release should have already removed this from the list.
        SkASSERT(head != fResources.head());
    }
    SkASSERT(!fScratchMap.count());
    SkASSERT(!fCount);
}

class GrResourceCache2::AvailableForScratchUse {
public:
    bool operator()(const GrGpuResource* resource) const {
        // Because duties are currently shared between GrResourceCache and GrResourceCache2, the
        // current interpretation of this rule is that only GrResourceCache has a ref but that
        // it has been marked as a scratch resource.
        return resource->reffedOnlyByCache() && GrIORef::kYes_IsScratch == resource->fIsScratch;
    }
};

GrGpuResource* GrResourceCache2::findAndRefScratchResource(const GrResourceKey& scratchKey) {
    SkASSERT(scratchKey.isScratch());
    return SkSafeRef(fScratchMap.find(scratchKey, AvailableForScratchUse()));
}
