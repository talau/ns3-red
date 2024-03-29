/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This code was ported from NS-2.34, with licence:
 *
 * Copyright (c) 1990-1997 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the Computer Systems
 *	Engineering Group at Lawrence Berkeley Laboratory.
 * 4. Neither the name of the University nor of the Laboratory may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *
 * This port:
 *
 * Copyright © 2011 Marcos Talau
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Marcos Talau (talau@users.sourceforge.net)
 *
 * Thanks to: Duy Nguyen<duy@soe.ucsc.edu> by RED efforts in NS3
 *
 */

/*
 * PORT NOTE: Almost all comments also been ported from NS-2
 */

#ifndef RED_QUEUE_H
#define RED_QUEUE_H

#include <queue>
#include "ns3/packet.h"
#include "ns3/queue.h"
#include "ns3/nstime.h"
#include "ns3/random-variable.h"
#include "ns3/boolean.h"
#include "ns3/data-rate.h"
#include "ns3/nstime.h"

namespace ns3 {

class TraceContainer;

/*
 * \ingroup queue
 *
 * \brief A RED packet queue
 */
class RedQueue : public Queue
{
public:
  static TypeId GetTypeId (void);
  /*
   * \brief RedQueue Constructor
   *
   * Create a RED queue
   */
  RedQueue ();

  virtual ~RedQueue ();

  typedef struct
  {
    // Early probability drops
    uint32_t unforcedDrop;
    // Forced drops, qavg > max threshold
    uint32_t forcedDrop;
    // Drops due to queue limits
    uint32_t qLimDrop;
  } Stats;

  // Drop types
  enum
    {
      DTYPE_NONE,      // Ok, no drop
      DTYPE_FORCED,    // A "forced" drop
      DTYPE_UNFORCED,  // An "unforced" (random) drop
    };

  /*
   * Enumeration of the modes supported in the class.
   *
   */
  enum Mode
  {
    ILLEGAL,     // Mode not set
    PACKETS,     // Use number of packets for maximum queue size
    BYTES,       // Use number of bytes for maximum queue size
  };

  /*
   * Set the operating mode of this device.
   *
   * \param mode The operating mode of this device.
   */
  void SetMode (RedQueue::Mode mode);

  /*
   * Get the encapsulation mode of this device.
   *
   * \returns The encapsulation mode of this device.
   */
  RedQueue::Mode  GetMode (void);

  /*
   * Get the current value of the queue in bytes or packets.
   *
   * \returns The queue size in bytes or packets.
   */
  uint32_t GetQueueSize (void);

  /*
   * Set the limit of the queue.
   *
   * \param lim The limit in bytes or packets.
   */
  void SetQueueLimit(uint32_t lim);

  /*
   * Set the thresh limits of RED.
   *
   * \param min Minimum thresh in bytes or packets.
   * \param max Maximum thresh in bytes or packets.
   */
  void SetTh(double minTh, double maxTh);

  /*
   * Get the RED statistics after running.
   *
   * \returns The drop statistics.
   */
  Stats GetStats();

private:
  virtual bool DoEnqueue (Ptr<Packet> p);
  virtual Ptr<Packet> DoDequeue (void);
  virtual Ptr<const Packet> DoPeek (void) const;

  // ...
  void InitializeParams (void);
  // Compute the average queue size
  double Estimator (uint32_t nQueued, uint32_t m, double qAvg, double qW);
  // Check if packet p needs to be dropped due to probability mark
  uint32_t DropEarly (Ptr<Packet> p, uint32_t qSize);
  // Returns a probability using these function parameters for the DropEarly funtion
  double CalculatePNew (double qAvg, double maxTh, bool gentle, double vA,
                        double vB, double vC, double vD, double maxP);
  // Returns a probability using these function parameters for the DropEarly funtion
  double ModifyP(double p, uint32_t count, uint32_t countBytes,
                 uint32_t meanPktSize, bool wait, uint32_t size);

  std::list<Ptr<Packet> > m_packets;

  uint32_t m_bytesInQueue;
  bool m_hasRedStarted;
  Stats m_stats;

  // ** Variables supplied by user
  // Bytes or packets?
  Mode m_mode;
  // Avg pkt size
  uint32_t m_meanPktSize;
  // Avg pkt size used during idle times
  uint32_t m_idlePktSize;
  // True for waiting between dropped packets
  bool m_isWait;
  // True to increases dropping prob. slowly when ave queue exceeds maxthresh
  bool m_isGentle;
  // Min avg length threshold (bytes)
  double m_minTh;
  // Max avg length threshold (bytes), should be >= 2*minTh
  double m_maxTh;
  // Queue limit in bytes / packets
  uint32_t m_queueLimit;
  // Queue weight given to cur q size sample
  double m_qW;
  // The max probability of dropping a packet
  double m_lInterm;
  // Ns-1 compatibility
  bool m_isNs1Compat;
  // Link bandwidth
  DataRate m_linkBandwidth;
  // Link delay
  Time m_linkDelay;

  // ** Variables maintained by RED
  // Prob. of packet drop before "count"
  double m_vProb1;
  // v_prob = v_a * v_ave + v_b
  double m_vA;
  double m_vB;
  // Used for "gentle" mode
  double m_vC;
  // Used for "gentle" mode
  double m_vD;
  // Current max_p
  double m_curMaxP;
  // Prob. of packet drop
  double m_vProb;
  // # of bytes since last drop
  uint32_t m_countBytes;
  // 0 when average queue first exceeds thresh
  uint32_t m_old;
  // 0/1 idle status
  uint32_t m_idle;
  // packet time constant in packets/second
  double m_ptc;
  // Average queue length
  double m_qAvg;
  // number of packets since last random number generation
  uint32_t m_count;
  /*
   * 0 for default RED
   * 1 experimental (see red-queue.cc)
   * 2 experimental (see red-queue.cc)
   * 3 use Idle packet size in the ptc
   */
  uint32_t m_cautious;
  // Start of current idle period
  Time m_idleTime;
};

}; // namespace ns3

#endif // RED_QUEUE_H
