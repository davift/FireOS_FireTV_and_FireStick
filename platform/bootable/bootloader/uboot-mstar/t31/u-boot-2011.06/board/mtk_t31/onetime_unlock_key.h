/*
 * Copyright (c) 2019-2020 Amazon.com, Inc. or its affiliates.  All rights reserved.
 */

#ifndef __ONETIME_UNLOCK_KEY_H__
#define __ONETIME_UNLOCK_KEY_H__

#if defined(UFBL_FEATURE_ONETIME_UNLOCK)
#ifdef CONFIG_MSTAR_M7332
#if defined(CONFIG_MTK_BD_MT168B_10AT_19133_MT5870_M7332_ABC)
#define ONETIME_UNLOCK_KEY \
	"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01" \
	"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01" \
	"\x00\xea\x77\x1a\x3c\x0d\x91\x2c\x97\xfa\xb2\x7c\x93\xe6\xb6\xfa" \
	"\x7f\xa4\xcd\xf5\x93\x7d\x49\x6d\x34\xd8\x6a\x76\xcd\xaf\x02\x0a" \
	"\xf4\xc4\x40\x24\xa5\x8d\xab\xa9\x94\xc7\xe2\x6e\xc3\xad\x70\x08" \
	"\x89\x7e\x55\x32\x8d\x0a\xe6\x4d\x46\xd2\x92\x6f\x1b\xb3\x6e\x6e" \
	"\x53\xeb\x11\xae\x7e\xdc\x5b\x7c\xe0\xdc\xb0\x99\x9f\x67\x0b\x4e" \
	"\xf9\x14\x27\xd4\x7a\x91\x22\xf0\x10\xc7\xa7\x9f\x98\x6a\x0b\x29" \
	"\xe2\x11\x97\x93\xed\x70\xb0\x9d\x30\x2d\xf3\x32\x7a\x47\x4a\xc4" \
	"\x4c\x92\x95\xa2\x02\x68\x14\xb5\x0f\xb3\x04\x8f\x9c\xbd\x3e\xde" \
	"\xd2\xac\x74\xb3\xbd\xeb\x56\x3e\x71\x6e\x3c\x79\x2f\xb8\x96\xa2" \
	"\xaa\x09\x90\xcf\xb2\xfb\x38\xf3\x92\xbb\xd2\xab\xfc\x00\xa9\x79" \
	"\x6a\x90\x68\x58\xb9\x9a\x4e\x28\xbf\x4e\xd2\x76\xa2\x25\xdd\xc0" \
	"\x7d\x8a\x86\x89\x40\x1c\xa1\x5a\xba\x6d\x05\xf5\xdc\x85\x9c\x0f" \
	"\xa6\xec\xdd\xaf\xb4\xc0\x02\x25\x5f\xf7\x5e\x07\x68\x97\xc6\xc8" \
	"\xed\x34\x0f\x98\x32\x21\x1e\x37\x46\xbe\x37\xdc\x7e\x7c\x1b\x02" \
	"\x56\x84\xce\x0a\x7b\x29\xe4\xa1\xd6\xa8\xd4\x82\x65\x4b\x63\x57" \
	"\xb2\x9d\x1d\xd3\x94\xf2\xea\x36\xcf\xcc\x59\x87\x57\x30\xfe\x18" \
	"\x81\x02\x03\x01\x00\x01"
#else
#define ONETIME_UNLOCK_KEY \
	"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01" \
	"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01" \
	"\x00\xaa\x06\x56\x8d\x13\x2e\x35\x05\xa8\xbd\xda\x78\xc7\xc6\x75" \
	"\x70\xa0\xb7\x9c\x14\xec\x0a\x1e\x77\x3b\x67\x25\xca\x49\xea\x3c" \
	"\xc0\x61\xce\xf4\xee\xec\x1d\xb3\x73\x99\x41\x67\xce\x7b\x8a\x63" \
	"\x16\xe2\x68\x73\x94\x59\x12\x24\x70\xe9\x9d\x8e\xf3\x0c\x04\xf1" \
	"\x33\x2e\x5b\xa3\x1c\x46\x99\x80\x60\xe7\x8c\xfe\xf3\x22\xb2\x34" \
	"\x46\xa7\xad\x8c\x60\x7f\x05\x74\xba\x89\x8b\xfd\xf0\x09\x68\x5e" \
	"\xa1\xb7\x75\xf3\xe3\x8c\xc0\x90\x9b\x80\xe7\x3b\xcd\x48\x8a\x9c" \
	"\xa3\x22\x1b\xe0\xc6\x20\x8c\x87\x17\x66\xf2\x27\x18\x76\xb0\x31" \
	"\x64\x6c\xfb\x50\x9d\xde\xf3\x34\x98\x40\x6e\x86\x04\xa5\x0c\xc0" \
	"\x13\x55\x18\x75\x1c\x35\xd6\xf4\x5d\x86\x16\xb2\x21\x77\x47\x31" \
	"\x87\xac\x68\x42\x69\xc0\x71\x0c\xbe\xb2\x81\xac\x50\x25\x35\xf2" \
	"\x0e\xf2\x53\x14\x4d\xab\x4e\x11\x87\x54\x44\x7b\xc2\x79\x87\xdd" \
	"\x42\x8b\x2d\x53\x89\xe1\x2c\x8c\xfe\x7d\x2c\x89\x9f\xde\xd7\x09" \
	"\x4d\x33\x70\x36\x5d\x62\x63\x3e\x75\x44\xa1\x75\x70\xda\xb9\xeb" \
	"\xf7\x1d\x07\xe5\x75\x2c\xc7\x3a\x0c\x31\x51\xf4\x92\xe0\x85\xd7" \
	"\xdd\x1e\xe3\x55\x58\x9c\x33\x63\xc7\x98\xbd\xab\x24\xc1\x38\xff" \
	"\x93\x02\x03\x01\x00\x01"
#endif //CONFIG_MTK_BD_MT168B_10AT_19133_MT5870_M7332_ABC
#else
/* Brandenburg/Kaine/Kayala NA */
#if defined(CONFIG_MTK_BD_MT164B_10AT_M7632_BRANDENBURG)
#define ONETIME_UNLOCK_KEY \
	"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01" \
	"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01" \
	"\x00\xcd\x62\x98\x6f\x64\xa9\x0d\xa3\x2a\x3f\xbb\x79\x72\x79\xf5" \
	"\x08\xb1\x91\x0b\xa0\x9d\xbd\x9b\x97\x8c\x5e\x7d\x63\xa8\xab\xe2" \
	"\x04\x53\x52\x61\x15\x72\x83\x73\x67\xd2\xea\x66\x6a\x7d\x07\x69" \
	"\xb7\x93\x3c\xc9\x9f\xec\x05\xe4\xe0\x15\xd0\xf7\x35\x7d\x93\x57" \
	"\xca\x37\x2e\x05\x18\xaa\xae\x68\x62\xe9\xb0\x7d\x43\xd7\x3a\x1c" \
	"\x48\xa6\x24\xb5\xe0\xab\x35\xf4\x65\x98\x14\xcc\x8f\x01\xe7\xd9" \
	"\xfb\x1f\xd6\xf2\x9e\x56\x5f\x14\x9d\xb2\x26\x69\x93\x13\x34\xde" \
	"\x0d\xa1\x0b\x9c\xc1\xa3\x27\xf7\xf4\x6f\x23\x80\xf5\xc1\xeb\x3f" \
	"\x2e\xfb\x3e\x6b\x17\x35\x96\x64\x82\xf7\xaa\x30\x89\x1f\x5e\x36" \
	"\x5e\xe4\xb6\x3b\x3c\xb6\x77\xe2\xef\xf9\x7a\x8f\x72\x4f\xb6\xe1" \
	"\x7f\x72\x3e\xc7\x2f\x45\xb9\x0a\xf7\xad\x48\x6d\xd2\x5f\xdb\x8c" \
	"\xec\x4b\x89\x59\x74\xd7\x24\x35\xd1\xcb\x63\x68\x71\x02\xa3\x6d" \
	"\xa6\x5f\xd9\x5d\x5f\x01\x05\xbb\xf0\xf5\xa3\x97\x05\xc3\xb9\x9f" \
	"\xce\x45\x12\x13\x56\xb0\x27\x81\x52\x23\x80\x6c\xda\x0d\x9a\xb5" \
	"\xfb\xf1\xb2\x76\x9f\x9b\xd9\x6e\xf2\xbf\xff\x77\x2b\x66\x15\xcd" \
	"\xfc\x96\x38\x83\xcb\xc8\x2a\x86\x20\x0a\x79\x4b\x6d\x15\x39\x21" \
	"\xaf\x02\x03\x01\x00\x01"
#else
/* 3P T31 projects */
#define ONETIME_UNLOCK_KEY \
	"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01" \
	"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01" \
	"\x00\xd0\x51\x6e\x8e\x4b\xed\x43\x90\x15\xf9\xf8\x68\xfb\xeb\xec" \
	"\x2b\x1e\x29\x73\x49\xd6\x9a\x44\xfb\xf1\xd4\xd4\x34\x64\x04\xc9" \
	"\x0e\xdf\x4e\x65\x52\x05\x54\x91\xa8\xea\xb9\xbb\x26\x8a\xe1\x51" \
	"\x03\xc9\x2c\xe3\x06\x17\x66\xe0\x00\x3d\xf4\x25\x89\x11\xcc\xc1" \
	"\x0f\xfc\xc7\x00\x24\xd5\x3a\x84\xe9\x6f\xbf\x7c\x8f\xf0\x28\x69" \
	"\x51\x03\x62\x7f\xaa\xcc\xe3\xae\xa0\x2c\x85\x8c\x3c\x44\x7a\xa4" \
	"\x9a\xc4\x7f\x5c\x80\x50\x79\xcb\x86\x61\x4c\x38\x08\x06\xec\xcb" \
	"\x97\xce\xe2\x05\x7d\xc5\xb6\x5d\x3b\xb4\x6c\x6c\x82\x01\xc3\xa7" \
	"\xc6\xc7\x7b\x3b\x1c\x2e\x8c\xf2\x42\x11\x7a\x99\xd7\xa7\x1b\x0e" \
	"\x0f\x3a\x0e\x39\xb3\xe6\x63\xb7\x9d\x44\x4d\x20\x64\x95\x0c\xbe" \
	"\xed\xbe\x57\x36\x2d\x10\x7a\xd9\x41\x63\x30\x0a\xc9\x1e\x8e\x5e" \
	"\xcd\xd3\xe6\x9a\xe5\x85\x12\xda\xde\x12\x24\x44\x96\x4d\xe1\xe3" \
	"\x33\x43\x54\x3f\xc1\x3b\xe1\x62\xc1\xfd\xa8\x68\x06\xa4\xcd\xb4" \
	"\xf1\xdf\x3a\xe0\x4c\x21\x7b\xe4\x2d\x18\x32\x8f\x6a\xfc\xc0\xf0" \
	"\x21\x2b\x27\xf1\xb4\xcb\x4f\x12\xe7\x5d\x28\x6a\xc7\x63\x54\x27" \
	"\x1f\xf7\xcd\xb7\xf2\xf8\x1d\x83\x71\xfe\xc1\x76\x65\xaf\x3b\xd5" \
	"\xa5\x02\x03\x01\x00\x01"
#endif
#endif  // CONFIG_MSTAR_M7332
/* ABC EU */
#define ONETIME_1P_UNLOCK_KEY \
	"\x30\x82\x01\x22\x30\x0d\x06\x09\x2a\x86\x48\x86\xf7\x0d\x01\x01" \
	"\x01\x05\x00\x03\x82\x01\x0f\x00\x30\x82\x01\x0a\x02\x82\x01\x01" \
	"\x00\xc4\x0b\x81\x9f\x89\x64\xf0\x05\x0a\x3a\x64\xb4\x54\x43\xac" \
	"\xd9\x09\x3a\xfb\x15\xae\x00\xf3\xff\xf3\x31\x4e\xca\x4e\x9f\x43" \
	"\xa0\xe6\x36\x7a\x57\xcd\xb0\x2b\xbb\x52\xdc\x2a\x45\x56\x8b\x22" \
	"\x32\x8e\x28\x59\x85\x4a\x7b\x50\xdd\x3f\x1b\x35\xd4\x87\x52\xa6" \
	"\x01\xe5\xc5\x2a\xf0\x3c\x8e\x5b\x37\x4e\x34\x72\x8a\x5a\xcf\xaa" \
	"\x0f\x13\x88\x1e\x5c\xdd\x58\xf8\x3b\xdc\x00\x7c\x01\xff\x9b\xda" \
	"\xe4\xee\x3b\x9c\x74\x14\xf9\xc4\xc7\x12\x02\xa2\xb3\x5f\x10\x3a" \
	"\xb3\x69\x56\x17\x1e\xdf\xcd\xd1\x86\x69\x8c\xa4\x9a\x2c\xd0\x21" \
	"\xcb\x80\xc7\xcd\x88\xae\x21\x0a\x2e\xc3\x19\x09\x10\x56\x85\xd9" \
	"\x2c\x90\xea\xe7\xc3\xa5\xdb\xdf\x8c\x97\x3c\xb2\xe7\xf6\xff\x82" \
	"\xab\x68\xac\x2d\xbf\x8f\x34\x1c\x99\xc0\xb3\x8a\xe9\xca\x07\x9b" \
	"\x2b\xbf\x5d\x0e\x3f\x24\xa9\x9b\x40\x83\x1b\x12\x24\x4b\x70\xc8" \
	"\xa0\xc7\xa5\xe0\xb0\x96\x6c\x87\x76\xdb\x8e\xe5\xa1\x9a\xff\xde" \
	"\x11\x2a\x66\x9c\xbd\xdd\x6e\x32\xad\xea\x5e\xec\x4c\xd4\xfe\x42" \
	"\x25\xcc\x5f\xfb\x32\x4f\x49\x71\x3d\x14\x07\xf3\x42\x3d\x18\x66" \
	"\xb6\xf4\x97\x44\x07\x48\x01\xae\xb4\xfe\xe1\xa9\xde\x64\xf2\x36" \
	"\x55\x02\x03\x01\x00\x01"
#endif  // UFBL_FEATURE_ONETIME_UNLOCK
#endif  // __ONETIME_UNLOCK_KEY_H__

