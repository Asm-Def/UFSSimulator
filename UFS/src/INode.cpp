#include "INode.h"
diskaddr_t INode::size() const
{
	return (diskaddr_t) blocks * BLOCK_SIZE + rem_bytes;
}