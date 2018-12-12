#include "UCC.h"


// TODO: Make an enum with the states

enum State
{
	ST_WAITING_ITEM_REQUEST,
	ST_WAITING_ITEM_CONSTRAINT,
	ST_NEGOTIATION_FINISHED
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
	case PacketType::ItemRequest:
		if (state() == ST_WAITING_ITEM_REQUEST) {
			PacketItemRequest packetBody;
			packetBody.Read(stream);
			if (packetBody._requestedItemId == contributedItemId) {
				// Sending ConstraintRequest to UCP
				PacketHeader oPacketHeader;
				oPacketHeader.srcAgentId = id();
				oPacketHeader.dstAgentId = packetHeader.srcAgentId;
				oPacketHeader.packetType = PacketType::ConstraintRequest;
				OutputMemoryStream ostream;
				oPacketHeader.Write(ostream);
				PacketConstraintRequest oPacketBody;
				oPacketBody._constraintItemId = constraintItemId;
				oPacketBody.Write(ostream);
				socket->SendPacket(ostream.GetBufferPtr(), ostream.GetSize());
				
				setState(ST_WAITING_ITEM_CONSTRAINT);

			}
		}
		break;
	
	case PacketType::ConstraintResult:
		if (state() == ST_WAITING_ITEM_CONSTRAINT) {
			PacketConstraintResult packetBody;
			packetBody.Read(stream);
			// Checking Item ID
			if (packetBody._itemId == constraintItemId) {
				// Sending ConstraintAck to UCP
				PacketHeader oPacketHeader;
				oPacketHeader.packetType = PacketType::ConstraintAck;
				oPacketHeader.srcAgentId = id();
				oPacketHeader.dstAgentId = packetHeader.srcAgentId;
				OutputMemoryStream ostream;
				oPacketHeader.Write(ostream);

				socket->Send(ostream.GetBufferPtr(), ostream.GetSize());
				
				setState(ST_NEGOTIATION_FINISHED);

			}
		}
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}
