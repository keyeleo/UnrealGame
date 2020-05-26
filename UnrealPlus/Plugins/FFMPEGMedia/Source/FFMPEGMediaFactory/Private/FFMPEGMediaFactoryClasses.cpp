// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FFMPEGMediaSettings.h"


UFFMPEGMediaSettings::UFFMPEGMediaSettings()
    : UseInfiniteBuffer (false)
    , AllowFrameDrop (true)
	, UseHardwareAcceleratedCodecs(true)
    , DisableAudio (false)
    , SpeedUpTricks (false)
    , AudioThreads(0)
    , VideoThreads(0)
    , SyncType  (ESynchronizationType::AudioMaster)
{
	AllowDataCache = false;
	CacheDataFile = TEXT("ffmcd.bin");
}
