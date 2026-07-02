/*
 * File: PlatformLogCallbacks.h
 * Description: Platform-dependant iRFE logging API implementation for the NVIDIA Jetson
 * Project: MIMOrad
 * Created Date: Tuesday, 23 April 2024, 10:07
 * Author: Daniel Klepatsch (daniel.klepatsch@silicon-austria.com)
 * ---------------
 * Last Modified: Tuesday, 23 April 2024, 16:40
 * Modified By: Daniel Klepatsch
 * ---------------
 * Copyright (c) 2024 Silicon Austria Labs GmbH
 */

#ifndef PLATFORMLOGCALLBACKS_H
#define PLATFORMLOGCALLBACKS_H

void PlatformLogCallbacks_error(const char *msg);
void PlatformLogCallbacks_warning(const char *msg);
void PlatformLogCallbacks_info(const char *msg);

#endif // PLATFORMLOGCALLBACKS_H