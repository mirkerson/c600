/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef   __SPARSE_FORMAT_H__
#define   __SPARSE_FORMAT_H__

typedef struct sparse_header
{
  unsigned int	magic;		/* 0xed26ff3a */
  unsigned short	major_version;	/* (0x1) - reject images with higher major versions */
  unsigned short	minor_version;	/* (0x0) - allow images with higer minor versions */
  unsigned short	file_hdr_sz;	/* 28 bytes for first revision of the file format */
  unsigned short	chunk_hdr_sz;	/* 12 bytes for first revision of the file format */
  unsigned int	blk_sz;		/* block size in bytes, must be a multiple of 4 (4096) */
  unsigned int	total_blks;	/* total blocks in the non-sparse output image */
  unsigned int	total_chunks;	/* total chunks in the sparse input image */
  unsigned int	image_checksum; /* CRC32 checksum of the original data, counting "don't care" */
				/* as 0. Standard 802.3 polynomial, use a Public Domain */
				/* table implementation */
}
__attribute__ ((packed)) sparse_header_t;

#define SPARSE_HEADER_MAJOR_VER 1
#define SPARSE_HEADER_MAGIC	0xed26ff3a

#define CHUNK_TYPE_RAW		0xCAC1
#define CHUNK_TYPE_FILL		0xCAC2
#define CHUNK_TYPE_DONT_CARE	0xCAC3

typedef struct chunk_header
{
  unsigned short	chunk_type;	/* 0xCAC1 -> raw; 0xCAC2 -> fill; 0xCAC3 -> don't care */
  unsigned short	reserved1;
  unsigned int	chunk_sz;	/* in blocks in output image */
  unsigned int	total_sz;	/* in bytes of chunk input file including chunk header and data */
}
__attribute__ ((packed)) chunk_header_t;

/* Following a Raw or Fill chunk is data.  For a Raw chunk, it's the data in chunk_sz * blk_sz.
 *  For a Fill chunk, it's 4 bytes of the fill data.
 */
#define   SPARSE_FORMAT_TYPE_TOTAL_HEAD       0xff00
#define   SPARSE_FORMAT_TYPE_CHUNK_HEAD       0xff01
#define   SPARSE_FORMAT_TYPE_CHUNK_DATA       0xff02


#endif   //__SPARSE_FORMAT_H__

