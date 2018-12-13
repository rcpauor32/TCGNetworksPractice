#pragma once
#include "Agent.h"

// Forward declaration
class MCP;
using MCPPtr = std::shared_ptr<MCP>;

class UCP :
	public Agent
{
public:

	// Constructor and destructor
	UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLoc, unsigned int searchDepth);
	~UCP();

	// Agent methods
	void update() override;
	void stop() override;
	UCP* asUCP() override { return this; }
	void OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream) override;
	bool SendItemRequest();
	bool SendConstraintResult(bool res);
	// TODO
	int agreement = -1; // -1 = 'Negotiating' ; 0 = 'Failed Negotiation' ; 1 = 'Negotiation Completed'

	uint16_t requestedItemId;
	uint16_t contributedItemId;
	AgentLocation uccLocation;
	unsigned int searchDepth;

	void createChildMCP();
	void destroyChildMCP();

	MCPPtr _mcp;

};

