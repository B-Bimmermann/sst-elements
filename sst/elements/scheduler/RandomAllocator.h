// Copyright 2009-2013 Sandia Corporation. Under the terms
// of Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2013, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.

/*
 * Allocator for SimpleMachine- no representation of locality so just
 * keep track of the number of free processors
 */

#ifndef SST_SCHEDULER_RANDOMALLOCATOR_H__
#define SST_SCHEDULER_RANDOMALLOCATOR_H__

#include <string>
#include <vector>

#include "sst/core/serialization.h"

#include "Allocator.h"


namespace SST {
    namespace Scheduler {
        class Machine;
        class Job;

        class RandomAllocator : public Allocator {

            public:
                RandomAllocator(Machine* mesh);

                RandomAllocator Make(std::vector<std::string*>* params);

                std::string getParamHelp();

                std::string getSetupInfo(bool comment);

                AllocInfo* allocate(Job* job);
        };

    }
}
#endif

