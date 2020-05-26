// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "FFMPEGMediaPrivate.h"



#include "Containers/UnrealString.h"
#include "Containers/Queue.h"
#include "IMediaCache.h"
#include "IMediaPlayer.h"
#include "IMediaView.h"
#include "Misc/Timespan.h"


class FFFMPEGMediaTracks;
class IMediaEventSink;


struct AVIOContext;
struct AVFormatContext;
struct AVStream;

struct FRational
{
	int32 num;
	int32 den;

	friend FArchive& operator <<(FArchive& Ar, FRational& Rational)
	{
		Ar << Rational.num;
		Ar << Rational.den;
		return Ar;
	}
};

struct FMediaStreamInfo
{
	int32 codec_id;
	int32 width;
	int32 height;
	int32 ticks_per_frame;
	int32 pix_fmt;
	int32 sample_fmt;
	int32 sample_rate;
	uint32 codec_tag;
	FRational codec_time_base;
	int32 bits_per_coded_sample;
	int32 channels;
	int32 channel_layout;
	int64 bit_rate;
	int32 profile;
	int32 level;
	int32 refs;
	int32 has_b_frames;

	int32 pts_wrap_bits;
	FRational s_time_base;
	FRational avg_frame_rate;
	FRational r_frame_rate;

	TArray<uint8> extradata;

	friend FArchive& operator <<(FArchive& Ar, FMediaStreamInfo& Info)
	{
		Ar << Info.codec_id;
		Ar << Info.width;
		Ar << Info.height;
		Ar << Info.ticks_per_frame;
		Ar << Info.pix_fmt;
		Ar << Info.sample_fmt;
		Ar << Info.sample_rate;
		Ar << Info.codec_tag;
		Ar << Info.codec_id;
		Ar << Info.codec_time_base;
		Ar << Info.bits_per_coded_sample;
		Ar << Info.channels;
		Ar << Info.channel_layout;
		Ar << Info.bit_rate;
		Ar << Info.profile;
		Ar << Info.level;

		Ar << Info.pts_wrap_bits;
		Ar << Info.s_time_base;
		Ar << Info.avg_frame_rate;
		Ar << Info.r_frame_rate;

		Ar << Info.extradata;

		return Ar;
	}
};

struct FMediaCacheData
{
	FMediaStreamInfo AudioInfo;
	FMediaStreamInfo VideoInfo;
	//TMap<F
};

/**
 * Implements a media player using the Windows Media Foundation framework.
 */
class FFFMPEGMediaPlayer
	: public IMediaPlayer
	, protected IMediaCache
    , protected IMediaView
    
{
public:

	/**
	 * Create and initialize a new instance.
	 *
	 * @param InEventSink The object that receives media events from this player.
	 */
	FFFMPEGMediaPlayer(IMediaEventSink& InEventSink);

	/** Virtual destructor. */
	virtual ~FFFMPEGMediaPlayer();

public:

	//~ IMediaPlayer interface

	virtual void Close() override;
	virtual IMediaCache& GetCache() override;
	virtual IMediaControls& GetControls() override;
	virtual FString GetInfo() const override;
	virtual FName GetPlayerName() const override;
	virtual IMediaSamples& GetSamples() override;
	virtual FString GetStats() const override;
	virtual IMediaTracks& GetTracks() override;
	virtual FString GetUrl() const override;
	virtual IMediaView& GetView() override;
	virtual bool Open(const FString& Url, const IMediaOptions* Options) override;
	virtual bool Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* Options) override;
	virtual void TickFetch(FTimespan DeltaTime, FTimespan Timecode) override;
	virtual void TickInput(FTimespan DeltaTime, FTimespan Timecode) override;

public:

	static void SerielizeMediaCacheDataMap();

	static void DeserielizeMediaCacheDataMap();

protected:

	/**
	 * Initialize the native AvPlayer instance.
	 *
	 * @param Archive The archive being used as a media source (optional).
	 * @param Url The media URL being opened.
	 * @param Precache Whether to precache media into RAM if InURL is a local file.
	 * @return true on success, false otherwise.
	 */
	bool InitializePlayer(const TSharedPtr<FArchive, ESPMode::ThreadSafe>& Archive, const FString& Url, bool Precache);

    
	void SetMediaUrl(const FString& InUrl);


	FString GetMediaUrlWithMutex();

private:

	int init_decode(AVFormatContext *s, const FMediaCacheData& InData);

	AVStream *create_stream(AVFormatContext *s, int codec_type);

	void cache_decode(AVFormatContext *s, const FString& InUrl);
private:

    
	/** The media event handler. */
	IMediaEventSink& EventSink;

	/** The URL of the currently opened media. */
	FString MediaUrl;

	FString PlayerUrl;

	FString FailedUrl;

    /** Tasks to be executed on the player thread. */
    TQueue<TFunction<void()>> PlayerTasks;

	
	/** Media streams collection. */
	TSharedPtr<FFFMPEGMediaTracks, ESPMode::ThreadSafe> Tracks;

    /** FFMPEG Callbacks */
    /** Returns 1 when we would like to stop the application */
    static int DecodeInterruptCallback(void *ctx);

    /** This is called when it's reading an Archive instead of an url*/
    static int ReadtStreamCallback(void* ptr, uint8_t* buf, int buf_size);

    /** This is called when it's reading an Archive instead of an url*/
    static int64_t SeekStreamCallback(void *opaque, int64_t offset, int whence);

    /** FFMPEG Functions */

    AVFormatContext*  ReadContext(const TSharedPtr<FArchive, ESPMode::ThreadSafe>& Archive, const FString& Url, bool Precache);

    /** FFMPEG Structs */
    AVFormatContext     *FormatContext;
    AVIOContext         *IOContext;        
    bool                 stopped;
	bool				 reading;
	bool				 opened;
	FDateTime            lastReadingTime;


	FRunnableThread* ReadThread;
	bool abort;
	bool inited;

    TSharedPtr<FArchive, ESPMode::ThreadSafe> CurrentArchive;

	FCriticalSection Mutex;

	FCriticalSection DestructMutex;

	friend class FFFMPEGMediaTracks;

	static TMap<FString, FMediaCacheData> CacheDataMap;
};



