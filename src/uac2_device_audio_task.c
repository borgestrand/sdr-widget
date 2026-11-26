/* -*- mode: c++; tab-width: 4; c-basic-offset: 4 -*- */
/* uac2_device_audio_task.c - 20261126: selects the UAC2 audio task implementation.
 *
 * uac2_device_audio_task_Legacy.c	The task as of commit 24352e1, all features (default)
 * uac2_device_audio_task_V2.c		SPRX only, slimmed down for 352.8k / 384k. Not built yet
 *
 * Only this file is in the source list. The included file is compiled as part of it
 */

#ifdef FEATURE_UAC2_TASK_V2
	#include "uac2_device_audio_task_V2.c"
#else
	#include "uac2_device_audio_task_Legacy.c"
#endif
