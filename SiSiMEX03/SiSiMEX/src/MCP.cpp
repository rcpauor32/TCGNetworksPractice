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
		break;

	// TODO: Handle other states
	case ST_WAITING_ACCEPTANCE:
		break;

	case ST_NEGOTIATING:
		if (_ucp != nullptr && _ucp->negotiationFinished() == true) {
			if (_ucp->agreement == true) { // Completed Negotiation
				setState(ST_FINISHED);
			}
			else if (_ucp->agreement == false) { // Failed Negotiation
				setState(ST_ITERATING_OVER_MCCs);
				_mccRegisterIndex++;
			}
		}
		else { // Negotiating

		}
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
	destroyChildUCP();
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

	iLog << "MCP::Asking Negotiation";
	return sendPacketToAgent(mcc.hostIP, mcc.hostPort, stream);
}
//void MCP::Exchange()
//{
//	iLog << "MCP::Exchange";
//	node()->itemList().addItem((int)requestedItemId());
//	node()->itemList().removeItem((int)contributedItemId());
//}

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
			wLog << "MCP::OnPacketReceived() - PacketType::ReturnMCCsForItem was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::NegotiationResponse:
		if (state() == ST_WAITING_ACCEPTANCE) {
			PacketNegotiationResponse packetBody;
			packetBody.Read(stream);
			if (packetBody.acceptNegotiation == true) {
				iLog << "MCP::Accepted Negotiation";
				iLog << packetBody.uccLoc.hostIP;
				createChildUCP(packetBody.uccLoc);
				setState(ST_NEGOTIATING);
			}
			else {
				setState(ST_ITERATING_OVER_MCCs);
				_mccRegisterIndex++;
			}
		}
		break;
	default:
		wLog << "MCP::OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCP::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCP::negotiationAgreement() const
{
	if(_ucp!= nullptr){
	iLog << _ucp->agreement;
	return _ucp->agreement == true; // TODO: Did the child UCP find a solution?
	}
	else {
		return false;
	}

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

void MCP::createChildUCP(AgentLocation &ucc)
{
	if (_ucp != nullptr)
		destroyChildUCP();
	iLog << "MCP::Creating Child UCP";
	_ucp = App->agentContainer->createUCP(node(), requestedItemId(), contributedItemId(), ucc, searchDepth() + 1);
}

void MCP::destroyChildUCP()
{
	if (_ucp != nullptr) {
		_ucp->stop();
		_ucp.reset();
	}
}
