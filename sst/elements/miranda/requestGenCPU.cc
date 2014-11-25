
#include <sst_config.h>
#include "requestGenModule.h"
#include "requestGenCPU.h"

RequestGenCPU::RequestGenCPU(SST::ComponentId_t id, SST::Params& params) {
	const int verbose = (int) params.find_integer("verbose", 0);
	out = new Output("RequestGenCPU[@p:@l]: ", verbose, 0, SST::Output::STDOUT);

	maxRequestsPending = (uint32_t) params.find_integer("maxmemoryreqs", 16);
	requestsPending = 0;

	out->verbose(CALL_INFO, 1, 0, "Configured CPU to allow %" PRIu32 " maximum requests to be memory to be outstanding.\n",
		maxRequestsPending);

	std::string reqGenModName = params.find_string("generator", "");

	out->verbose(CALL_INFO, 1, 0, "Request generator to be loaded is: %s\n", reqGenModName.c_str());
	Params genParams = params.find_prefix_params("generatorParams.");
	reqGen = dynamic_cast<RequestGenerator*>( loadModuleWithComponent(reqGenModName, this, genParams) );

	if(NULL == reqGen) {
		out->fatal(CALL_INFO, -1, "Failed to load generator: %s\n", reqGenModName.c_str());
	} else {
		out->verbose(CALL_INFO, 1, 0, "Generator loaded successfully.\n");
	}

	std::string interfaceName = params.find_string("memoryinterface", "memHierarchy.memInterface");
	out->verbose(CALL_INFO, 1, 0, "Memory interface to be loaded is: %s\n", interfaceName.c_str());

	Params interfaceParams = params.find_prefix_params("memoryinterfaceparams.");
	memory = dynamic_cast<SimpleMem*>( loadModuleWithComponent(interfaceName.c_str(), this, interfaceParams) );
	memory->initialize("cache_link", new SimpleMem::Handler<RequestGenCPU>(this, &RequestGenCPU::handleEvent) );

	if(NULL == memory) {
		out->fatal(CALL_INFO, -1, "Failed to load interface: %s\n", interfaceName.c_str());
	} else {
		out->verbose(CALL_INFO, 1, 0, "Loaded memory interface successfully.\n");
	}

	std::string cpuClock = params.find_string("clock", "2GHz");
	registerClock(cpuClock, new Clock::Handler<RequestGenCPU>(this, &RequestGenCPU::clockTick));

	out->verbose(CALL_INFO, 1, 0, "CPU clock configured for %s\n", cpuClock.c_str());
	registerAsPrimaryComponent();
	primaryComponentDoNotEndSim();

	out->verbose(CALL_INFO, 1, 0, "Configuration completed.\n");
}

RequestGenCPU::~RequestGenCPU() {
	delete memory;
	delete reqGen;
}

void RequestGenCPU::finish() {

}

void RequestGenCPU::init(unsigned int phase) {

}

void RequestGenCPU::handleEvent( Interfaces::SimpleMem::Request* ev) {
	out->verbose(CALL_INFO, 2, 0, "Recv event for processing from interface\n");

	// Decrement pending requests, we have recv'd a response
	outstandingRequests--;
}

void RequestGenCPU::issueRequest(const RequestGeneratorRequest* req) {
	const uint64_t reqAddress = req->getAddress();
	const uint64_t reqLength  = req->getLength();
	bool isRead               = req->isRead();
	const uint64_t lineOffset = reqAddress % cacheLine;

	out->verbose(CALL_INFO, 4, 0, "Issue request: address=%" PRIu64 ", length=%" PRIu64 ", operation=%s, cache line offset=%" PRIu64 "\n",
		reqAddress, reqLength, (isRead ? "READ" : "WRITE"), lineOffset);

	if(lineOffset + entryLength > cacheLine) {
		// Request is for a split operation (i.e. split over cache lines)
		const uint64_t lowerlength = cacheLineSize - lineOffset;
		const uint64_t upperLength = entryLength - lowerLength;

		// Ensure that lengths are calculated correctly.
		assert(lowerLength + upperLength == reqLength);

		const uint64_t lowerAdress = entryAddress;
		const uint64_t upperAddress = (lowerAddress - lowerAddress % cacheLine) +
						cacheLine;

		out->verbose(CALL_INFO, 4, 0, "Issuing a split cache line operation:\n");
		out->verbose(CALL_INFO, 4, 0, "L -> Address: %" PRIu64 ", Length=%" PRIu64 "\n",
			lowerAddress, lowerLength);
		out->verbose(CALL_INFO, 4, 0, "U -> Address: %" PRIu64 ", Length=%" PRIu64 "\n",
			upperAddress, upperLength);

		SimpleMem::Request* reqLower = new SimpleMem::Request(
			isRead ? SimpleMem::Request::Read : SimpleMem::Request::Write,
			lowerAddress, lowerLength);

		SimpleMem::Request* reqUpper = new SimpleMem::Request(
			isRead ? SimpleMem::Request::Read : SimpleMem::Request::Write,
			upperAddress, upperLength);

		out->verbose(CALL_INFO, 4, 0, "Issuing requesting into cache link...\n");
		cache_link->sendRequest(reqLower);
		cache_link->sendRequest(reqUpper);
		out->verbose(CALL_INFO, 4, 0, "Completed issue.\n");

		requestsPending += 2;
	} else {
		// This is not a split laod, i.e. issue in a single transaction
		SimpleMem::Request* request = new SimpleMem::Request(
			isRead ? SimpleMem::Request::Read : SimpleMem::Request::Write,
			reqAddress, reqLength);

		cache_link->sendRequest(request);

		requestsPending++;
	}

	// Delete the request
	delete req;
}

bool RequestGenCPU::clockTick(SST::Cycle_t cycle) {
	if(reqGen->isFinished()) {
		// Deregister here
		primaryComponentOKToEndSim();
		return true;
	}

	if(NULL == nextReq) {
		nextReq = reqGen->nextRequest();

		if(NULL == nextReq) {
			out->fatal(CALL_INFO, -1, "Request generator returned a NULL when getting the next request\n");
		}
	}

	// Process the request which may require splitting into multiple
	// requests (if breaks over a cache line)
	out->verbose(CALL_INFO, 2, 0, "Requests pending %" PRIu32 ", maximum permitted %" PRIu32 ".\n",
		requestsPending, maxOutstanding);

	if(requestsPending < maxOutstanding) {
		out->verbose(CALL_INFO, 2, 0, "Will attempt to issue as free slots in load/store unit.\n");
		issueRequest(nextReq);
	} else {
		out->verbose(CALL_INFO, 4, 0, "Will not issue, not free slots in load/store unit.\n");
	}

	return false;
}
