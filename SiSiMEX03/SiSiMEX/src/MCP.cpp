#include "MCP.h"
#include "UCP.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REQUESTING_MCCs,
	ST_ITERATING_OVER_MCCs,

	// TODO: Other states
	ST_WAITING_ACCEPTANCE,
	ST_NEGOTIATING,
	ST_WAITINGUCPRESULT,
	ST_FINISHED
};

MCP::MCP(Node *node, uint16_t requestedItemID, uint16_t contributedItemID, unsigned int searchDepth) :
	Agent(node),
	_requestedItemId(requestedItemID),
	_contributedItemId(contributedItemID),
	_searchDepth(searchDepth)
{
	setState(ST_INIT);
}

MCP::~MCP()
{
}

void MCP::update()
{
	switch (state())
	{
	case ST_INIT:
		queryMCCsForItem(_requestedItemId);
		setState(ST_REQUESTING_MCCs);
		break;

	case ST_REQUESTING_MCCs:
		// See OnPacketReceived -> PacketType::ReturnMCCsForItem
		break;

	case ST_ITERATING_OVER_MCCs:
		// TODO: Handle this state
		if (_mccRegisterIndex < _mccRegisters.size()) {
			AskNegotiation(_mccRegisters[_mccRegisterIndex]);
			setState(ST_WAITING_ACCEPTANCE);
		}
		else {
			setState(ST_FINISHED);
			_mccRegisterIndex = 0;
		}
		_mccRegisterIndex++;
		break;

	// TODO: Handle other states
	case ST_WAITING_ACCEPTANCE:
		break;

	case ST_NEGOTIATING:
		break;

	case ST_WAITINGUCPRESULT:
		break;

	case ST_FINISHED:
		break;

	default:;
	}
}

void MCP::stop()
{
	// TODO: Destroy the underlying search hierarchy (UCP->MCP->UCP->...)

	destroy();
}

bool MCP::IterateMCCs()
{
	for (int i = 0; i < _mccRegisters.size(); i++) 
	{
		AskNegotiation(_mccRegisters[i]);
	}

	setState(ST_WAITING_ACCEPTANCE);
	return true;
}

bool MCP::AskNegotiation(AgentLocation &mcc) 
{
	PacketHeader packethead;
	packethead.packetType = PacketType::NegotiationRequest;
	packethead.dstAgentId = mcc.agentId;
	packethead.srcAgentId = this->id();

	PacketNegotiationRequest body;
	body._requestedItemId = requestedItemId();
	body._contributedItemId = contributedItemId();

	OutputMemoryStream stream;
	packethead.Write(stream);
	body.Write(stream);


	return sendPacketToAgent(mcc.hostIP, mcc.hostPort, stream);
}
void MCP::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::ReturnMCCsForItem:
		if (state() == ST_REQUESTING_MCCs)
		{
			// Read the packet
			PacketReturnMCCsForItem packetData;
			packetData.Read(stream);

			// Log the returned MCCs
			for (auto &mccdata : packetData.mccAddresses)
			{
				uint16_t agentId = mccdata.agentId;
				const std::string &hostIp = mccdata.hostIP;
				uint16_t hostPort = mccdata.hostPort;
				//iLog << " - MCC: " << agentId << " - host: " << hostIp << ":" << hostPort;
			}

			// Store the returned MCCs from YP
			_mccRegisters.swap(packetData.mccAddresses);

			// Select the first MCC to negociate
			_mccRegisterIndex = 0;
			setState(ST_ITERATING_OVER_MCCs);

			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::ReturnMCCsForItem was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::NegotiationResponse:
		PacketNegotiationResponse packetBody;
		packetBody.Read(stream);
		if (packetBody.acceptNegotiation == true) {

		}
		else {
			setState(ST_ITERATING_OVER_MCCs);
		}
		break;
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCP::negotiationFinished() const
{
	return state() == ST_WAITINGUCPRESULT;
}

bool MCP::negotiationAgreement() const
{
	
	return false; // TODO: Did the child UCP find a solution?
}


bool MCP::queryMCCsForItem(int itemId)
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::QueryMCCsForItem;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketQueryMCCsForItem packetData;
	packetData.itemId = _requestedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	// 1) Ask YP for MCC hosting the item 'itemId'
	return sendPacketToYellowPages(stream);
}

void MCP::createChildUCP()
{
	if (_ucp != nullptr)
		destroyChildUCP();
	//_ucp = App->agentContainer->createUCP(node(), requestedItemId(), contributedItemId(), );
}

void MCP::destroyChildUCP()
{

}
