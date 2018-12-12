#include "UCC.h"


// TODO: Make an enum with the states

enum State
{
	ST_WAITINGITEMREQUEST,
	ST_WAITINGITEMCONSTRAINT,
	ST_NEGOTATIONFINISHED
};


UCC::UCC(Node *node, uint16_t contributedItemId, uint16_t constraintItemId) :
	Agent(node)
{
	// TODO: Save input parameters
	this->contributedItemId = contributedItemId;
	this->contributedItemId = constraintItemId;
}

UCC::~UCC()
{
}

void UCC::stop()
{
	destroy();
}

void UCC::OnPacketReceived(TCPSocketPtr socket, const PacketHeader &packetHeader, InputMemoryStream &stream)
{
	PacketType packetType = packetHeader.packetType;

	switch (packetType)
	{
		// TODO: Handle packets

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}
