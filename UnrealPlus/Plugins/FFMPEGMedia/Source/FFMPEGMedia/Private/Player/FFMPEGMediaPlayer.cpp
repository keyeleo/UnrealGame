// Copyright 1998-2018 Epic Games, Inc. All Rights Reserved.

#include "FFMPEGMediaPlayer.h"


#include "Async/Async.h"
#include "IMediaEventSink.h"
#include "IMediaOptions.h"
#include "Misc/Optional.h"
#include "UObject/Class.h"
#include "LambdaFunctionRunnable.h"

#include "FFMPEGMediaTracks.h"
#include "FFMPEGMediaSettings.h"

#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Misc/FileHelper.h"

extern  "C" {
#include "libavformat/avformat.h"
#include "libavutil/time.h"
}


#define FF_INPUT_BUFFER_PADDING_SIZE 32

/* FWmfVideoPlayer structors
 *****************************************************************************/
TMap<FString, FMediaCacheData> FFFMPEGMediaPlayer::CacheDataMap;

FFFMPEGMediaPlayer::FFFMPEGMediaPlayer(IMediaEventSink& InEventSink)
	:
	EventSink(InEventSink)
	, Tracks(MakeShared<FFFMPEGMediaTracks, ESPMode::ThreadSafe>())
{
	check(Tracks.IsValid());

	IOContext = nullptr;
	FormatContext = nullptr;
	stopped = true;
	reading = false;
	opened = false;
	inited = false;
	abort = false;
	ReadThread = nullptr;
	Tracks->TargetPlayer = this;
}


FFFMPEGMediaPlayer::~FFFMPEGMediaPlayer()
{
	Close();
	abort = true;

	if (ReadThread)
	{
//		ReadThread->Kill(false);
		//delete ReadThread;
		ReadThread = nullptr;
	}
}

/* IMediaPlayer interface
 *****************************************************************************/

void FFFMPEGMediaPlayer::Close()
{
	if (Tracks->GetState() == EMediaState::Closed/* || stopped*/)
	{
		return;
	}
	// reset player
	stopped = true;

//	if (opened)
	{
		SetMediaUrl("");
		Tracks->Shutdown();

		// notify listeners
	}

	//EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
	EventSink.ReceiveMediaEvent(EMediaEvent::MediaClosed);
}


IMediaCache& FFFMPEGMediaPlayer::GetCache()
{
	return *this;
}


IMediaControls& FFFMPEGMediaPlayer::GetControls()
{
	return *Tracks;
}


FString FFFMPEGMediaPlayer::GetInfo() const
{
	return Tracks->GetInfo();
}


FName FFFMPEGMediaPlayer::GetPlayerName() const
{
	static FName PlayerName(TEXT("FFMPEGMedia"));
	return PlayerName;
}


IMediaSamples& FFFMPEGMediaPlayer::GetSamples()
{
	return *Tracks;
}


FString FFFMPEGMediaPlayer::GetStats() const
{
	FString Result;
	Tracks->AppendStats(Result);

	return Result;
}


IMediaTracks& FFFMPEGMediaPlayer::GetTracks()
{
	return *Tracks;
}


FString FFFMPEGMediaPlayer::GetUrl() const
{
	return PlayerUrl;
}


IMediaView& FFFMPEGMediaPlayer::GetView()
{
	return *this;
}


bool FFFMPEGMediaPlayer::Open(const FString& Url, const IMediaOptions* Options)
{
	Close();
	if (Url.IsEmpty())
	{
		return false;
	}

	const bool Precache = (Options != nullptr) ? Options->GetMediaOption("PrecacheFile", false) : false;

	return InitializePlayer(nullptr, Url, Precache);
}


bool FFFMPEGMediaPlayer::Open(const TSharedRef<FArchive, ESPMode::ThreadSafe>& Archive, const FString& OriginalUrl, const IMediaOptions* /*Options*/)
{
	Close();
	if (Archive->TotalSize() == 0)
	{
		UE_LOG(LogFFMPEGMedia, Verbose, TEXT("Player %p: Cannot open media from archive (archive is empty)"), this);
		return false;
	}

	if (OriginalUrl.IsEmpty())
	{
		UE_LOG(LogFFMPEGMedia, Verbose, TEXT("Player %p: Cannot open media from archive (no original URL provided)"), this);
		return false;
	}

	return InitializePlayer(Archive, OriginalUrl, false);
}


void FFFMPEGMediaPlayer::TickFetch(FTimespan DeltaTime, FTimespan Timecode)
{
	bool MediaSourceChanged = false;
	bool TrackSelectionChanged = false;

	Tracks->GetFlags(MediaSourceChanged, TrackSelectionChanged);

	if (MediaSourceChanged)
	{
		EventSink.ReceiveMediaEvent(EMediaEvent::TracksChanged);
	}

	if (TrackSelectionChanged)
	{

	}

	if (MediaSourceChanged || TrackSelectionChanged)
	{
		Tracks->ClearFlags();
	}
}


void FFFMPEGMediaPlayer::TickInput(FTimespan DeltaTime, FTimespan Timecode)
{
	Tracks->TickInput(DeltaTime, Timecode);

	// forward session events
	TArray<EMediaEvent> OutEvents;
	Tracks->GetEvents(OutEvents);

	for (const auto& Event : OutEvents)
	{
		EventSink.ReceiveMediaEvent(Event);

// 		if (Event == EMediaEvent::MediaOpened)
// 		{
// 			if (stopped)
// 			{
// 				SetMediaUrl("");
// 				Tracks->Shutdown();
// 			}
// 			else
// 			{
// 				opened = true;
// 			}
// 		}
	}

	// process deferred tasks
	TFunction<void()> Task;

	while (PlayerTasks.Dequeue(Task))
	{
		Task();
	}
}

/* FFFMPEGMediaPlayer implementation
 *****************************************************************************/

void FFFMPEGMediaPlayer::SetMediaUrl(const FString& InUrl)
{
	FScopeLock Lock(&Mutex);
	MediaUrl = InUrl;
}

FString FFFMPEGMediaPlayer::GetMediaUrlWithMutex()
{
	FScopeLock Lock(&Mutex);
	return MediaUrl;
}

bool FFFMPEGMediaPlayer::InitializePlayer(const TSharedPtr<FArchive, ESPMode::ThreadSafe>& Archive, const FString& Url, bool Precache)
{
	UE_LOG(LogFFMPEGMedia, Verbose, TEXT("Player %llx: Initializing %s (archive = %s, precache = %s)"), this, *Url, Archive.IsValid() ? TEXT("yes") : TEXT("no"), Precache ? TEXT("yes") : TEXT("no"));

	const auto Settings = GetDefault<UFFMPEGMediaSettings>();
	check(Settings != nullptr);

	SetMediaUrl(Url);
	FailedUrl = "";
	stopped = false;
	opened = false;

	EventSink.ReceiveMediaEvent(EMediaEvent::MediaConnecting);

	if (!inited)
	{
		ReadThread = LambdaFunctionRunnable::RunThreaded("ReadThread", [Archive, Precache, TracksPtr = TWeakPtr<FFFMPEGMediaTracks, ESPMode::ThreadSafe>(Tracks), this]() {
			while (!abort)
			{
				//FScopeLock Lock(&DestructMutex);
				FString lastUrl = GetMediaUrlWithMutex();
				if (!lastUrl.IsEmpty() && !stopped/*FailedUrl != lastUrl*/)
				{
					AVFormatContext* context = ReadContext(Archive, lastUrl, Precache);
					if (context) {
						FailedUrl = "";
						Tracks->Initialize(context, lastUrl);
					}
					else
					{
						//SetMediaUrl("");
						//Tracks->Shutdown();
					}

					if (FormatContext) {
						avformat_close_input(&FormatContext);
						FormatContext = nullptr;
					}

					if (IOContext) {
						av_free(IOContext->buffer);
						av_free(IOContext);
						IOContext = nullptr;
					}

					//EventSink.ReceiveMediaEvent(EMediaEvent::MediaClosed);

					//PlayerUrl = "";
				}
				/*else
				{
					Tracks->Shutdown();
				}*/
			}
		});

		inited = true;
	}
	// initialize presentation on a separate thread
	/*const EAsyncExecution Execution = Precache ? EAsyncExecution::Thread : EAsyncExecution::ThreadPool;

	Async<void>(Execution, [Archive, Precache, TracksPtr = TWeakPtr<FFFMPEGMediaTracks, ESPMode::ThreadSafe>(Tracks), ThisPtr=this]()
	{
		TSharedPtr<FFFMPEGMediaTracks, ESPMode::ThreadSafe> PinnedTracks = TracksPtr.Pin();
		if (PinnedTracks.IsValid() )
		{
			FScopeLock Lock(&ThisPtr->Mutex);
			AVFormatContext* context = ThisPtr->ReadContext(Archive, ThisPtr->MediaUrl, Precache);
			ThisPtr->openning = false;
			if (context) {
				PinnedTracks->Initialize(context, ThisPtr->MediaUrl);
			}

			if (ThisPtr->FormatContext) {
				ThisPtr->FormatContext->video_codec = NULL;
				ThisPtr->FormatContext->audio_codec = NULL;
				avformat_close_input(&ThisPtr->FormatContext);
				ThisPtr->FormatContext = nullptr;
			}

			if (ThisPtr->IOContext) {
				av_free(ThisPtr->IOContext->buffer);
				av_free(ThisPtr->IOContext);
				ThisPtr->IOContext = nullptr;
			}
		}
	});*/

	return true;
}

int FFFMPEGMediaPlayer::DecodeInterruptCallback(void *ctx) {
	FFFMPEGMediaPlayer* player = static_cast<FFFMPEGMediaPlayer*>(ctx);
	FTimespan d = FDateTime::Now() - player->lastReadingTime;
	if (player->stopped || (player->reading && d.GetTotalSeconds() > 5.0f))
	{
		return 1;
	}
	else
	{

		return 0;
	}
}

int FFFMPEGMediaPlayer::ReadtStreamCallback(void* opaque, uint8_t* buf, int buf_size) {
	FFFMPEGMediaPlayer* player = static_cast<FFFMPEGMediaPlayer*>(opaque);
	int64 Position = player->CurrentArchive->Tell();
	int64 Size = player->CurrentArchive->TotalSize();
	int64 BytesToRead = buf_size;
	if (BytesToRead > (int64)Size)
	{
		BytesToRead = Size;
	}

	if ((Size - BytesToRead) < player->CurrentArchive->Tell())
	{
		BytesToRead = Size - Position;
	}
	if (BytesToRead > 0)
	{
		player->CurrentArchive->Serialize(buf, BytesToRead);
	}

	player->CurrentArchive->Seek(Position + BytesToRead);

	return BytesToRead;
}

int64_t FFFMPEGMediaPlayer::SeekStreamCallback(void *opaque, int64_t offset, int whence) {
	FFFMPEGMediaPlayer* player = static_cast<FFFMPEGMediaPlayer*>(opaque);
	if (whence == AVSEEK_SIZE) {
		return player->CurrentArchive->TotalSize();
	}
	int64_t pos = player->CurrentArchive->Tell();
	player->CurrentArchive->Seek(pos + offset);
	return player->CurrentArchive->Tell();

}

AVFormatContext* FFFMPEGMediaPlayer::ReadContext(const TSharedPtr<FArchive, ESPMode::ThreadSafe>& Archive, const FString& Url, bool Precache) {
	AVDictionary *format_opts = NULL;
	int scan_all_pmts_set = 0;

	FormatContext = avformat_alloc_context();

	FormatContext->interrupt_callback.callback = DecodeInterruptCallback;
	FormatContext->interrupt_callback.opaque = this;

	reading = false;

	if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
		av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
		scan_all_pmts_set = 1;
	}
	int err = 0;

	if (!Archive.IsValid()) {
		if (Url.StartsWith(TEXT("file://")) && Url.Len() > 7)
		{
			const TCHAR* FilePath = &Url[7];
			err = avformat_open_input(&FormatContext, TCHAR_TO_UTF8(FilePath), NULL, &format_opts);
		}
		else {

			if (Url.StartsWith(TEXT("rtsp://")))
			{
				av_dict_set(&format_opts, "rtsp_transport", "tcp", 0);
				av_dict_set(&format_opts, "max_delay", "100", 0);
				av_dict_set_int(&format_opts, "stimeout", (int64_t)10000000, 0);

				//FormatContext->probesize = 100 * 1024;
				FormatContext->max_analyze_duration = 2 * AV_TIME_BASE;
			}

			err = avformat_open_input(&FormatContext, TCHAR_TO_UTF8(*Url), NULL, &format_opts);
		}
	}
	else {
		CurrentArchive = Archive;
		const int ioBufferSize = 32768;
		unsigned char * ioBuffer = (unsigned char *)av_malloc(ioBufferSize + FF_INPUT_BUFFER_PADDING_SIZE);
		IOContext = avio_alloc_context(ioBuffer, ioBufferSize, 0, this, ReadtStreamCallback, NULL, SeekStreamCallback);
		FormatContext->pb = IOContext;
		err = avformat_open_input(&FormatContext, "InMemoryFile", NULL, &format_opts);
	}

	if (err < 0) {
		char errbuf[128];
		const char *errbuf_ptr = errbuf;
#if PLATFORM_WINDOWS
		if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
			strerror_s(errbuf, 128, AVUNERROR(err));
#else
		if (av_strerror(err, errbuf, sizeof(errbuf)) < 0)
			errbuf_ptr = strerror(AVUNERROR(err));
#endif
		PlayerTasks.Enqueue([=]() {
			EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpenFailed);
		});

		if (FormatContext) {
			avformat_close_input(&FormatContext);
			FormatContext = nullptr;
		}
		stopped = true;
		FailedUrl = Url;
		return FormatContext;
	}

	if (scan_all_pmts_set)
		av_dict_set(&format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE);

	AVDictionaryEntry *t = av_dict_get(format_opts, "", NULL, AV_DICT_IGNORE_SUFFIX);
	if (t) {
		UE_LOG(LogFFMPEGMedia, Error, TEXT("Option %s not found"), UTF8_TO_TCHAR(t->key));

		PlayerTasks.Enqueue([=]() {
			EventSink.ReceiveMediaEvent(EMediaEvent::MediaOpenFailed);
			//EventSink.ReceiveMediaEvent(EMediaEvent::MediaClosed);
		});
		if (FormatContext) {
			avformat_close_input(&FormatContext);
			FormatContext = nullptr;
		}
		stopped = true;
		FailedUrl = Url;
		return FormatContext;
	}

	av_dict_free(&format_opts);

	/*if (CacheDataMap.Contains(Url))
	{
		err = init_decode(FormatContext, CacheDataMap[Url]);
	}
	else
	*/
	{
		av_format_inject_global_side_data(FormatContext);

		for (int32 i = 0; i < 3; i++)
		{
			err = avformat_find_stream_info(FormatContext, NULL);
			if (err >= 0)
			{
				break;
			}
		}

		if (err >= 0)
		{
			cache_decode(FormatContext, Url);
		}
	}

	if (err < 0)
	{
		if (FormatContext) {
			avformat_close_input(&FormatContext);
			FormatContext = nullptr;
		}
		stopped = true;
		FailedUrl = Url;
		return FormatContext;
	}

	PlayerUrl = Url;
	if (FormatContext->pb)
		FormatContext->pb->eof_reached = 0; // FIXME hack, ffplay maybe should not use avio_feof() to test for the end
	return FormatContext;
}


void FFFMPEGMediaPlayer::SerielizeMediaCacheDataMap()
{
	TArray<uint8> Buffers;
	FMemoryWriter Writer(Buffers, true);
	int32 Num = CacheDataMap.Num();
	Writer << Num;
	for (auto It = CacheDataMap.CreateIterator(); It; ++It)
	{
		//FString Url = It.Key();
		Writer << It.Key();

		FMediaCacheData& Data = It.Value();
		Writer << Data.AudioInfo;
		Writer << Data.VideoInfo;
	}

	FString CacheFile = FPaths::ProjectSavedDir() / GetDefault<UFFMPEGMediaSettings>()->CacheDataFile;
	FFileHelper::SaveArrayToFile(Buffers, *CacheFile);
}

void FFFMPEGMediaPlayer::DeserielizeMediaCacheDataMap()
{
	TArray<uint8> Buffers;
	FString CacheFile = FPaths::ProjectSavedDir() / GetDefault<UFFMPEGMediaSettings>()->CacheDataFile;
	if (IFileManager::Get().FileExists(*CacheFile))
	{
		FFileHelper::LoadFileToArray(Buffers, *CacheFile);
		FMemoryReader Reader(Buffers);
		int32 Num = 0;
		Reader << Num;
		for (int32 i = 0; i < Num; i++)
		{
			FString Url;
			Reader << Url;
			FMediaCacheData& Data = CacheDataMap.FindOrAdd(Url);
			Reader << Data.AudioInfo;
			Reader << Data.VideoInfo;
		}
	}
}


enum {
	FLV_TAG_TYPE_AUDIO = 0x08,
	FLV_TAG_TYPE_VIDEO = 0x09,
	FLV_TAG_TYPE_META = 0x12,
};

AVStream* FFFMPEGMediaPlayer::create_stream(AVFormatContext *s, int codec_type)
{
	AVStream *st = avformat_new_stream(s, NULL);
	if (!st)
		return NULL;
	//st->codecpar->codec_type = (AVMediaType)codec_type;
	return st;
}


int FFFMPEGMediaPlayer::init_decode(AVFormatContext *s, const FMediaCacheData& InData)
{
	int video_index = -1;
	int audio_index = -1;
	int ret = -1;

	if (!s)
		return ret;

	/*
	Get video stream index, if no video stream then create it.
	And audio so on.
	*/
	if (0 == s->nb_streams) {
		create_stream(s, AVMEDIA_TYPE_VIDEO);
		create_stream(s, AVMEDIA_TYPE_AUDIO);
		video_index = 0;
		audio_index = 1;
	}
	else if (1 == s->nb_streams) {
		if (AVMEDIA_TYPE_VIDEO == s->streams[0]->codecpar->codec_type) {
			create_stream(s, AVMEDIA_TYPE_AUDIO);
			video_index = 0;
			audio_index = 1;
		}
		else if (AVMEDIA_TYPE_AUDIO == s->streams[0]->codecpar->codec_type) {
			create_stream(s, AVMEDIA_TYPE_VIDEO);
			video_index = 1;
			audio_index = 0;
		}
	}
	else if (2 == s->nb_streams) {
		if (AVMEDIA_TYPE_VIDEO == s->streams[0]->codecpar->codec_type) {
			video_index = 0;
			audio_index = 1;
		}
		else if (AVMEDIA_TYPE_VIDEO == s->streams[1]->codecpar->codec_type) {
			video_index = 1;
			audio_index = 0;
		}
	}

	/*Error. I can't find video stream.*/
	if (video_index != 0 && video_index != 1)
		return ret;

	//Init the audio codecpar(AAC).
	/*
	s->streams[audio_index]->codecpar->codec_id = (AVCodecID)InData.AudioInfo.codec_id;
	s->streams[audio_index]->codecpar->sample_rate = InData.AudioInfo.sample_rate;
	s->streams[audio_index]->codecpar->time_base.den = InData.AudioInfo.codec_time_base.den;
	s->streams[audio_index]->codecpar->time_base.num = InData.AudioInfo.codec_time_base.num;
	s->streams[audio_index]->codecpar->bits_per_coded_sample = InData.AudioInfo.bits_per_coded_sample; //
	s->streams[audio_index]->codecpar->channels = InData.AudioInfo.channels;
	s->streams[audio_index]->codecpar->channel_layout = InData.AudioInfo.channel_layout;
	s->streams[audio_index]->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
	s->streams[audio_index]->codecpar->bit_rate = InData.AudioInfo.bit_rate;
	s->streams[audio_index]->codecpar->refs = InData.AudioInfo.refs;
	s->streams[audio_index]->codecpar->sample_fmt = (AVSampleFormat)InData.AudioInfo.sample_fmt;
	s->streams[audio_index]->codecpar->profile = InData.AudioInfo.profile;
	s->streams[audio_index]->codecpar->level = InData.AudioInfo.level;

	s->streams[audio_index]->pts_wrap_bits = InData.AudioInfo.pts_wrap_bits;
	s->streams[audio_index]->time_base.den = InData.AudioInfo.s_time_base.den;
	s->streams[audio_index]->time_base.num = InData.AudioInfo.s_time_base.num;

	//Init the video codecpar(H264).
	s->streams[video_index]->codecpar->codec_id = (AVCodecID)InData.VideoInfo.codec_id;
	s->streams[video_index]->codecpar->width = InData.VideoInfo.width;
	s->streams[video_index]->codecpar->height = InData.VideoInfo.height;
	s->streams[video_index]->codecpar->ticks_per_frame = InData.VideoInfo.ticks_per_frame;
	s->streams[video_index]->codecpar->pix_fmt = (AVPixelFormat)InData.VideoInfo.pix_fmt;
	s->streams[video_index]->codecpar->time_base.den = InData.VideoInfo.codec_time_base.den;
	s->streams[video_index]->codecpar->time_base.num = InData.VideoInfo.codec_time_base.num;
	s->streams[video_index]->codecpar->sample_fmt = (AVSampleFormat)InData.VideoInfo.sample_fmt;
	//s->streams[video_index]->codecpar->frame_size = 0;
	//s->streams[video_index]->codecpar->frame_number = 7;
	s->streams[video_index]->codecpar->has_b_frames = InData.VideoInfo.has_b_frames;
	s->streams[video_index]->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	s->streams[video_index]->codecpar->codec_tag = InData.VideoInfo.codec_tag;
	s->streams[video_index]->codecpar->bit_rate = InData.VideoInfo.bit_rate;
	s->streams[video_index]->codecpar->refs = InData.VideoInfo.refs;
	s->streams[video_index]->codecpar->sample_rate = InData.VideoInfo.sample_rate;
	s->streams[video_index]->codecpar->channels = InData.VideoInfo.channels;
	s->streams[video_index]->codecpar->profile = InData.VideoInfo.profile;
	s->streams[video_index]->codecpar->level = InData.VideoInfo.level;
	s->streams[video_index]->pts_wrap_bits = InData.VideoInfo.pts_wrap_bits;
	s->streams[video_index]->time_base.den = InData.VideoInfo.s_time_base.den;
	s->streams[video_index]->time_base.num = InData.VideoInfo.s_time_base.num;
	s->streams[video_index]->avg_frame_rate.den = InData.VideoInfo.avg_frame_rate.den;
	s->streams[video_index]->avg_frame_rate.num = InData.VideoInfo.avg_frame_rate.num;

	int32 size = InData.VideoInfo.extradata.Num();
	s->streams[video_index]->codecpar->extradata = (uint8_t*)av_mallocz(size + FF_INPUT_BUFFER_PADDING_SIZE);
	if (NULL != s->streams[video_index]->codecpar->extradata)
	{
		memcpy(s->streams[video_index]->codecpar->extradata, InData.VideoInfo.extradata.GetData(), size);

	}
	s->streams[video_index]->codecpar->extradata_size = size;
	s->streams[video_index]->r_frame_rate.den = InData.VideoInfo.r_frame_rate.den;
	s->streams[video_index]->r_frame_rate.num = InData.VideoInfo.r_frame_rate.num;
	*/
	/*
	char option_key[] = "encoder";
	char option_value[] = "Lavf57.0.100";
	ret = av_dict_set(&(s->metadata), option_key, option_value, 0);
	AVDictionaryEntry *tag = NULL;
	tag = av_dict_get((s->metadata), "", tag, AV_DICT_IGNORE_SUFFIX);
	*/

	s->duration = 0;
	s->start_time = 0;
	s->bit_rate = 0;
	s->iformat->flags = 0;
	s->duration_estimation_method = AVFMT_DURATION_FROM_STREAM;

	/*
	AVPacket packet;
	av_init_packet(&packet);
	while (true)
	{
		int ret1 = av_read_frame(s, &packet);
		if (packet.flags & AV_PKT_FLAG_KEY)
		{
			break;
		}
	}
	add_to_pktbuf(&(s->packet_buffer), &packet, &(s->packet_buffer_end));
	*/

	/*Update the AVFormatContext Info*/
	s->nb_streams = 2;

	/*
	something wrong.
	TODO: find out the 'pos' means what.
	then set it.
	*/
	s->pb->pos = (int64_t)s->pb->buf_end;

	return ret;
}

void FFFMPEGMediaPlayer::cache_decode(AVFormatContext *s, const FString& InUrl)
{
	if (GetDefault<UFFMPEGMediaSettings>()->AllowDataCache)
	{
		FMediaCacheData& InData = CacheDataMap.FindOrAdd(InUrl);

		int video_index = -1;
		int audio_index = -1;
		if (2 == s->nb_streams) {
			if (AVMEDIA_TYPE_VIDEO == s->streams[0]->codecpar->codec_type) {
				video_index = 0;
				audio_index = 1;
			}
			else if (AVMEDIA_TYPE_VIDEO == s->streams[1]->codecpar->codec_type) {
				video_index = 1;
				audio_index = 0;
			}
		}

		if (video_index != 0 && video_index != 1)
			return;

		/*
		InData.AudioInfo.codec_id = (int32)s->streams[audio_index]->codecpar->codec_id;
		InData.AudioInfo.sample_rate = s->streams[audio_index]->codecpar->sample_rate;
		InData.AudioInfo.codec_time_base.den = s->streams[audio_index]->codecpar->time_base.den;
		InData.AudioInfo.codec_time_base.num = s->streams[audio_index]->codecpar->time_base.num;
		InData.AudioInfo.bits_per_coded_sample = s->streams[audio_index]->codecpar->bits_per_coded_sample; //
		InData.AudioInfo.channels = s->streams[audio_index]->codecpar->channels;
		InData.AudioInfo.channel_layout = s->streams[audio_index]->codecpar->channel_layout;
		InData.AudioInfo.bit_rate = s->streams[audio_index]->codecpar->bit_rate;
		InData.AudioInfo.refs = s->streams[audio_index]->codecpar->refs;
		InData.AudioInfo.sample_fmt = (int32)s->streams[audio_index]->codecpar->sample_fmt;
		InData.AudioInfo.profile = s->streams[audio_index]->codecpar->profile;
		InData.AudioInfo.level = s->streams[audio_index]->codecpar->level;

		InData.AudioInfo.pts_wrap_bits = s->streams[audio_index]->pts_wrap_bits;
		InData.AudioInfo.s_time_base.den = s->streams[audio_index]->time_base.den;
		InData.AudioInfo.s_time_base.num = s->streams[audio_index]->time_base.num;

		//Init the video codecpar(H264).
		InData.VideoInfo.codec_id = (int32)s->streams[video_index]->codecpar->codec_id;
		InData.VideoInfo.width = s->streams[video_index]->codecpar->width;
		InData.VideoInfo.height = s->streams[video_index]->codecpar->height;
		InData.VideoInfo.ticks_per_frame = s->streams[video_index]->codecpar->ticks_per_frame;
		InData.VideoInfo.pix_fmt = (int32)s->streams[video_index]->codecpar->pix_fmt;
		InData.VideoInfo.codec_time_base.den = s->streams[video_index]->codecpar->time_base.den;
		InData.VideoInfo.codec_time_base.num = s->streams[video_index]->codecpar->time_base.num;
		InData.VideoInfo.sample_fmt =  (int32)s->streams[video_index]->codecpar->sample_fmt;
		//s->streams[video_index]->codecpar->frame_size = 0;
		//s->streams[video_index]->codecpar->frame_number = 7;
		InData.VideoInfo.has_b_frames = s->streams[video_index]->codecpar->has_b_frames;
		//s->streams[video_index]->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
		InData.VideoInfo.codec_tag = s->streams[video_index]->codecpar->codec_tag;
		InData.VideoInfo.bit_rate = s->streams[video_index]->codecpar->bit_rate;
		InData.VideoInfo.refs = s->streams[video_index]->codecpar->refs;
		InData.VideoInfo.sample_rate = s->streams[video_index]->codecpar->sample_rate;
		InData.VideoInfo.channels = s->streams[video_index]->codecpar->channels;
		InData.VideoInfo.profile = s->streams[video_index]->codecpar->profile;
		InData.VideoInfo.level = s->streams[video_index]->codecpar->level;
		InData.VideoInfo.pts_wrap_bits = s->streams[video_index]->pts_wrap_bits;
		InData.VideoInfo.s_time_base.den = s->streams[video_index]->time_base.den;
		InData.VideoInfo.s_time_base.num = s->streams[video_index]->time_base.num;
		InData.VideoInfo.avg_frame_rate.den = s->streams[video_index]->avg_frame_rate.den;
		InData.VideoInfo.avg_frame_rate.num = s->streams[video_index]->avg_frame_rate.num;

		int32 size = s->streams[video_index]->codecpar->extradata_size;
		InData.VideoInfo.extradata.AddZeroed(size);
		//s->streams[video_index]->codecpar->extradata = (uint8_t*)av_mallocz(size + FF_INPUT_BUFFER_PADDING_SIZE);
		if (NULL != s->streams[video_index]->codecpar->extradata)
		{
			memcpy(InData.VideoInfo.extradata.GetData(), s->streams[video_index]->codecpar->extradata, size);
		}
		InData.VideoInfo.r_frame_rate.den = s->streams[video_index]->r_frame_rate.den;
		InData.VideoInfo.r_frame_rate.num = s->streams[video_index]->r_frame_rate.num;
		*/
	}
}