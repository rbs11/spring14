#ifndef REPL_STATE_H
#define REPL_STATE_H

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

#include <cstdlib>
#include <cassert>
#include <math.h>
#include "utils.h"
#include "crc_cache_defs.h"

#define PSEL_MAX         1023    // Width of the PSEL counter
#define NumSHCTCtrBits			3
#define NumSigBits			14
#define HIT_CNT_MAX			255
#define NumLeaderSets           32    // Use 32 sets per SDM
#define NumSHCTEntries		pow(2,NumSigBits)
// Replacement Policies Supported
typedef enum 
{
    CRC_REPL_LRU        = 0,
    CRC_REPL_RANDOM     = 1,
    CRC_REPL_DRRIP			= 2,
    CRC_REPL_SHIP			= 3,
    CRC_REPL_CONTESTANT	 = 4
} ReplacemntPolicy;
typedef enum
{
    FOLLOWER = 0,
    LEADER_SRRIP = 1,
    LEADER_BRRIP = 2
} SDMType;

// Replacement State Per Cache Line
typedef struct
{
    UINT32	LRUstackposition;	/*This size shouldn't be considered for championship*/

    // CONTESTANTS: Add extra state per cache line here
    unsigned short int	Signature;	/*True space required is 14 bits*/
    unsigned char		RRPV;		/*Requires 3 bits*/
    bool		Outcome;	/*Requires 1 bit*/
} LINE_REPLACEMENT_STATE;

/*Hybrid signature table*/
typedef struct
{
    // CONTESTANTS: Add extra state per cache line here
	unsigned char	Signature_Strength;	/*Using 3 bits*/
	unsigned char	nHits;		/*Using 8 bits*/
	unsigned char	nMax_Old;	/*Using 8 bits*/
	unsigned char	nMax_Stored;	/*Using 8 bits*/
    bool	expired;			/*Using 1 bit*/
} HybridSignatureTable;

// The implementation for the cache replacement policy
class CACHE_REPLACEMENT_STATE
{

  private:
    UINT32 numsets;
    UINT32 assoc;
    UINT32 replPolicy;
    
    LINE_REPLACEMENT_STATE   **repl;
    COUNTER mytimer;  // tracks # of references to the cache

    // CONTESTANTS:  Add extra state for cache here
    /*Used for DRRIP*/
    UINT32* SDM_info;
    UINT32 RRIP_MAX;
    UINT32 PSEL_VAL;
    UINT32 nLeaderSets;
    /*Used for SHIP*/
    UINT32* SHCT;
    UINT32 SigHashMask;

    /*Cache Championship*/
    HybridSignatureTable* HST;	/*2^14 Entries will be held in this table*/
    Addr_t TagBits;
    UINT32 TagShift;
  public:

    // The constructor CAN NOT be changed
    CACHE_REPLACEMENT_STATE( UINT32 _sets, UINT32 _assoc, UINT32 _pol );

    INT32  GetVictimInSet( UINT32 tid, UINT32 setIndex, const LINE_STATE *vicSet, UINT32 assoc, Addr_t PC, Addr_t paddr, UINT32 accessType );
    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID );

    void   SetReplacementPolicy( UINT32 _pol ) { replPolicy = _pol; } 
    void   IncrementTimer() { mytimer++; } 

    void   UpdateReplacementState( UINT32 setIndex, INT32 updateWayID, const LINE_STATE *currLine, 
                                   UINT32 tid, Addr_t PC, UINT32 accessType, bool cacheHit );

    ostream&   PrintStats( ostream &out);

  private:
    
    void   InitReplacementState();
    INT32  Get_Random_Victim( UINT32 setIndex );

    INT32  Get_LRU_Victim( UINT32 setIndex );
    INT32  Get_RRIP_Victim( UINT32 setIndex );
    INT32  Get_SHIP_Victim( UINT32 setIndex );
    INT32  Get_Contestant_Victim( UINT32 setIndex );
    void   UpdateLRU( UINT32 setIndex, INT32 updateWayID );
    void   UpdateDRRIP( UINT32 setIndex, INT32 updateWayID, bool cacheHit, UINT32  accessType);
    void   UpdateSHIP( UINT32 setIndex, INT32 updateWayID, bool cacheHit,  UINT32 Signature );
    void   UpdateMyPolicy( UINT32 setIndex, INT32 updateWayID, bool cacheHit,  UINT32 Signature );
};


#endif
