#include "replacement_state.h"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This file is distributed as part of the Cache Replacement Championship     //
// workshop held in conjunction with ISCA'2010.                               //
//                                                                            //
//                                                                            //
// Everyone is granted permission to copy, modify, and/or re-distribute       //
// this software.                                                             //
//                                                                            //
// Please contact Aamer Jaleel <ajaleel@gmail.com> should you have any        //
// questions                                                                  //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

/*
** This file implements the cache replacement state. Users can enhance the code
** below to develop their cache replacement ideas.
**
*/


////////////////////////////////////////////////////////////////////////////////
// The replacement state constructor:                                         //
// Inputs: number of sets, associativity, and replacement policy to use       //
// Outputs: None                                                              //
//                                                                            //
// DO NOT CHANGE THE CONSTRUCTOR PROTOTYPE                                    //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
CACHE_REPLACEMENT_STATE::CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol )
{

    numsets    = _sets;
    assoc      = _assoc;
    replPolicy = _pol;

    mytimer    = 0;

    InitReplacementState();
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function initializes the replacement policy hardware by creating      //
// storage for the replacement state on a per-line/per-cache basis.           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::InitReplacementState()
{
    // Create the state for sets, then create the state for the ways
    repl  = new LINE_REPLACEMENT_STATE* [ numsets ];

    // ensure that we were able to create replacement state
    assert(repl);

    // Create the state for the sets
    for(UINT32 setIndex=0; setIndex<numsets; setIndex++) 
    {
        repl[ setIndex ]  = new LINE_REPLACEMENT_STATE[ assoc ];

        for(UINT32 way=0; way<assoc; way++) 
        {
            // initialize stack position (for true LRU)
            repl[ setIndex ][ way ].LRUstackposition = way;
            repl[ setIndex ][ way ].RRPV = 0;
            repl[ setIndex ][ way ].Outcome = 0;
            repl[ setIndex ][ way ].Signature = 0;
        }
    }

    // Contestants:  ADD INITIALIZATION FOR YOUR HARDWARE HERE
    /*DRRIP stuff*/
    PSEL_VAL = 0;
    nLeaderSets = 32;
   // cout<<"Sets total is "<<numsets<<"\n";
    SDM_info = (UINT32*)calloc(numsets, sizeof(UINT32));
    for (UINT32 setIndex = 0; setIndex < numsets; setIndex++)
	   {
		if (setIndex < 2048)
		{
			UINT32 MASK_LOW = (setIndex & 31);
			UINT32 MASK_HIGH ((setIndex >> 6) & 31);
			UINT32 SEL_BIT = (setIndex >> 5)&1;
			if ((MASK_LOW == MASK_HIGH))//Select Insertion policy
			{
				if (SEL_BIT)//Use SRIP
				{
					SDM_info[setIndex] = LEADER_SRRIP;
					nLeaderSets--;
				}
				else //Use BRRIP
				{
					SDM_info[setIndex] = LEADER_BRRIP;
					nLeaderSets--;
				}
			}
			else
				SDM_info[setIndex] = FOLLOWER;
		}
		else
			SDM_info[setIndex] = FOLLOWER;
	   }
    /*For SHIP*/
    SigHashMask = ((1<<NumSigBits)-1);

    SHCT = (UINT32*)calloc((SigHashMask+1), sizeof(UINT32));

    /*Championship*/
    HST = (HybridSignatureTable*)(calloc((SigHashMask+1), sizeof(HybridSignatureTable)));
    if (numsets == 4096)
    {
    	TagBits = 64 - 12 - 5;
    	TagShift = 12 + 5;
    }
    else if (numsets == 1024)
    {
    	TagBits = 64 - 10 - 5;
    	TagShift = 10 + 5;
    }
    else
    {
    	TagBits = 0;
    	TagShift = 0;
    	cout<<"Set count not supported "<<numsets<<"\n";
    }

    cout<<"SignHashMask"<<SigHashMask<<"TagBits "<<TagBits<<"TagShift "<<TagShift<<"\n";
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache on every cache miss. The input        //
// arguments are the thread id, set index, pointers to ways in current set    //
// and the associativity.  We are also providing the PC, physical address,    //
// and accesstype should you wish to use them at victim selection time.       //
// The return value is the physical way index for the line being replaced.    //
// Return -1 if you wish to bypass LLC.                                       //
//                                                                            //
// vicSet is the current set. You can access the contents of the set by       //
// indexing using the wayID which ranges from 0 to assoc-1 e.g. vicSet[0]     //
// is the first way and vicSet[4] is the 4th physical way of the cache.       //
// Elements of LINE_STATE are defined in crc_cache_defs.h                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc,
                                               Addr_t PC, Addr_t paddr, UINT32 accessType )
{
    // If no invalid lines, then replace based on replacement policy
    if( replPolicy == CRC_REPL_LRU ) 
    {
        return Get_LRU_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        return Get_Random_Victim( setIndex );
    }
    else if( replPolicy == CRC_REPL_DRRIP )
    {
    	return Get_RRIP_Victim( setIndex );
    }
    else if(replPolicy == CRC_REPL_SHIP)
	{
    	return Get_SHIP_Victim( setIndex );
	}
    else if(replPolicy == CRC_REPL_CONTESTANT)
	{
		// Contestants:  ADD YOUR VICTIM SELECTION FUNCTION HERE
		return Get_Contestant_Victim( setIndex );
	}

    // We should never get here
    assert(0);

    return -1; // Returning -1 bypasses the LLC
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function is called by the cache after every cache hit/miss            //
// The arguments are: the set index, the physical way of the cache,           //
// the pointer to the physical line (should contestants need access           //
// to information of the line filled or hit upon), the thread id              //
// of the request, the PC of the request, the accesstype, and finall          //
// whether the line was a cachehit or not (cacheHit=true implies hit)         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateReplacementState( 
    UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
    UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit )
{
    // What replacement policy?
    if( replPolicy == CRC_REPL_LRU ) 
    {
        UpdateLRU( setIndex, updateWayID );
    }
    else if( replPolicy == CRC_REPL_RANDOM )
    {
        // Random replacement requires no replacement state update
    }
    else if( replPolicy == CRC_REPL_DRRIP )
    {
        // Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
        // Feel free to use any of the input parameters to make
        // updates to your replacement policy
    	UpdateDRRIP( setIndex, updateWayID, cacheHit, accessType );
    }
    else if( replPolicy == CRC_REPL_SHIP )
	{
		// Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
		// Feel free to use any of the input parameters to make
		// updates to your replacement policy
		UpdateSHIP( setIndex, updateWayID, cacheHit, (PC&SigHashMask) );
	}
    else if( replPolicy == CRC_REPL_CONTESTANT )
	{
		// Contestants:  ADD YOUR UPDATE REPLACEMENT STATE FUNCTION HERE
		// Feel free to use any of the input parameters to make
		// updates to your replacement policy
		//UpdateMyPolicy( setIndex, updateWayID, cacheHit, (PC&SigHashMask) );
    	//UINT32 TagSignature = (((PC))&SigHashMask);
    	  //UpdateMyPolicy( setIndex, updateWayID, cacheHit, (currLine->tag & SigHashMask) );
    	UINT32 TagSignature = (((currLine->tag))&SigHashMask);
    	UpdateMyPolicy( setIndex, updateWayID, cacheHit, (TagSignature) );
	}

    
    
}

INT32 CACHE_REPLACEMENT_STATE::Get_RRIP_Victim( UINT32 setIndex )
{
    LINE_REPLACEMENT_STATE *CurrLineState = repl[ setIndex ];

    INT32   victim   = 0;
    bool    bDone = false;

    while( !bDone )
    {
        for(UINT32 iway=0; iway<assoc; iway++)
        {
            if( CurrLineState[iway].RRPV == (assoc-1) )
            {
                victim   = iway;
                bDone = true;
                break;
            }
        }
        if( !bDone )
        {
            for(UINT32 iway=0; iway<assoc; iway++)
                CurrLineState[iway].RRPV++;
        }
    }
    return victim;
}

INT32 CACHE_REPLACEMENT_STATE::Get_SHIP_Victim( UINT32 setIndex )
{
    LINE_REPLACEMENT_STATE *CurrLineState = repl[ setIndex ];
    INT32   victim   = 0;
    bool    bDone = false;

    while( !bDone )
    {
        for(UINT32 iway=0; iway<assoc; iway++)
        {
            if( CurrLineState[iway].RRPV == (assoc-1) )
            {
                victim   = iway;
                bDone = true;
                break;
            }
        }
        if( !bDone )
        {
            for(UINT32 iway=0; iway<assoc; iway++)
                CurrLineState[iway].RRPV++;
        }
    }
    return victim;
}


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//////// HELPER FUNCTIONS FOR REPLACEMENT UPDATE AND VICTIM SELECTION //////////
//                                                                            //
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds the LRU victim in the cache set by returning the       //
// cache block at the bottom of the LRU stack. Top of LRU stack is '0'        //
// while bottom of LRU stack is 'assoc-1'                                     //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_LRU_Victim( UINT32 setIndex )
{
    // Get pointer to replacement state of current set
    LINE_REPLACEMENT_STATE *replSet = repl[ setIndex ];

    INT32   lruWay   = 0;

    // Search for victim whose stack position is assoc-1
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( replSet[way].LRUstackposition == (assoc-1) ) 
        {
            lruWay = way;
            break;
        }
    }

    // return lru way
    return lruWay;
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function finds a random victim in the cache set                       //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
INT32 CACHE_REPLACEMENT_STATE::Get_Random_Victim( UINT32 setIndex )
{
    INT32 way = (rand() % assoc);
    
    return way;
}

INT32 CACHE_REPLACEMENT_STATE::Get_Contestant_Victim( UINT32 setIndex )
{
    LINE_REPLACEMENT_STATE *CurrLineState = repl[ setIndex ];
    INT32   victim   = 0;
    UINT32  iMaxRRPV    = 0;
    UINT32  iMinSig    = 0;
    UINT32	Sigtemp;
    bool	bMyVicFound = 0;
    bool    bDone = false;


    for(UINT32 iway=0; iway<assoc; iway++)
    {
    	Sigtemp = CurrLineState[iway].Signature;
    	/*Compare current value with old value, which is indicative that this line
    	 * may not be referenced again, based on previous history*/
    	if ((HST[Sigtemp].expired) && (HST[Sigtemp].nHits >= HST[Sigtemp].nMax_Old))
    	{
    		if (HST[Sigtemp].nMax_Old != 0)
    		{
				/*Expired line is found*/
    			if (CurrLineState[iway].RRPV > iMaxRRPV)
    			{
    				iMaxRRPV = CurrLineState[iway].RRPV;
    				victim   = iway;
    				bMyVicFound = 1;
    			}
    		}
    	}
    }

    if (bMyVicFound)
    {
        for(UINT32 iway=0; iway<assoc; iway++)
        {
            if (CurrLineState[iway].RRPV == iMaxRRPV)
    		{
            	Sigtemp = CurrLineState[iway].Signature;
            	if ( HST[Sigtemp].Signature_Strength < iMinSig)
            	{
    				iMinSig = HST[Sigtemp].Signature_Strength;
    				victim   = iway;
            	}
    		}
        }
        return victim;
    }


    /*If victim using my policy is not found, then use SHIP*/
    while( !bDone )
    {
        for(UINT32 iway=0; iway<assoc; iway++)
        {
            if( CurrLineState[iway].RRPV == (assoc-1) )
            {
                victim   = iway;
                bDone = true;
                break;
            }
        }
        if( !bDone )
        {
            for(UINT32 iway=0; iway<assoc; iway++)
                CurrLineState[iway].RRPV++;
        }
    }
    return victim;
}

void CACHE_REPLACEMENT_STATE::UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID, bool cacheHit,  UINT32 Signature )
{
	if (cacheHit)
	{
		Signature = repl[ setIndex ][ updateWayID ].Signature;
		repl[ setIndex ][ updateWayID ].RRPV = 0;
		repl[ setIndex ][ updateWayID ].Outcome = 1;

		if (HST[Signature].nHits < (HIT_CNT_MAX))
			HST[Signature].nHits++;

		if (HST[Signature].Signature_Strength < (NumSHCTCtrBits))
			HST[Signature].Signature_Strength++;
		/*Increase the hit count for the line associated with this signature*/


	}
	else
	{
		UINT32 SigTemp = repl[ setIndex ][ updateWayID ].Signature;

		if (!repl[ setIndex ][ updateWayID ].Outcome)
		{
			if (HST[SigTemp].Signature_Strength > 0)
				HST[SigTemp].Signature_Strength--;
		}
		HST[SigTemp].nMax_Stored = HST[SigTemp].nHits;
		/*Compare current value of hits with the value stored earlier*/
		if (HST[SigTemp].nMax_Old == HST[SigTemp].nHits)
		{
			/*If there is a match, we know that this line has
			 * been accessed the same number of times again and mark
			 * it as expired*/
			if (HST[SigTemp].nMax_Old != 0)
				HST[SigTemp].expired = 1;
			else
				HST[SigTemp].expired = 0;
		}
		else
			HST[SigTemp].expired = 0;


		if (HST[Signature].Signature_Strength == 0)
			repl[ setIndex ][ updateWayID ].RRPV = assoc - 1;
		else
			repl[ setIndex ][ updateWayID ].RRPV = assoc - 2;

		repl[ setIndex ][ updateWayID ].Outcome = 0;
		repl[ setIndex ][ updateWayID ].Signature = Signature;
		/*Reset the parameters and save the value of current hits
		 * into another register value*/
		HST[Signature].nHits = 0;
		HST[Signature].nMax_Old = HST[Signature].nMax_Stored;

	}
	UpdateLRU(setIndex, updateWayID);
}

////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// This function implements the LRU update routine for the traditional        //
// LRU replacement policy. The arguments to the function are the physical     //
// way and set index.                                                         //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
void CACHE_REPLACEMENT_STATE::UpdateLRU( UINT32 setIndex, INT32 updateWayID )
{
    // Determine current LRU stack position
    UINT32 currLRUstackposition = repl[ setIndex ][ updateWayID ].LRUstackposition;

    // Update the stack position of all lines before the current line
    // Update implies incremeting their stack positions by one
    for(UINT32 way=0; way<assoc; way++) 
    {
        if( repl[setIndex][way].LRUstackposition < currLRUstackposition ) 
        {
            repl[setIndex][way].LRUstackposition++;
        }
    }

    // Set the LRU stack position of new line to be zero
    repl[ setIndex ][ updateWayID ].LRUstackposition = 0;
}

void CACHE_REPLACEMENT_STATE::UpdateSHIP( UINT32 setIndex, INT32 updateWayID, bool cacheHit,  UINT32 Signature )
{
	if (cacheHit)
	{
		Signature  = repl[ setIndex ][ updateWayID ].Signature;
		repl[ setIndex ][ updateWayID ].RRPV = 0;
		repl[ setIndex ][ updateWayID ].Outcome = 1;
		if (SHCT[Signature] < (NumSHCTCtrBits))
			SHCT[Signature]++;
	}
	else
	{
		UINT32 SigTemp = repl[ setIndex ][ updateWayID ].Signature;
		if (!repl[ setIndex ][ updateWayID ].Outcome)
		{
			if (SHCT[SigTemp] > 0)
				  SHCT[SigTemp]--;
		}
		if (SHCT[Signature] == 0)
			repl[ setIndex ][ updateWayID ].RRPV = assoc - 1;
		else
			repl[ setIndex ][ updateWayID ].RRPV = assoc - 2;
		repl[ setIndex ][ updateWayID ].Outcome = 0;
		repl[ setIndex ][ updateWayID ].Signature = Signature;
	}
}


void CACHE_REPLACEMENT_STATE::UpdateDRRIP( UINT32 setIndex, INT32 updateWayID, bool cacheHit, UINT32 accessType )
{
#if 0
#else
	if (cacheHit)
		repl[ setIndex ][ updateWayID ].RRPV = 0;
	else
	{
		if (SDM_info[setIndex] == LEADER_SRRIP)
		{
			repl[ setIndex ][ updateWayID ].RRPV = assoc - 1;
			if (PSEL_VAL < PSEL_MAX)
				PSEL_VAL++;
		}
		else if (SDM_info[setIndex] == LEADER_BRRIP)
		{
			repl[ setIndex ][ updateWayID ].RRPV = assoc - 2;
			if (PSEL_VAL > 0)
				PSEL_VAL--;
		}
		else//follower
		{
			if (PSEL_VAL > 512 && ((rand()%100)<5))
			{
				repl[ setIndex ][ updateWayID ].RRPV = assoc - 2;
			}
			else
			{
				repl[ setIndex ][ updateWayID ].RRPV = assoc - 1;
			}
		}
	}
	return;
#endif
}
////////////////////////////////////////////////////////////////////////////////
//                                                                            //
// The function prints the statistics for the cache                           //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////
ostream & CACHE_REPLACEMENT_STATE::PrintStats(ostream &out)
{

    out<<"=========================================================="<<endl;
    out<<"=========== Replacement Policy Statistics ================"<<endl;
    out<<"=========================================================="<<endl;

    // CONTESTANTS:  Insert your statistics printing here

    return out;
    
}

