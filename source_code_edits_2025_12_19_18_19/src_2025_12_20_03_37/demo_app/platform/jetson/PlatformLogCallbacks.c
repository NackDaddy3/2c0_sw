/*
 * File: PlatformLogCallbacks.c
 * Description: Platform-dependant iRFE logging API implementation for the NVIDIA Jetson
 * Copyright (c) 2024 Silicon Austria Labs GmbH
 */

#include "PlatformLogCallbacks.h"

#include "Util.h"

// TODO: implement async

void PlatformLogCallbacks_error(const char *msg)
{
    printf("ERROR: %s\n", msg);
}

void PlatformLogCallbacks_warning(const char *msg)
{
    printf("WARNING: %s\n", msg);
}

void PlatformLogCallbacks_info(const char *msg)
{
    printf("INFO: %s\n", msg);
}