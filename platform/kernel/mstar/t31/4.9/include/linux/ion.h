#ifndef _INCLUDE_LINUX_ION_H_
#define _INCLUDE_LINUX_ION_H_
#include "../../drivers/staging/android/uapi/ion.h"
#include "../../drivers/staging/android/ion/ion.h"
#include "../../drivers/staging/android/ion/ion_priv.h"
#ifdef CONFIG_MP_ION_PATCH_FAKE_MEM
#include "../../drivers/staging/android/ion/ion_priv.h"
#include "../../drivers/staging/android/ion/ion_mstar_cma_heap.h"
#endif
#endif
