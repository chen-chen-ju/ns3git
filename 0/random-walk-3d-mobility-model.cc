/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2006,2007 INRIA
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
#include "random-walk-3d-mobility-model.h"
#include "ns3/enum.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/pointer.h"
#include "ns3/simulator.h"
#include "ns3/log.h"
#include <cmath>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RandomWalk3d");

NS_OBJECT_ENSURE_REGISTERED (RandomWalk3dMobilityModel);

TypeId
RandomWalk3dMobilityModel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RandomWalk3dMobilityModel")
    .SetParent<MobilityModel> ()
    .SetGroupName ("Mobility")
    .AddConstructor<RandomWalk3dMobilityModel> ()
    .AddAttribute ("Bounds",
                   "Bounds of the area to cruise.",
                   BoxValue (Box (0.0, 100.0, 0.0, 100.0, 0.0, 100.0)),
                   MakeBoxAccessor (&RandomWalk3dMobilityModel::m_bounds),
                   MakeBoxChecker ())
    .AddAttribute ("Time",
                   "Change current direction and speed after moving for this delay.",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&RandomWalk3dMobilityModel::m_modeTime),
                   MakeTimeChecker ())
    .AddAttribute ("Distance",
                   "Change current direction and speed after moving for this distance.",
                   DoubleValue (1.0),
                   MakeDoubleAccessor (&RandomWalk3dMobilityModel::m_modeDistance),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Mode",
                   "The mode indicates the condition used to "
                   "change the current speed and direction",
                   EnumValue (RandomWalk3dMobilityModel::MODE_DISTANCE),
                   MakeEnumAccessor (&RandomWalk3dMobilityModel::m_mode),
                   MakeEnumChecker (RandomWalk3dMobilityModel::MODE_DISTANCE, "Distance",
                                    RandomWalk3dMobilityModel::MODE_TIME, "Time"))
    .AddAttribute ("HDirection",
                   "A random variable used to pick the horizon direction (radians).",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=6.283184]"),
                   MakePointerAccessor (&RandomWalk3dMobilityModel::m_hdirection),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("VDirection",
                   "A random variable used to pick the vertical direction (radians).",
                   StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=3.141592]"),
                   MakePointerAccessor (&RandomWalk3dMobilityModel::m_vdirection),
                   MakePointerChecker<RandomVariableStream> ())
    .AddAttribute ("Speed",
                   "A random variable used to pick the speed (m/s).",
                   DoubleValue (10),
                   MakeDoubleAccessor (&RandomWalk3dMobilityModel::m_speed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("SpeedChange",
                   "A random variable used to pick the speed variable quantity.",
                   StringValue ("ns3::UniformRandomVariable[Min=2.0|Max=4.0]"),
                   MakePointerAccessor (&RandomWalk3dMobilityModel::m_speedvary),
                   MakePointerChecker<RandomVariableStream> ());
  return tid;
}

void
RandomWalk3dMobilityModel::DoInitialize (void)
{
  DoInitializePrivate ();
  MobilityModel::DoInitialize ();
}

void
RandomWalk3dMobilityModel::DoInitializePrivate (void)
{
  m_helper.Update ();
  m_speed = m_speed + m_speedvary->GetValue ();
  if (m_speed < 0)
  {
      m_speed = 0;
  }
  if (m_speed > 60)
  {
      m_speed = 60;
  }
  double speed = m_speed;
  double hdirection = m_hdirection->GetValue ();
  double vdirection = m_vdirection->GetValue ();
  Vector vector (std::sin (vdirection) *std::cos (hdirection) * speed,
                 std::sin (vdirection) *std::sin (hdirection) * speed,
                 std::cos (vdirection) * speed);
  m_helper.SetVelocity (vector);
  m_helper.Unpause ();

  Time delayLeft;
  if (m_mode == RandomWalk3dMobilityModel::MODE_TIME)
    {
      delayLeft = m_modeTime;
    }
  else
    {
      delayLeft = Seconds (m_modeDistance / speed); 
    }
  DoWalk (delayLeft);
}

void
RandomWalk3dMobilityModel::DoWalk (Time delayLeft)
{
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  Vector nextPosition = position;
  nextPosition.x += speed.x * delayLeft.GetSeconds ();
  nextPosition.y += speed.y * delayLeft.GetSeconds ();
  nextPosition.z += speed.z * delayLeft.GetSeconds ();
  m_event.Cancel ();
  if (m_bounds.IsInside (nextPosition))
    {
      m_event = Simulator::Schedule (delayLeft, &RandomWalk3dMobilityModel::DoInitializePrivate, this);
    }
  else
    {
      nextPosition = m_bounds.CalculateIntersection (position, speed);
      Time delay = Seconds ((nextPosition.x - position.x) / speed.x);
      m_event = Simulator::Schedule (delay, &RandomWalk3dMobilityModel::Rebound, this,
                                     delayLeft - delay);
    }
  NotifyCourseChange ();
}

void
RandomWalk3dMobilityModel::Rebound (Time delayLeft)
{
  m_helper.UpdateWithBounds (m_bounds);
  Vector position = m_helper.GetCurrentPosition ();
  Vector speed = m_helper.GetVelocity ();
  switch (m_bounds.GetClosestSide (position))
    {
    case Box::RIGHT:
    case Box::LEFT:
      speed.x = -speed.x;
      break;
    case Box::TOP:
    case Box::BOTTOM:
      speed.y = -speed.y;
      break;
    case Box::UP:
    case Box::DOWN:
      speed.z = -speed.z;
      break;
    }
  m_helper.SetVelocity (speed);
  m_helper.Unpause ();
  DoWalk (delayLeft);
}

void
RandomWalk3dMobilityModel::DoDispose (void)
{
  // chain up
  MobilityModel::DoDispose ();
}
Vector
RandomWalk3dMobilityModel::DoGetPosition (void) const
{
  m_helper.UpdateWithBounds (m_bounds);
  return m_helper.GetCurrentPosition ();
}
void
RandomWalk3dMobilityModel::DoSetPosition (const Vector &position)
{
  NS_ASSERT (m_bounds.IsInside (position));
  m_helper.SetPosition (position);
  Simulator::Remove (m_event);
  m_event = Simulator::ScheduleNow (&RandomWalk3dMobilityModel::DoInitializePrivate, this);
}
Vector
RandomWalk3dMobilityModel::DoGetVelocity (void) const
{
  return m_helper.GetVelocity ();
}
int64_t
RandomWalk3dMobilityModel::DoAssignStreams (int64_t stream)
{
  m_speedvary->SetStream (stream);
  m_hdirection->SetStream (stream + 1);
  m_vdirection->SetStream (stream + 2);
  return 3;
}


} // namespace ns3
