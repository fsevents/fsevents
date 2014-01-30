/*
** © 2014 by Philipp Dunkel <pip@pipobscure.com>
** Licensed under MIT License.
*/

#ifndef fse_types_h
#define fse_types_h

#include <CoreServices/CoreServices.h>

typedef void (*notify_callback)(void *data);
typedef void (*event_callback)(void *ctx, const char *path, UInt32 flags, UInt64 id);

#endif // fse_types_h
