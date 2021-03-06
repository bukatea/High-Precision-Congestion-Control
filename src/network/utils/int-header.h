#ifndef INT_HEADER_H
#define INT_HEADER_H

#include "ns3/buffer.h"
#include <stdint.h>
#include <cstdio>

namespace ns3 {

class IntHop{
public:
	static const uint32_t timeWidth = 32;
	static const uint32_t bytesWidth = 32;
	static const uint32_t qlenWidth = 29;
	static const uint32_t numeratorWidth = 32;
	static const uint64_t lineRateValues[8];
	union{
		struct {
			uint64_t lineRate: 64-qlenWidth-bytesWidth,
					 bytes: bytesWidth,
					 qlen: qlenWidth;
			uint64_t time: timeWidth,
					 numerator: numeratorWidth;
			#if 0
			uint32_t lineRate:32-timeWidth,
					 time: timeWidth;
			uint32_t bytes: bytesWidth,
					 qlen: 32-bytesWidth;
			#endif
		};
		uint32_t buf[4];
	};

	static const uint32_t byteUnit = 128;
	static const uint32_t qlenUnit = 80;
	static uint32_t multi;

	uint64_t GetLineRate(){
		return lineRateValues[lineRate];
	}
	uint32_t GetBytes(){
		return (uint64_t)bytes * byteUnit * multi;
	}
	uint32_t GetQlen(){
		return (uint32_t)qlen * qlenUnit * multi;
	}
	uint32_t GetTime(){
		return time;
	}
	uint32_t GetNumerator(){
		return numerator;
	}
	void Set(uint32_t _time, uint32_t _bytes, uint32_t _qlen, uint64_t _rate, uint32_t _numerator){
		time = _time;
		bytes = _bytes / (byteUnit * multi);
		qlen = _qlen / (qlenUnit * multi);
		numerator = _numerator;
		switch (_rate){
			case 25000000000lu:
				lineRate=0;break;
			case 50000000000lu:
				lineRate=1;break;
			case 100000000000lu:
				lineRate=2;break;
			case 200000000000lu:
				lineRate=3;break;
			case 400000000000lu:
				lineRate=4;break;
			default:
				printf("Error: IntHeader unknown rate: %lu\n", _rate);
				break;
		}
	}
	uint64_t GetBytesDelta(IntHop &b){
		if (bytes >= b.bytes)
			return (bytes - b.bytes) * byteUnit * multi;
		else
			return (bytes + (1lu<<bytesWidth) - b.bytes) * byteUnit * multi;
	}
	uint64_t GetTimeDelta(IntHop &b){
		if (time >= b.time)
			return time - b.time;
		else
			return time + (1lu<<timeWidth) - b.time;
	}
};

class IntHeader{
public:
	static const uint32_t maxHop = 5;
	static uint32_t mode; // 0/20: INT, 1: ts, 5:0

	// Note: the structure of IntHeader must have no internal padding, because we will directly transform the part of packet buffer to IntHeader*
	union{
		struct {
			IntHop hop[maxHop];
			uint16_t nhop;
		};
		uint64_t ts;
	};

	IntHeader();
	static uint32_t GetStaticSize();
	void PushHop(uint32_t time, uint32_t bytes, uint32_t qlen, uint64_t rate, uint32_t numerator);
	void Serialize (Buffer::Iterator start) const;
	uint32_t Deserialize (Buffer::Iterator start);
	uint64_t GetTs(void);
};

}

#endif /* INT_HEADER_H */
