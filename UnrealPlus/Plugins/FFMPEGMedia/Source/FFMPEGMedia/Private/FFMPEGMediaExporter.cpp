// Fill out your copyright notice in the Description page of Project Settings.

#include "FFMPEGMediaExporter.h"
#include "FFMPEGMediaPrivate.h"
#include "LambdaFunctionRunnable.h"

extern  "C" {
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
#include "libavutil/time.h"
}

void UFFMPEGMediaExporter::Export(const FString& Url, const FString& OutputFile, int max_seconds){
    LambdaFunctionRunnable::RunThreaded("ExportThread", [Url, OutputFile, max_seconds]() {
        const char* fin=TCHAR_TO_ANSI(*Url);
        const char* fout=TCHAR_TO_ANSI(*OutputFile);
        
        AVDictionary *format_opts = NULL;
        AVFormatContext* input_fmtctx = avformat_alloc_context();
        if (!av_dict_get(format_opts, "scan_all_pmts", NULL, AV_DICT_MATCH_CASE)) {
            av_dict_set(&format_opts, "scan_all_pmts", "1", AV_DICT_DONT_OVERWRITE);
        }
        
        // open input stream
        int err = 0;
        if (Url.StartsWith(TEXT("file://")) && Url.Len() > 7){
            const TCHAR* FilePath = &Url[7];
            err = avformat_open_input(&input_fmtctx, fin, NULL, &format_opts);
        }else {
            if (Url.StartsWith(TEXT("rtsp://"))){
                av_dict_set(&format_opts, "rtsp_transport", "tcp", 0);
                av_dict_set(&format_opts, "max_delay", "100", 0);
                av_dict_set(&format_opts, "flush_packets", "1", 0);
                av_dict_set_int(&format_opts, "stimeout", (int64_t)10000000, 0);
                
                //FormatContext->probesize = 100 * 1024;
                input_fmtctx->max_analyze_duration = 2 * AV_TIME_BASE;
            }
            
            err = avformat_open_input(&input_fmtctx, fin, NULL, &format_opts);
        }
        if(format_opts)
            av_dict_free(&format_opts);
        
        if (err < 0) {
            UE_LOG(LogFFMPEGMedia, Error, TEXT("Failed to open input stream %s"), *Url);
            return;
        }
        // export
        while(true){
            if (avformat_find_stream_info(input_fmtctx,0) < 0) {
                UE_LOG(LogFFMPEGMedia, Error, TEXT("Failed to retrieve input stream information %s"), *Url);
                break;
            }
            //av_dump_format(input_fmtctx, 0, fin, 0);
            
            // alloc output
            AVFormatContext *output_fmtctx = NULL;
            if (avformat_alloc_output_context2(&output_fmtctx, NULL, NULL, fout) < 0) {
                UE_LOG(LogFFMPEGMedia, Error, TEXT("Cannot open output stream %s"), *OutputFile);
                break;
            }
            //read & write
            while(true){
                bool err_stream=false;
                for (int i = 0; i < input_fmtctx->nb_streams; i++) {
                    AVStream *in_stream = input_fmtctx->streams[i];
                    if(in_stream->codecpar->codec_id==AV_CODEC_ID_NONE)
                        continue;
                    AVStream *out_stream = avformat_new_stream(output_fmtctx, NULL);
                    if (!out_stream) {
                        UE_LOG(LogFFMPEGMedia, Warning, TEXT("Alloc new Stream error"));
                        err_stream=true;
                        break;
                    }
                    
                    avcodec_parameters_copy(out_stream->codecpar, in_stream->codecpar);
                    out_stream->codecpar->codec_tag = 0;
                }
                if(err_stream)
                    break;
                //av_dump_format(output_fmtctx, 0, fout, 1);
                
                // open output
                if (avio_open(&output_fmtctx->pb, fout, AVIO_FLAG_WRITE) < 0) {
                    UE_LOG(LogFFMPEGMedia, Error, TEXT("Cannot open the output file %s"), *OutputFile);
                    break;
                }
                
                while(true){
                    // write
                    if (avformat_write_header(output_fmtctx, NULL) < 0) {
                        UE_LOG(LogFFMPEGMedia, Error, TEXT("Cannot write the header"));
                        break;
                    }

                    AVPacket pkt;
                    av_init_packet(&pkt);
                    pkt.data = NULL;
                    pkt.size = 0;
                    
                    while (true) {
                        AVStream *in_stream = NULL;
                        AVStream *out_stream = NULL;
                        
                        if (av_read_frame(input_fmtctx, &pkt) < 0) {
                            UE_LOG(LogFFMPEGMedia, Warning, TEXT("Read frame error"));
                            break;
                        }
                        
                        in_stream = input_fmtctx->streams[pkt.stream_index];
                        out_stream = output_fmtctx->streams[pkt.stream_index];
                        
                        if (max_seconds>0 && av_q2d(in_stream->time_base) * pkt.pts > max_seconds) {
                            break;
                        }
                        
                        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, AVRounding(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                        if (pkt.pts < 0) {
                            pkt.pts = 0;
                        }
                        if (pkt.dts < 0) {
                            pkt.dts = 0;
                        }
                        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
                        pkt.pos = -1;
                        
                        if (av_write_frame(output_fmtctx, &pkt) < 0) {
                            UE_LOG(LogFFMPEGMedia, Warning, TEXT("Muxing Error"));
                            break;
                        }
                    }
                    av_packet_unref(&pkt);
                    av_write_trailer(output_fmtctx);
                    UE_LOG(LogFFMPEGMedia, Log, TEXT("Succeeded export %s to %s"), *Url, *OutputFile);
                    break;
                }
                avio_close(output_fmtctx->pb);
                break;
            }
            avformat_free_context(output_fmtctx);
            break;
        }
        avformat_close_input(&input_fmtctx);
    });
}
