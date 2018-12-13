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
	this->requestedItemId = requestedItemId;
	this->contributedItemId = contributedItemId;
	this->uccLocation = uccLocation;
	this->searchDepth = searchDepth;

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
		agreement = -1;
		SendItemRequest();
		setState(ST_REQUESTINGITEM);
		break;

	case ST_REQUESTINGITEM:
		break;

	case ST_RESOLVINGCONSTRAINT:
		if (_mcp->negotiationFinished()) {
			if (_mcp->negotiationAgreement()) {
				SendConstraintResult(true);
				agreement = true;
			}
			else {
				SendConstraintResult(false);
				agreement = false;
			}
			setState(ST_SENDINGCONSTRAINT);
		}
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
	destroyChildMCP();
	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::ConstraintRequest:
		PacketConstraintRequest packetbody;
		packetbody.Read(stream);
		if (packetbody._constraintItemId == this->contributedItemId) {
			agreement = true;
			SendConstraintResult(true);
		}
		else
		{
			if (searchDepth >= MAX_SEARCH_DEPTH) {
				SendConstraintResult(false);
				agreement = false;
				setState(ST_SENDINGCONSTRAINT);
			}
			else {
				createChildMCP();
				setState(ST_RESOLVINGCONSTRAINT);
			}
		}
		break;

	case PacketType::ConstraintAck:
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool UCP::SendItemRequest()
{
	PacketHeader packethead;
	packethead.packetType = PacketType::ItemRequest;
	packethead.dstAgentId = uccLocation.agentId;
	packethead.srcAgentId = this->id();

	PacketItemRequest body;
	body._requestedItemId = this->requestedItemId;
	OutputMemoryStream stream;
	packethead.Write(stream);
	body.Write(stream);

	return sendPacketToAgent(uccLocation.hostIP, uccLocation.hostPort, stream);
}

bool UCP::SendConstraintResult(bool res)
{
	PacketHeader packethead;
	packethead.packetType = PacketType::ConstraintResult;
	packethead.dstAgentId = uccLocation.agentId;
	packethead.srcAgentId = this->id();

	PacketConstraintResult body;
	body.accepted = res;
	OutputMemoryStream stream;
	packethead.Write(stream);
	body.Write(stream);

	return sendPacketToAgent(uccLocation.hostIP,uccLocation.hostPort,stream);
}

void UCP::createChildMCP()
{
	if (_mcp != nullptr)
		destroyChildMCP();
	_mcp = App->agentContainer->createMCP(node(), requestedItemId, contributedItemId, searchDepth);
}

void UCP::destroyChildMCP()
{
	if (_mcp != nullptr) {
		_mcp->stop();
		_mcp.reset();
	}
}


