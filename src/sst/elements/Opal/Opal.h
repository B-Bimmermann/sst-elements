// Copyright 2009-2017 Sandia Corporation. Under the terms
// of Contract DE-NA0003525 with Sandia Corporation, the U.S.
// Government retains certain rights in this software.
// 
// Copyright (c) 2009-2017, Sandia Corporation
// All rights reserved.
// 
// This file is part of the SST software package. For license
// information, see the LICENSE file in the top level directory of the
// distribution.
//


/* Author: Amro Awad
 * E-mail: amro.awad@ucf.edu
 */



#include <sst/core/sst_types.h>
#include <sst/core/event.h>
#include <sst/core/component.h>
#include <sst/core/link.h>
#include <sst/core/timeConverter.h>
#include <sst/core/interfaces/simpleMem.h>

#include <sst/core/output.h>
#include "sst/core/elementinfo.h"
#include <cstring>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include <stdio.h>
#include <stdint.h>
#include <poll.h>
#include "mempool.h"


using namespace std;
using namespace SST;

//using namespace SST::OpalComponent;

namespace SST {
	namespace OpalComponent {

		class NVM_PARAMS;

		class Opal : public SST::Component {
			public:

				Opal( SST::ComponentId_t id, SST::Params& params); 
				void setup()  { };
				void finish() {};
				void handleEvent(SST::Event* event) {};
				bool tick(SST::Cycle_t x);

				void parser(NVM_PARAMS * nvm, SST::Params& params);				

				SST_ELI_REGISTER_COMPONENT(
						Opal,
						"Opal",
						"Opal",
						SST_ELI_ELEMENT_VERSION(1,0,0),
						"Memory Allocation Manager",
						COMPONENT_CATEGORY_PROCESSOR
						)


					SST_ELI_DOCUMENT_PARAMS(
							{"clock",              "Internal Controller Clock Rate.", "1.0 Ghz"},
							{"num_pools",          "This determines the number of memory pools", "1"},
							{"num_domains", "The number of domains in the system, typically similar to number of sockets/SoCs", "1"},
							{"allocation_policy",          "0 is private pools, then clustered pools, then public pools", "0"},
							{"size%(num_pools)", "Size of each memory pool in KBs", "8388608"},
							{"startaddress%(num_pools)", "the starting physical address of the pool", "0"},
							{"type%(num_pools)", "0 means private for specific NUMA domain, 1 means shared among specific NUMA domains, 2 means public", "2"},
							{"cluster_size", "This determines the number of NUMA domains in each cluster, if clustering is used", "1"},
							{"memtype%(num_pools)", "0 for typical DRAM, 1 for die-stacked DRAM, 2 for NVM", "0"},
							{"typepriority%(num_pools)", "0 means die-stacked, typical DRAM, then NVM", "0"},
							{ NULL, NULL }
							)

					// Optional since there is nothing to document
					SST_ELI_DOCUMENT_STATISTICS(
							)

					SST_ELI_DOCUMENT_PORTS(
							{"Nlink", "Link to the simpleComponent to the North", { "simpleComponent.simpleComponentEvent", "" } },
							{"Slink", "Link to the simpleComponent to the South", { "simpleComponent.simpleComponentEvent", "" } },
							{"Elink", "Link to the simpleComponent to the East",  { "simpleComponent.simpleComponentEvent", "" } },
							{"Wlink", "Link to the simpleComponent to the West",  { "simpleComponent.simpleComponentEvent", "" } }
							)

					// Optional since there is nothing to document
					SST_ELI_DOCUMENT_SUBCOMPONENT_SLOTS(
							)

			private:
					Opal();  // for serialization only
					Opal(const Opal&); // do not implement
					void operator=(const Opal&); // do not implement

					int create_pinchild(char* prog_binary, char** arg_list){return 0;}

					SST::Link * m_memChan; 

					SST::Link * event_link; // Note that this is a self-link for events

					Pool * mempools; // This represents the memory pools of the system

					long long int max_inst;
					char* named_pipe;
					int* pipe_id;
					std::string user_binary;
					Output* output;


					Statistic<long long int>* statReadRequests;
					Statistic<long long int>* statWriteRequests;
					Statistic<long long int>* statAvgTime;



		};

	}
}
