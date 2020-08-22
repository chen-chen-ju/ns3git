/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 Dan Broyles
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
 * Author: Dan Broyles <dbroyl01@ku.edu>
 */

#include "box.h"
#include "ns3/vector.h"
#include "ns3/assert.h"
#include "ns3/fatal-error.h"
#include <cmath>
#include <algorithm>
#include <sstream> 

namespace ns3 {

Box::Box (double _xMin, double _xMax,
          double _yMin, double _yMax,
          double _zMin, double _zMax)
  : xMin (_xMin),
    xMax (_xMax),
    yMin (_yMin),
    yMax (_yMax),
    zMin (_zMin),
    zMax (_zMax)
{
}

Box::Box ()
  : xMin (0.0),
    xMax (0.0),
    yMin (0.0),
    yMax (0.0),
    zMin (0.0),
    zMax (0.0)
{
}

bool
Box::IsInside (const Vector &position) const
{
  return
    position.x <= this->xMax && position.x >= this->xMin &&
    position.y <= this->yMax && position.y >= this->yMin &&
    position.z <= this->zMax && position.z >= this->zMin;
}

Box::Side
Box::GetClosestSide (const Vector &position) const
{
  double xMinDist = std::abs (position.x - this->xMin);
  double xMaxDist = std::abs (this->xMax - position.x);
  double yMinDist = std::abs (position.y - this->yMin);
  double yMaxDist = std::abs (this->yMax - position.y);
  double zMinDist = std::abs (position.z - this->zMin);
  double zMaxDist = std::abs (this->zMax - position.z);
  double minX = std::min (xMinDist, xMaxDist);
  double minY = std::min (yMinDist, yMaxDist);
  double minZ = std::min (zMinDist, zMaxDist);
  if (minX < minY && minX < minZ)
    {
      if (xMinDist < xMaxDist)
        {
          return LEFT;
        }
      else
        {
          return RIGHT;
        }
    }
  else if (minY < minZ)
    {
      if (yMinDist < yMaxDist)
        {
          return BOTTOM;
        }
      else
        {
          return TOP;
        }
    }
  else
    {
      if (zMinDist < zMaxDist)
        {
          return DOWN;
        }
      else
        {
          return UP;
        }
    }
}

Vector
Box::CalculateIntersection (const Vector &current, const Vector &speed) const
{
  NS_ASSERT (IsInside (current));
  double xMaxY = current.y + (this->xMax - current.x) / speed.x * speed.y;
  double xMinY = current.y + (this->xMin - current.x) / speed.x * speed.y;
  double xMaxZ = current.z + (this->xMax - current.x) / speed.x * speed.z;
  double xMinZ = current.z + (this->xMin - current.x) / speed.x * speed.z;

  double yMaxX = current.x + (this->yMax - current.y) / speed.y * speed.x;
  double yMinX = current.x + (this->yMin - current.y) / speed.y * speed.x;
  double yMaxZ = current.z + (this->yMax - current.y) / speed.y * speed.z;
  double yMinZ = current.z + (this->yMin - current.y) / speed.y * speed.z;

  double zMaxX = current.x + (this->zMax - current.z) / speed.z * speed.x;
  double zMinX = current.x + (this->zMin - current.z) / speed.z * speed.x;
  double zMaxY = current.y + (this->zMax - current.z) / speed.z * speed.y;
  double zMinY = current.y + (this->zMin - current.z) / speed.z * speed.y;


  bool xMaxOk = (xMaxY <= this->yMax && xMaxY >= this->yMin &&  xMaxZ <= this->zMax && xMaxZ >= this->zMin);
  bool xMinOk = (xMinY <= this->yMax && xMinY >= this->yMin && xMinZ <= this->zMax && xMinZ >= this->zMin);


  bool yMaxOk = (yMaxX <= this->xMax && yMaxX >= this->xMin && yMaxZ <= this->zMax && yMaxZ >= this->zMin);
  bool yMinOk = (yMinX <= this->xMax && yMinX >= this->xMin && yMinZ <= this->zMax && yMinZ >= this->zMin);

  bool zMaxOk = (zMaxX <= this->xMax && zMaxX >= this->xMin && zMaxY <= this->yMax && zMaxY >= this->yMin);
  bool zMinOk = (zMinX <= this->xMax && zMinX >= this->xMin && zMinY <= this->yMax && zMinY >= this->yMin);

  if (xMaxOk && speed.x >= 0)
    {
      return Vector (this->xMax, xMaxY, xMaxZ);
    }
  else if (xMinOk && speed.x <= 0)
    {
      return Vector (this->xMin, xMinY, xMinZ);
    }
  else if (yMaxOk && speed.y >= 0)
    {
      return Vector (yMaxX, this->yMax, yMaxZ);
    }
  else if (yMinOk && speed.y <= 0)
    {
      return Vector (yMinX, this->yMin, yMinZ);
    }
  else if (zMaxOk && speed.z >= 0)
    {
      return Vector (zMaxX, zMaxY, this->zMax);
    }
  else if (zMinOk && speed.z <= 0)
    {
      return Vector (zMinX, zMinY, this->zMin);
    }
  else
    {
      NS_ASSERT (false);
      // quiet compiler
      return Vector (0.0, 0.0, 0.0);
    }

}

ATTRIBUTE_HELPER_CPP (Box);

/**
 * \brief Stream insertion operator.
 *
 * \param os the stream
 * \param box the box
 * \returns a reference to the stream
 */
std::ostream &
operator << (std::ostream &os, const Box &box)
{
  os << box.xMin << "|" << box.xMax << "|" << box.yMin << "|" << box.yMax << "|" << box.zMin << "|" << box.zMax;
  return os;
}
/**
 * \brief Stream extraction operator.
 *
 * \param is the stream
 * \param box the box
 * \returns a reference to the stream
 */
std::istream &
operator >> (std::istream &is, Box &box)
{
  char c1, c2, c3, c4, c5;
  is >> box.xMin >> c1 >> box.xMax >> c2 >> box.yMin >> c3 >> box.yMax >> c4 >> box.zMin >> c5 >> box.zMax;
  if (c1 != '|' ||
      c2 != '|' ||
      c3 != '|' ||
      c4 != '|' ||
      c5 != '|')
    {
      is.setstate (std::ios_base::failbit);
    }
  return is;
}


} // namespace ns3
