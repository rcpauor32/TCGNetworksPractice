#include "MCC.h"
#include "UCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"


enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	
	// TODO: Other states
	ST_NEGOTIATING,
	ST_WAITINGUCCRESULT,
	ST_UNREGISTERING,
	ST_FINISHED
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
	setState(ST_INIT);
}


MCC::~MCC()
{
}

void MCC::update()
{
	switch (state())
	{
	case ST_INIT:
		if (registerIntoYellowPages()) {
			setState(ST_REGISTERING);
		}
		else {
			setState(ST_FINISHED);
		}
		break;

	case ST_REGISTERING:
		// See OnPacketReceived() -> PacketType::RegisterAck
		break;

		// TODO: Handle other states
	case ST_IDLE:
		// See OnPacketReceived() -> PacketType::NegotiationRequest
		break;
	case ST_NEGOTIATING:
		if (negotiationAgreement() == true) {
			setState(ST_UNREGISTERING);
		}
		else {
			setState(ST_IDLE);
		}
		break;

	case ST_WAITINGUCCRESULT:
		break;

	case ST_UNREGISTERING:
		unregisterFromYellowPages();
		setState(ST_FINISHED);
		break;
	
	case ST_FINISHED:
		destroy();
		break;
	}
}

void MCC::stop()
{
	// Destroy hierarchy below this agent (only a UCC, actually)
	destroyChildUCC();

	unregisterFromYellowPages();
	setState(ST_FINISHED);
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	const PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
	case PacketType::RegisterMCCAck:
		if (state() == ST_REGISTERING)
		{
			setState(ST_IDLE);
			socket->Disconnect();
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::RegisterMCCAck was unexpected.";
		}
		break;

	// TODO: Handle other packets
	case PacketType::NegotiationRequest:
		if (state() == ST_IDLE) 
		{
			createChildUCC();
		
			sendAcceptNegotiation(socket, packetHeader.srcAgentId);
		}
		else
		{
			wLog << "OnPacketReceived() - PacketType::NegotiationRequest was unexpected.";
		}
		break;
	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool MCC::isIdling() const
{
	return state() == ST_IDLE;
}

bool MCC::negotiationFinished() const
{
	return state() == ST_FINISHED;
}

bool MCC::negotiationAgreement() const
{
	// If this agent finished, means that it was an agreement
	// Otherwise, it would return to state ST_IDLE
	return negotiationFinished();
}

bool MCC::sendAcceptNegotiation(TCPSocketPtr socket, uint16_t dstID)
{
	PacketHeader packetHead;
	packetHead.packetType = PacketType::Accept;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = dstID;

	// Serialize
	OutputMemoryStream stream;
	packetHead.Write(stream);

	socket->SendPacket(stream.GetBufferPtr(), stream.GetSize());

	return false;
}

bool MCC::registerIntoYellowPages()
{
	// Create message header and data
	PacketHeader packetHead;
	packetHead.packetType = PacketType::RegisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketRegisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	return sendPacketToYellowPages(stream);
}

void MCC::unregisterFromYellowPages()
{
	// Create message
	PacketHeader packetHead;
	packetHead.packetType = PacketType::UnregisterMCC;
	packetHead.srcAgentId = id();
	packetHead.dstAgentId = -1;
	PacketUnregisterMCC packetData;
	packetData.itemId = _contributedItemId;

	// Serialize message
	OutputMemoryStream stream;
	packetHead.Write(stream);
	packetData.Write(stream);

	sendPacketToYellowPages(stream);
}

void MCC::createChildUCC()
{
	// TODO: Create a unicast contributor
	if (_ucc != nullptr)
		destroyChildUCC();

	_ucc = App->agentContainer->createUCC(node(), contributedItemId(), constraintItemId());

}
	

void MCC::destroyChildUCC()
{
	// TODO: Destroy the unicast contributor child
	if (_ucc != nullptr) 
	{
		_ucc->stop();
		_ucc.reset();
	}
}
