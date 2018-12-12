#include "UCP.h"
#include "MCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


// TODO: Make an enum with the statesç

enum State {
	ST_INIT,
	ST_REQUESTINGITEM,
	ST_RESOLVINGCONSTRAINT,
	ST_SENDINGCONSTRAINT,
	ST_NEGOTIATIONFINISHED

};


UCP::UCP(Node *node, uint16_t requestedItemId, uint16_t contributedItemId, const AgentLocation &uccLocation, unsigned int searchDepth) :
	Agent(node)
{
	// TODO: Save input parameters
}

UCP::~UCP()
{
}

void UCP::update()
{
	switch (state())
	{
		// TODO: Handle states
	case ST_INIT:
		
		break;

	case ST_REQUESTINGITEM:
		break;

	case ST_RESOLVINGCONSTRAINT:
		break;
	
	case ST_SENDINGCONSTRAINT:
		break;
	
	case ST_NEGOTIATIONFINISHED:
		break;

	default:;
	}
}

void UCP::stop()
{
	// TODO: Destroy search hierarchy below this agent

	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::ConstraintRequest:
		break;

	case PacketType::ConstraintAck:
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}
