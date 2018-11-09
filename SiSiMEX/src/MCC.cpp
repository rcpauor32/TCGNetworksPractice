#include "MCC.h"
#include "Application.h"
#include "ModuleAgentContainer.h"

// With these states you have enough so far...
enum State
{
	ST_INIT,
	ST_REGISTERING,
	ST_IDLE,
	ST_UNREGISTERING,
	ST_FINISHED
};

MCC::MCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node),
	_contributedItemId(contributedItemId),
	_constraintItemId(constraintItemId)
{
}


MCC::~MCC()
{
}

void MCC::start()
{
	// TODO: Set the initial state
	setState(ST_INIT);
}

void MCC::update()
{
	switch (state())
	{
		// TODO:
		// - Register or unregister into/from YellowPages depending on the state
		//       Use the functions registerIntoYellowPages and unregisterFromYellowPages
		//       so that this switch statement remains clean and readable
		// - Set the next state when needed ...

	case ST_INIT:
		registerIntoYellowPages();
		setState(ST_REGISTERING);
		break;
	case ST_REGISTERING:
		break;
	case ST_IDLE:
		break;
	case ST_UNREGISTERING:
		break;
	case ST_FINISHED:
		destroy();
		break;
	}
}

void MCC::stop()
{
	unregisterFromYellowPages();
	setState(ST_UNREGISTERING);
}


void MCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	// Taking the state into account, receive and deserialize packets (Ack packets) and set next state
	if (state() == ST_REGISTERING && packetHeader.packetType == PacketType::RegisterMCCAck)
	{
		// TODO: Set the next state (Idle in this case)
		setState(ST_IDLE);
		// TODO: Disconnect the socket (we don't need it anymore)
		if(socket != nullptr)
			socket->Disconnect();
	}

	// TODO: Do the same for unregistering
	if (state() == ST_UNREGISTERING && packetHeader.packetType == PacketType::UnregisterMCCAck) {
		setState(ST_IDLE);
		if(socket != nullptr)
			socket->Disconnect();
	}
}

bool MCC::negotiationFinished() const
{
	return false;
}

bool MCC::negotiationAgreement() const
{
	return false;
}

bool MCC::registerIntoYellowPages()
{
	// TODO: Create a PacketHeader (make it in Packets.h)

	// TODO: Create a PacketRegisterMCC (make it in Packets.h)

	// TODO: Serialize both packets into an OutputMemoryStream

	// TODO: Send the stream (Agent::sendPacketToYellowPages)

	return false;
}

bool MCC::unregisterFromYellowPages()
{
	// TODO: Create a PacketHeader (make it in Packets.h)

	// TODO: Create a PacketUnregisterMCC (make it in Packets.h)

	// TODO: Serialize both packets into an OutputMemoryStream

	// TODO: Send the stream (Agent::sendPacketToYellowPages)

	return false;
}
