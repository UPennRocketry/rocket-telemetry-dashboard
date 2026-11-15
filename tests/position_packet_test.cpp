#include "../position_packet.h"
#include <cassert>

int main() {
  assert(isPositionPacket("POS,39.952583,-75.165222,123.45"));
  assert(isPositionPacket("POS,-90,180,-5"));
  assert(isPositionPacket("POS,+90,-180,1e3"));
  assert(!isPositionPacket("POS,90.01,0,1"));
  assert(!isPositionPacket("POS,0,-180.01,1"));
  assert(!isPositionPacket("POS,0,0,nan"));
  assert(!isPositionPacket("POS,0,0,1e999"));
  assert(!isPositionPacket("POS,0,0,"));
  assert(!isPositionPacket("POS,0,0,1,2"));
  assert(!isPositionPacket("POS, 0,0,1"));
  assert(!isPositionPacket("POS,0,0,1garbage"));
  assert(!isPositionPacket("OTHER,0,0,1"));
}
