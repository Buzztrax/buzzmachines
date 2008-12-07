
// samples		- data
// numsamples	= number of samples
// nch			= number of channes (1 or 2)
// bps			= bytes per sample (8 or 16)
// srate		= samplerate

int modify_samples1(struct winampDSPModule *this_mod, short int *samples, int numsamples, int bps, int nch, int srate)
{
	// echo doesn't support 8 bit right now cause I'm lazy.
	if (bps==16)
	{
/*
		float nobassLeft;
		float nobassRight;
		float slowLeft;
		float slowRight;
		float oldleft;
		float oldright;
		int iLeft;
		int iRight;
		int t;
		for (t=0; t<numsamples-1; t=t+2)							// scan gennem 16-bit lyddata i left-right par
		{
	//		float left = (float)blockBuffer[t]/32768.0f;					// lav om til floats der gaar fra -1.0 til +1.0
	//		float right = (float)blockBuffer[t+1]/32768.0f;					// should handle mono too!
			float left  = (float)samples[t]/32768.0f;
			float right = (float)samples[t+nch-1]/32768.0f;

			// ------- fjern bassen for at RMS ikke skal pumpe på bassen

			float tl = 0.0f;
			float tr = 0.0f;
			int a;
			for (a=40; a>0; a=a-1)
			{
				trebleLeft[a]  = trebleLeft[a-1];
				trebleRight[a] = trebleRight[a-1];
				tl = tl+(trebleLeft[0]-trebleLeft[a]);
				tr = tr+(trebleRight[0]-trebleRight[a]);
			}
			trebleLeft[0]  = left;
			trebleRight[0] = right;

			nobassRight = tr*0.1f;
			nobassLeft  = tl*0.1f;

			// ------- langsomme volume ændringer

			m_fRmsLeft = fabs(nobassLeft)*0.00003f + m_fRmsLeft*0.99997f;				// RMS values
			m_fRmsRight = fabs(nobassRight)*0.00003f + m_fRmsRight*0.99997f;
			slowLeft = m_fRmsLeft*8.0f+0.01f;
			slowRight= m_fRmsRight*8.0f+0.01f;
			left = left/slowLeft;
			right = right/slowRight;

			// ------- hurtig peak limiter for at undgå overstyring

			oldleft = left;
			left = left*m_fFastFaderLevelLeft;
			if (fabs(left)>m_fFastFaderThreshold)							// er der et peak?
			{
				m_fFastFaderLevelLeft = m_fFastFaderThreshold/fabs(oldleft+0.001f); // ja, beregn styrken
				left = oldleft*m_fFastFaderLevelLeft;						// regn faderlevel ud igen
			}
			if (fabs(left)>0.001f)											// der er helt stille - ingen pumpende kompressor
			{
				m_fFastFaderLevelLeft *= (1.0f+1.0f/m_fFastReleasetime);
				m_fFastFaderLevelLeft = __min(1.0f,m_fFastFaderLevelLeft);
			}

			oldright = right;
			right = right*m_fFastFaderLevelRight;
			if (fabs(right)>m_fFastFaderThreshold)							// er der et peak?
			{
				m_fFastFaderLevelRight = m_fFastFaderThreshold/fabs(oldright+0.001f); // ja, beregn styrken
				right = oldright*m_fFastFaderLevelRight;					// regn faderlevel ud igen
			}
			if (fabs(right)>0.001f)											// der er helt stille - ingen pumpende kompressor
			{
				m_fFastFaderLevelRight *= (1.0f+1.0f/m_fFastReleasetime);
				m_fFastFaderLevelRight = __min(1.0f,m_fFastFaderLevelRight);
			}

		//	left  = nobassLeft;												// lyt til signalet uden bas (TEST)
		//	right = nobassRight;

			// ------- tilbage til fil
			iLeft = __min(32767,__max(-32768,(int)(left*32768.0f)));	// clip og tilbage til 16-bit words
			iRight = __min(32767,__max(-32768,(int)(right*32768.0f)));
	//		blockBuffer[t] = iLeft;
	//		blockBuffer[t+1] = iRight;
			samples[t]       = iLeft;
			samples[t+nch-1] = iRight;
		}
*/
	}
	return numsamples;
}


