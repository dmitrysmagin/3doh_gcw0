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

#include <stdint.h>
#include <string.h>

#include "XBUS.h"
#include "Clio.h"
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#pragma pack(push,1)

struct XBUSDatum {
	uint8_t XBSEL;
	uint8_t XBSELH;
	uint8_t POLF;
	uint8_t POLDEVF;
	uint8_t STDEVF[255];    //status of devices
	uint8_t STLENF;         //pointer in fifo
	uint8_t CmdF[7];
	uint8_t CmdPtrF;
};
#pragma pack(pop)

#define XBSEL xbus.XBSEL
#define XBSELH xbus.XBSELH
#define POLF xbus.POLF
#define POLDEVF xbus.POLDEVF
#define STDEVF xbus.STDEVF
#define STLENF xbus.STLENF
#define CmdF xbus.CmdF
#define CmdPtrF xbus.CmdPtrF

static struct XBUSDatum xbus;
static _xbus_device xdev[16];

#define POLSTMASK       0x01
#define POLDTMASK       0x02
#define POLMAMASK       0x04
#define POLREMASK       0x08
#define POLST           0x10
#define POLDT           0x20
#define POLMA           0x40
#define POLRE           0x80

void ExecuteCommandF(void);

void _xbus_SetCommandFIFO(uint32_t val)
{

	if (xdev[XBSEL]) {
		(*xdev[XBSEL])(XBP_SET_COMMAND, (void*)(uintptr_t)val);
		if ((*xdev[XBSEL])(XBP_FIQ, NULL))
			_clio_GenerateFiq(4, 0);
	} else if (XBSEL == 0xf) {
		if (CmdPtrF < 7) {
			CmdF[CmdPtrF] = (uint8_t)val;
			CmdPtrF++;
		}
		if (CmdPtrF >= 7) {
			ExecuteCommandF();
			CmdPtrF = 0;
		}
	}
}

uint32_t _xbus_GetDataFIFO(void)
{
	if (xdev[XBSEL])
		return (intptr_t)(*xdev[XBSEL])(XBP_GET_DATA, NULL);
	return 0;
}

uint32_t _xbus_GetPoll(void)
{
	uint32_t res = 0x30;

	if (XBSEL == 0xf)
		res = POLF;
	else if (xdev[XBSEL])
		res = (intptr_t)(*xdev[XBSEL])(XBP_GET_POLL, NULL);

	if (XBSELH & 0x80)
		res &= 0xf;

	return res;
}

uint32_t _xbus_GetRes(void)
{
	if (xdev[XBSEL])
		return (intptr_t)(*xdev[XBSEL])(XBP_RESERV, NULL);
	return 0;
}

void ExecuteCommandF(void)
{
	if (CmdF[0] == 0x83) {
		STLENF = 12;
		STDEVF[0] = 0x83;
		STDEVF[1] = 0x01;
		STDEVF[2] = 0x01;
		STDEVF[3] = 0x01;
		STDEVF[4] = 0x01;
		STDEVF[5] = 0x01;
		STDEVF[6] = 0x01;
		STDEVF[7] = 0x01;
		STDEVF[8] = 0x01;
		STDEVF[9] = 0x01;
		STDEVF[10] = 0x01;
		STDEVF[11] = 0x01;
		POLDEVF |= POLST;
	}

	if (((POLDEVF & POLST) && (POLDEVF & POLSTMASK)) || ((POLDEVF & POLDT) && (POLDEVF & POLDTMASK)))
		_clio_GenerateFiq(4, 0);
}

uint32_t _xbus_GetStatusFIFO(void)
{
	uint32_t res = 0;

	if (xdev[XBSEL])
		res = (intptr_t)(*xdev[XBSEL])(XBP_GET_STATUS, NULL);
	else if (XBSEL == 0xf) {
		if (STLENF > 0) {
			res = STDEVF[0];
			STLENF--;
			if (STLENF > 0) {
				int i;
				for (i = 0; i < STLENF; i++)
					STDEVF[i] = STDEVF[i + 1];
			}else
				POLDEVF &= ~POLST;
		}
		return res;
	}

	return res;
}

void _xbus_SetDataFIFO(uint32_t val)
{
	if (xdev[XBSEL])
		(*xdev[XBSEL])(XBP_SET_DATA, (void*)(uintptr_t)val);
}

void _xbus_SetPoll(uint32_t val)
{
	if (XBSEL == 0xf) {
		POLF &= 0xF0;
		POLF |= (val & 0xf);
	}
	if (xdev[XBSEL]) {
		(*xdev[XBSEL])(XBP_SET_POLL, (void*)(uintptr_t)val);
		if ((*xdev[XBSEL])(XBP_FIQ, NULL)) _clio_GenerateFiq(4, 0);
	}
}

void _xbus_SetSEL(uint32_t val)
{
	XBSEL = (uint8_t)val & 0xf;
	XBSELH = (uint8_t)val & 0xf0;
}

void _xbus_Init(_xbus_device zero_dev)
{
	int i;

	POLF = 0xf;

	for (i = 0; i < 15; i++)
		xdev[i] = NULL;

	_xbus_Attach(zero_dev);
}

int _xbus_Attach(_xbus_device dev)
{
	int i;

	for (i = 0; i < 16; i++) {
		if (!xdev[i])
			break;
	}

	if (i == 16)
		return -1;

	xdev[i] = dev;
	(*xdev[i])(XBP_INIT, NULL);

	return i;
}

void _xbus_DevLoad(int dev, const char * name)
{
	(*xdev[dev])(XBP_RESET, (void*)name);
}

void _xbus_DevEject(int dev)
{
	(*xdev[dev])(XBP_RESET, NULL);
}

void _xbus_Destroy(void)
{
	int i;

	for (i = 0; i < 16; i++) {
		if (xdev[i]) {
			(*xdev[i])(XBP_DESTROY, NULL);
			xdev[i] = NULL;
		}
	}
}

uint32_t _xbus_SaveSize(void)
{
	uint32_t tmp = sizeof(struct XBUSDatum);
	int i;

	tmp += 16 * 4;
	for (i = 0; i < 15; i++) {
		if (!xdev[i])
			continue;
		tmp += (intptr_t)(*xdev[i])(XBP_GET_SAVESIZE, NULL);
	}
	return tmp;
}

void _xbus_Save(void *buff)
{
	int i, off, j, tmp;

	memcpy(buff, &xbus, sizeof(struct XBUSDatum));
	j = off = sizeof(struct XBUSDatum);
	off += 16 * 4;

	for (i = 0; i < 15; i++) {
		if (!xdev[i]) {
			tmp = 0;
			memcpy(&((uint8_t*)buff)[j + i * 4], &tmp, 4);
		} else {
			(*xdev[i])(XBP_GET_SAVEDATA, &((uint8_t*)buff)[off]);
			memcpy(&((uint8_t*)buff)[j + i * 4], &off, 4);
			off += (intptr_t)(*xdev[i])(XBP_GET_SAVESIZE, NULL);
		}
	}
}

void _xbus_Load(void *buff)
{
	int i, offd;

	int j = sizeof(struct XBUSDatum);

	memcpy(&xbus, buff, j);

	for (i = 0; i < 15; i++) {
		memcpy(&offd, &((uint8_t*)buff)[j + i * 4], 4);

		if (!xdev[i])
			continue;

		if (!offd)
			(*xdev[i])(XBP_RESET, NULL);
		else
			(*xdev[i])(XBP_SET_SAVEDATA, &((uint8_t*)buff)[offd]);
	}
}
