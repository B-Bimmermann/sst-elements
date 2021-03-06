// Copyright 2009-2017 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
//
// Copyright (c) 2009-2017, Sandia Corporation
// All rights reserved.
//
// Portions are copyright of other developers:
// See the file CONTRIBUTORS.TXT in the top level directory
// the distribution for more information.
//
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.


#ifndef _H_EMBER_ALLTOALLV_MOTIF
#define _H_EMBER_ALLTOALLV_MOTIF

#include "mpi/embermpigen.h"

namespace SST {
namespace Ember {

class EmberAlltoallvGenerator : public EmberMessagePassingGenerator {

public:
	EmberAlltoallvGenerator(SST::Component* owner, Params& params);
    bool generate( std::queue<EmberEvent*>& evQ);

private:
	uint32_t m_iterations;
    uint32_t m_loopIndex;
    int      m_count;

    void*    m_sendBuf;
    void*    m_recvBuf;
    std::vector<int>    m_sendCnts;
    std::vector<int>    m_sendDsp;
    std::vector<int>    m_recvCnts;
    std::vector<int>    m_recvDsp;
};

}
}

#endif
