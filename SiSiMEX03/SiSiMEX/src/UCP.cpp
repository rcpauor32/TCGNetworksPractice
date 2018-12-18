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
	iLog << "Destroying UCP";
	destroyChildMCP();
	destroy();
}

void UCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	iLog << "UCP::OnPacketReceived()";

	switch (packetType)
	{
		// TODO: Handle packets
	case PacketType::ConstraintRequest:
		PacketConstraintRequest packetbody;
		packetbody.Read(stream);
		if (packetbody._constraintItemId == this->contributedItemId) {
			agreement = true;
			SendConstraintResult(true);
			setState(ST_SENDINGCONSTRAINT);
		}
		else
		{
			if (searchDepth >= MAX_SEARCH_DEPTH) {
				agreement = false;
				SendConstraintResult(false);
				wLog << "Max Depth Reached";
				setState(ST_SENDINGCONSTRAINT);
			}
			else {
				iLog << "UCP::Constraint Unresolved";
				createChildMCP(packetbody._constraintItemId);
				setState(ST_RESOLVINGCONSTRAINT);
			}
		}
		break;

	case PacketType::ConstraintAck:
		setState(ST_NEGOTIATIONFINISHED);
		iLog << "Constraint aknowledged";
		break;

	default:
		wLog << "UCP::OnPacketReceived() - Unexpected PacketType.";
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

	iLog << "UCP::Sending ItemRequest";

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

	iLog << "UCP::Sending Constraint Result:";
	iLog << res;

	return sendPacketToAgent(uccLocation.hostIP,uccLocation.hostPort,stream);
}

void UCP::createChildMCP(uint16_t newRequestedId)
{
	if (_mcp != nullptr)
		destroyChildMCP();
	iLog << "UCP::Creating Child MCP";
	_mcp = App->agentContainer->createMCP(node(), newRequestedId, contributedItemId, searchDepth);
}

void UCP::destroyChildMCP()
{
	if (_mcp != nullptr) {
		_mcp->stop();
		_mcp.reset();
	}
}

bool UCP::negotiationFinished()
{
	return state() == ST_NEGOTIATIONFINISHED;
}


