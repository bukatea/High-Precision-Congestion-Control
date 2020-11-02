/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/header.h"
#include "ns3/simulator.h"
#include "seq-ts-header.h"

NS_LOG_COMPONENT_DEFINE ("SeqTsHeader");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SeqTsHeader);

SeqTsHeader::SeqTsHeader ()
: m_seq (0) {
  if (IntHeader::mode == 1)
    ih.ts = Simulator::Now().GetTimeStep();
  else if (IntHeader::mode == 20)
    m_ts = Simulator::Now().GetTimeStep();
}

void
SeqTsHeader::SetSeq (uint32_t seq)
{
  m_seq = seq;
}
uint32_t
SeqTsHeader::GetSeq (void) const
{
  return m_seq;
}

void
SeqTsHeader::SetPG (uint16_t pg)
{
	m_pg = pg;
}
uint16_t
SeqTsHeader::GetPG (void) const
{
	return m_pg;
}

Time
SeqTsHeader::GetTs (void) const
{
  if (IntHeader::mode == 1)
    return TimeStep(ih.ts);
  else if (IntHeader::mode == 20)
    return TimeStep(m_ts);
  NS_ASSERT_MSG(IntHeader::mode == 1 || IntHeader::mode == 20, "SeqTsHeader cannot GetTs when IntHeader::mode != 1");
  return Time();
}

TypeId
SeqTsHeader::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SeqTsHeader")
  .SetParent<Header> ()
  .AddConstructor<SeqTsHeader> ()
  ;
  return tid;
}
TypeId
SeqTsHeader::GetInstanceTypeId (void) const
{
  return GetTypeId ();
}

void
SeqTsHeader::Print (std::ostream &os) const
{
  //os << "(seq=" << m_seq << " time=" << TimeStep (m_ts).GetSeconds () << ")";
	//os << m_seq << " " << TimeStep (m_ts).GetSeconds () << " " << m_pg;
	os << m_seq << " " << m_pg;
	if (IntHeader::mode == 20)
		os << std::endl << m_ts << " " << m_concflows_inc << " " << m_xcpId;
}
uint32_t
SeqTsHeader::GetSerializedSize (void) const
{
	return GetHeaderSize();
}
uint32_t SeqTsHeader::GetHeaderSize(void){
  uint32_t val = 6 + IntHeader::GetStaticSize();
  return IntHeader::mode == 20 ? val + sizeof(m_ts) + sizeof(m_concflows_inc) + sizeof(m_xcpId) : val;
}

void
SeqTsHeader::Serialize (Buffer::Iterator start) const
{
  Buffer::Iterator i = start;
  i.WriteHtonU32 (m_seq);
  i.WriteHtonU16 (m_pg);

  if (IntHeader::mode == 20) {
    i.WriteHtonU64(m_ts);
    uint64_t ui;
    std::memcpy(&ui, &m_concflows_inc, sizeof(double));
    i.WriteHtonU64(ui);
    i.WriteHtonU32(m_xcpId);
  }

  // write IntHeader
  ih.Serialize(i);
}
uint32_t
SeqTsHeader::Deserialize (Buffer::Iterator start)
{
  Buffer::Iterator i = start;
  m_seq = i.ReadNtohU32 ();
  m_pg =  i.ReadNtohU16 ();

  if (IntHeader::mode == 20) {
    m_ts = i.ReadNtohU64();
    uint64_t ui;
    ui = i.ReadNtohU64();
    std::memcpy(&m_concflows_inc, &ui, sizeof(double));
    m_xcpId = i.ReadNtohU32();
  }

  // read IntHeader
  ih.Deserialize(i);
  return GetSerializedSize ();
}

} // namespace ns3
