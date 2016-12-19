#include <stdint.h>
#include <stdbool.h>

#include "freedocore.h"
#include "frame.h"

uint8_t FIXED_CLUTR[32];
uint8_t FIXED_CLUTG[32];
uint8_t FIXED_CLUTB[32];

void _frame_Init(void)
{
	int j;

	for (j = 0; j < 32; j++) {
		FIXED_CLUTR[j] = (uint8_t)(((j & 0x1f) << 3) | ((j >> 2) & 7));
		FIXED_CLUTG[j] = FIXED_CLUTR[j];
		FIXED_CLUTB[j] = FIXED_CLUTR[j];
	}
}

void Get_Frame_Bitmap(struct VDLFrame* sourceFrame, void* destinationBitmap, int copyWidth, int copyHeight)
{
	int i, pix;
	uint8_t *destPtr = (uint8_t*)destinationBitmap;

	for (i = 0; i < copyHeight; i++) {
		struct VDLLine* linePtr = (struct VDLLine*)&sourceFrame->lines[i];
		int16_t *srcPtr = (int16_t*)linePtr;
		bool allowFixedClut = (linePtr->xOUTCONTROLL & 0x2000000) > 0;
		for (pix = 0; pix < copyWidth; pix++) {
			if (*srcPtr == 0) {
				*destPtr++ = (uint8_t)(linePtr->xBACKGROUND & 0x1F);
				*destPtr++ = (uint8_t)((linePtr->xBACKGROUND >> 5) & 0x1F);
				*destPtr++ = (uint8_t)((linePtr->xBACKGROUND >> 10) & 0x1F);
			} else if (allowFixedClut && (*srcPtr & 0x8000) > 0) {
				*destPtr++ = FIXED_CLUTB[(*srcPtr) & 0x1F];
				*destPtr++ = FIXED_CLUTG[((*srcPtr) >> 5) & 0x1F];
				*destPtr++ = FIXED_CLUTR[(*srcPtr) >> 10 & 0x1F];
			} else {
				*destPtr++ = (uint8_t)(linePtr->xCLUTB[(*srcPtr) & 0x1F]);
				*destPtr++ = linePtr->xCLUTG[((*srcPtr) >> 5) & 0x1F];
				*destPtr++ = linePtr->xCLUTR[(*srcPtr) >> 10 & 0x1F];
			}

			destPtr++;
			srcPtr++;
			/*
			   16-bits displays...
			   destPtr--;
			         srcPtr++;
			 */
		}
	}
}
