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
	this->constraintItemId = constraintItemId;
}

UCC::~UCC()
{
}

void UCC::stop()
{
	iLog << "Destroying UCC";
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
			iLog << "UCC::Sending ConstraintRequest";
			socket->SendPacket(ostream.GetBufferPtr(), ostream.GetSize());
			
			setState(ST_WAITING_ITEM_CONSTRAINT);
		}
		else {
			wLog << "UCC::PacketReceived() - Unexpected Item Request";
		}
		break;
	
	case PacketType::ConstraintResult:
		if (state() == ST_WAITING_ITEM_CONSTRAINT) {
			PacketConstraintResult packetBody;
			packetBody.Read(stream);
			if (packetBody.accepted == true) {
				agreement = true;
			}
			else {
				agreement = false;
			}
			// Sending ConstraintAck to UCP
			PacketHeader oPacketHeader;
			oPacketHeader.packetType = PacketType::ConstraintAck;
			oPacketHeader.srcAgentId = id();
			oPacketHeader.dstAgentId = packetHeader.srcAgentId;
			OutputMemoryStream ostream;
			oPacketHeader.Write(ostream);
			iLog << "UCC::Sending ConstraintAck";
			socket->SendPacket(ostream.GetBufferPtr(), ostream.GetSize());
			
			setState(ST_NEGOTIATION_FINISHED);
		}
		else {
			wLog << "UCC::PacketReceived() - Unexpected Constraint Result";
		}
		break;

	default:
		wLog << "OnPacketReceived() - Unexpected PacketType.";
	}
}

bool UCC::negotiationAgreement()
{
	return (state() == ST_NEGOTIATION_FINISHED && agreement == true);
}
