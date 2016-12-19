/*
   www.freedo.org
   The first and only working 3DO multiplayer emulator.

   The FreeDO licensed under modified GNU LGPL, with following notes:

 *   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
 *   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
 *   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

   The above notes are taking precedence over GNU LGPL in conflicting situations.

   Project authors:

   Alexander Troosh
   Maxim Grishin
   Allen Wright
   John Sammons
   Felix Lazarev
 */

// VDLP.h: interface for the CVDLP class.
//
//////////////////////////////////////////////////////////////////////

#ifndef VDLP_3DO_HEADER
#define VDLP_3DO_HEADER

#include "freedocore.h"

#ifdef __cplusplus
extern "C" {
#endif

void _vdl_Init(uint8_t *vramstart);
void _vdl_ProcessVDL( uint32_t addr);

void _vdl_DoLineNew(int line, struct VDLFrame *frame);

uint32_t _vdl_SaveSize(void);
void _vdl_Save(void *buff);
void _vdl_Load(void *buff);

#ifdef __cplusplus
}
#endif

#endif
