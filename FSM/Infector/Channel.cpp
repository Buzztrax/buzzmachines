/* $Id$
 *
 * buzzmachines
 * Copyright (C) 2007 Krzysztof Foltman  <kfoltman@users.sourceforge.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <assert.h>
#include <math.h>
#include <float.h>
#include <MachineInterface.h>
#include "../dspchips/DSPChips.h"

#include "Infector.h"

CChannel::CChannel()
{
  Frequency=0.01f;
  FilterEnv.m_nState=4;
  pTrack=NULL;
}

void CChannel::Init()
{
}

void CChannel::ClearFX()
{
}

void CChannel::Reset()
{
	AmpEnv.NoteOff();
	FilterEnv.NoteOff();
	AmpEnv.m_fSilence=1.0/128.0;
  Frequency=0.01f;
	pTrack=NULL;

}

void CChannel::NoteReset()
{
  PhaseOSC1=0.0f;
  PhaseOSC2=0.0f;
  Filter.ResetFilter();
  Phase1=0;
  Phase2=0;
}
