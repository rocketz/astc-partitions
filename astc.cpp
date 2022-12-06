// p *(int(*)[10])ptr
// p *(uint8(*)[16])curblock

#include <stdio.h>
#include <stdlib.h>
#include <QPainter>
#include <QPixmap>
#include "app.h"

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

uint32_t hash52( uint32_t p )
{
	p ^= p >> 15; 

	// p -= p << 17; 
	// p += p << 7; 
	// p += p << 4; 
	p *= 0xEEDE0891;			// (2^4+1)*(2^7+1)*(2^17-1)


	p ^= p >> 5; 
	p += p << 16; 
	p ^= p >> 7; 
	p ^= p >> 3; 
	p ^= p << 6; 
	p ^= p >> 17; 
	return p;
}

int select_partition(int seed, int x, int y, int z,
					 int partitioncount, int small_block)
{
	if( small_block ){ x <<= 1; y <<= 1; z <<= 1; }
	seed += (partitioncount-1) * 1024;
	uint32_t rnum = hash52(seed);
	uint8_t seed1  =  rnum        & 0xF;
	uint8_t seed2  = (rnum >>  4) & 0xF;
	uint8_t seed3  = (rnum >>  8) & 0xF;
	uint8_t seed4  = (rnum >> 12) & 0xF;
	uint8_t seed5  = (rnum >> 16) & 0xF;
	uint8_t seed6  = (rnum >> 20) & 0xF;
	uint8_t seed7  = (rnum >> 24) & 0xF;
	uint8_t seed8  = (rnum >> 28) & 0xF;
	uint8_t seed9  = (rnum >> 18) & 0xF;
	uint8_t seed10 = (rnum >> 22) & 0xF;
	uint8_t seed11 = (rnum >> 26) & 0xF;
	uint8_t seed12 = ((rnum >> 30) | (rnum << 2)) & 0xF;
	seed1 *= seed1; seed2 *= seed2; seed3 *= seed3; seed4 *= seed4; 
	seed5 *= seed5; seed6 *= seed6; seed7 *= seed7; seed8 *= seed8; 
	seed9 *= seed9; seed10 *= seed10; seed11 *= seed11; seed12 *= seed12;
	int sh1, sh2, sh3;
	if( seed & 1 )
		{ sh1 = (seed & 2 ? 4 : 5); sh2 = (partitioncount == 3 ? 6 : 5); }
	else
		{ sh1 = (partitioncount == 3 ? 6 : 5); sh2 = (seed & 2 ? 4 : 5); }
	sh3 = (seed & 0x10) ? sh1 : sh2;
	seed1 >>= sh1; seed2  >>= sh2; seed3  >>= sh1; seed4  >>= sh2;
	seed5 >>= sh1; seed6  >>= sh2; seed7  >>= sh1; seed8  >>= sh2;
	seed9 >>= sh3; seed10 >>= sh3; seed11 >>= sh3; seed12 >>= sh3;
	int a = seed1*x + seed2*y + seed11*z + (rnum >> 14);
	int b = seed3*x + seed4*y + seed12*z + (rnum >> 10);
	int c = seed5*x + seed6*y + seed9 *z + (rnum >>  6);
	int d = seed7*x + seed8*y + seed10*z + (rnum >>  2);
	a &= 0x3F; b &= 0x3F; c &= 0x3F; d &= 0x3F;
	if( partitioncount < 4 ) d = 0;
	if( partitioncount < 3 ) c = 0;
	if( a >= b && a >= c && a >= d ) return 0;
	else if( b >= c && b >= d ) return 1;
	else if( c >= d ) return 2;
	else return 3;
}


ZPartitionInfo::ZPartitionInfo(int w, int h, int partitions, const ZPartitionInfo* pExtra0, const ZPartitionInfo* pExtra1)
{
	mnWidth = w;
	mnHeight = h;
	const int blocksize = mnWidth * mnHeight;
	mpPartitions = new uint8[patterns * blocksize];
	mnNumPartitions = partitions;
	manPartitionUsage[0] = 0;
	manPartitionUsage[1] = 0;
	manPartitionUsage[2] = 0;
	manPartitionUsage[3] = 0;
	CalcPatterns();
	
	bool bCheckCross = true;
	int unique = 0;
	int crossDupes = 0;
	for (int pat = 0; pat < patterns; pat++)
	{
		manPartitionUsage[maNumActualPartitions[pat] - 1]++;
		maDupes[pat] = CheckUnique(pat);		// Checks for dupes within set
		if (maDupes[pat] == -1)
		{
			unique++;
			if (bCheckCross)
			{
				maSimilar[pat] = CheckCrossDupes(pat, pExtra0, pExtra1);		// Check for dupes in sets with less partitions
				if (maSimilar[pat].pattern != -1)
				{
					crossDupes++;
				}
			}
		}
		else
		{
			maSimilar[pat].Init();
		}
	}
	mnNumDupes = patterns - unique;
	mnNumCrossDupes = crossDupes;
}

void ZPartitionInfo::CalcPatterns()
{
	const uint8 anBitCnts[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4};
	const int blocksize = mnWidth * mnHeight;
	const int smallblock = (blocksize < 32) ? 1 : 0;
	for (int pat = 0; pat < patterns; pat++)
	{
		uint8* curblock = &mpPartitions[pat * blocksize];
		int nSeen = 0;
		for (int y = 0; y < mnHeight; y++)
		{
			for (int x = 0; x < mnWidth; x++)
			{
				int partition = select_partition(pat, x,y,0, mnNumPartitions, smallblock);
				curblock[y * mnWidth + x] = uint8(partition);
				nSeen |= 1U << partition;
			}
		}
		maNumActualPartitions[pat] = anBitCnts[nSeen];
	}
}


ZPartitionInfo::~ZPartitionInfo()
{
	delete [] mpPartitions;
}

// -1 if no similar, else id in set
ZPartitionInfo::SSimilar ZPartitionInfo::CheckCrossDupes(int pattern, const ZPartitionInfo* pExtra0, const ZPartitionInfo* pExtra1) const
{
	SSimilar similar;
	similar.Init();

	const uint blocksize = mnWidth * mnHeight;
	const uint8* curblock = &mpPartitions[pattern * blocksize];

	if (pExtra0)
	{
		uint match;
		if (pExtra0->Test(curblock, match))
		{
			// 'match' is unique id and not sequence id..
//			printf("match0 %d %d first\n", pattern, match);
			similar.pattern = match;
			similar.set = 0;
			return similar;
		}
	}
	if (pExtra1)
	{
		uint match;
		if (pExtra1->Test(curblock, match))
		{
//			printf("match1 %d %d second\n", pattern, match);
			similar.pattern = match;
			similar.set = 0;
			return similar;
		}
	}
	return similar;
}


// Checks a block against all blocks in another (this) partition set
bool ZPartitionInfo::Test(const uint8* curblock, uint& idx) const
{
	const uint blocksize = mnWidth * mnHeight;
	for (uint i = 0; i < patterns; i++)
	{
		uint same = 0;
		int anRemap[4] = {-1,-1,-1,-1};
		for (uint x = 0; x < blocksize; x++)
		{
			int map = anRemap[curblock[x]];
			if (map == -1)
			{
				int q;
				for (q = 0; q < 4; q++)
				{
					if (anRemap[q] == mpPartitions[i * blocksize + x])
					{
						break;
					}
				}
				if (q != 4)
					break;			// Already mapped

				anRemap[curblock[x]] = mpPartitions[i * blocksize + x];
				map = anRemap[curblock[x]];
			}
			
			if (map == mpPartitions[i * blocksize + x])
				same++;
			else
				break;
		}
		if (same == blocksize)
		{
			idx = i;
			return true;
		}
	}
	
	return false;
}

// Returns hit idx or -1 if unique
int ZPartitionInfo::CheckUnique(int pattern)
{
	const int blocksize = mnWidth * mnHeight;
	const uint8* curblock = &mpPartitions[pattern * blocksize];
	int i;
	for (i = 0; i < pattern - 1; i++)
	{
		int same = 0;
		for (int x = 0; x < blocksize; x++)
		{
			if (curblock[x] == mpPartitions[i * blocksize + x])
				same++;
			else
				break;
		}
		if (same == blocksize)
		{
			return i;		// Exactly the same
		}

		bool bSimilarCheck = true;
		if (bSimilarCheck)
		{
			same = 0;
			int anRemap[4] = {-1,-1,-1,-1};
			for (int x = 0; x < blocksize; x++)
			{
				int map = anRemap[curblock[x]];
				if (map == -1)
				{
					int q;
					for (q = 0; q < 4; q++)
					{
						if (anRemap[q] == mpPartitions[i * blocksize + x])
						{
							break;
						}
					}
					if (q != 4)
						break;			// Already mapped

					anRemap[curblock[x]] = mpPartitions[i * blocksize + x];
					map = anRemap[curblock[x]];
				}
				
				if (map == mpPartitions[i * blocksize + x])
					same++;
				else
					break;
			}
			if (same == blocksize)
			{
				return i;
			}
		}
	}
	
	return -1;
}

inline float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

QPixmap* ZPartitionInfo::CreatePixmap(uint nGUIWidth, uint size, int hideDupes, int showCrossDupes)
{
	const uint gap = (size >= 4) ? 6 : 2;
	const uint blocksizeX = (mnWidth * size);
	const uint blocksizeY = (mnHeight * size);

	const int nTotalDupes = mnNumDupes + mnNumCrossDupes;
	const uint totalBlocks = (hideDupes) ? (1024 - nTotalDupes) : 1024;

	uint adjust = (nGUIWidth - (blocksizeX / 2)) / (blocksizeX + gap);
	const uint blocksX = adjust;
	uint blocksY = (totalBlocks / blocksX);
	if ((blocksX * blocksY) < totalBlocks)
	{
		blocksY++;
	}

	const uint blockPixels = mnWidth * mnHeight;
	const uint sizeX = (blocksX * blocksizeX) + ((blocksX + 1) * gap);
	const uint sizeY = (blocksY * blocksizeY) + ((blocksY + 1) * gap);

	QPixmap* pPixmap = new QPixmap(sizeX, sizeY);
	QPainter painter(pPixmap);
	painter.fillRect(0, 0, pPixmap->width(), pPixmap->height(), Qt::white);
	
	QColor cols[4];
	QColor mutedCols[4];
	QBrush brush[4];
	QBrush mutedBrush[4];

	cols[0].setHsvF(0.08f, 0.9f, 1.0f);
	cols[1].setHsvF(0.35f, 0.8f, 0.9f);
	cols[2].setHsvF(0.6f, 0.9f, 1.0f);
	cols[3].setHsvF(0.8f, 0.9f, 0.9f);
	for (int c = 0; c < 4; c++)
	{
		mutedCols[c].setHsvF(cols[c].hueF(), cols[c].saturationF() * 0.8f, cols[c].valueF() * 0.5f);

		brush[c] = QBrush(Qt::SolidPattern);
		brush[c].setColor(cols[c]);

		mutedBrush[c] = QBrush(Qt::SolidPattern);

/*
		float v = 0.2f;
		float t = 0.8f;
		float b = 0.4f;

		QColor col;
		col.setRgbF(lerp(cols[c].redF(), v, t) + b, lerp(cols[c].greenF(), v, t) + b, lerp(cols[c].blueF(), v, t) + b);
		mutedBrush[c].setColor(col);
*/

		float v = 0.4f;
		float t = 0.5f;
		float b = 0.2f;

		QColor col;
		col.setRgbF(lerp(mutedCols[c].redF(), v, t) + b, lerp(mutedCols[c].greenF(), v, t) + b, lerp(mutedCols[c].blueF(), v, t) + b);
		mutedBrush[c].setColor(col);
		
//		mutedBrush[c].setColor(mutedCols[c]);
	}

	if (size < 4)
	{
		painter.setPen(Qt::NoPen);
	}
	else
	{
		painter.setPen(Qt::black);
	}

	uint blockid = 0;
	for (uint pattern = 0; pattern < 1024; pattern++)
	{
		bool isDupe = (maDupes[pattern] != -1) || (maSimilar[pattern].pattern != -1);

		if (hideDupes && isDupe)
			continue;
		
		uint by = blockid / blocksX;
		uint bx = blockid % blocksX;
		int id = pattern;
		for (int y = 0; y < mnHeight; y++)
		{
			for (int x = 0; x < mnWidth; x++)
			{
				uint px = (bx * (blocksizeX + gap)) + (x * size) + gap;
				uint py = (by * (blocksizeY + gap)) + (y * size) + gap;
				int p = mpPartitions[(id * blockPixels) + (y * mnWidth) + x];
				painter.setBrush((isDupe) ? mutedBrush[p] : brush[p]);
				painter.drawRect(px, py, size, size);
			}
		}
		
		if (showCrossDupes && (maSimilar[pattern].pattern != -1))
		{
			painter.setBrush(Qt::NoBrush);
			QPen pen = QPen(Qt::black);
			pen.setWidth(3);
			painter.setPen(pen);
			painter.drawRect((bx * (blocksizeX + gap)) + gap - 1, (by * (blocksizeY + gap)) + gap - 1, blocksizeX + 2, blocksizeY + 2);
			
			if (size < 4)
			{
				painter.setPen(Qt::NoPen);
			}
			else
			{
				painter.setPen(Qt::black);
			}
		}
		
		blockid++;
	}
	
	return pPixmap;
}


