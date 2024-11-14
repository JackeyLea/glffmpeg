/*
* Main source for the glFFmpeg library
* Copyright ( c ) 2006 Marco Ippolito
*
* This file is part of glFFmpeg.
*/

#include "glffmpeg.h"

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#else
#include <time.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
}

#include <map>
#include <string>
#include <cassert>

static bool s_bInitialized = false;

static const unsigned int s_kunMaxLineSize = 4096;

#ifdef WIN32

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}

	return TRUE;
}

#endif //WIN32

class ffmpegHelper
{
public:

	/**
	 * Default constructor
	 */
	ffmpegHelper() :
		m_pAVIFile(NULL),
		m_videoContext(NULL),
		m_status(0),
		m_width(0),
		m_height(0),
		m_bShutdownRequested(false),
		m_videoBuffer(NULL),
		m_unVideoBufferSize(0),
		m_img_convert_ctx(NULL)
	{
	}

	/**
	 * Default destructor
	 */
	~ffmpegHelper()
	{
		m_bShutdownRequested = true;

		// Close each codec
		if (m_pAVIFile != NULL)
		{
			avcodec_close(m_pAVIFile->codec);
			av_free(m_rgbFrame);
			free(m_videoBuffer);
		}

		if (m_yuvFrame != NULL)
		{
			free(m_yuvFrame->data[0]);
			av_free(m_yuvFrame);
			m_yuvFrame = NULL;
		}

		if (m_videoContext != NULL)
		{
			// Write the trailer, if any
			av_write_trailer(m_videoContext);

			// Free the streams
			for (unsigned int i = 0; i < m_videoContext->nb_streams; i++)
			{
				av_freep(&m_videoContext->streams[i]->codec);
				av_freep(&m_videoContext->streams[i]);
			}

			if ((m_videoFormat->flags & AVFMT_NOFILE))
			{
				// close the output file
				avio_close(m_videoContext->pb);
			}

			// free the stream
			av_free(m_videoContext);
		}

		if (m_img_convert_ctx != NULL)
		{
			sws_freeContext(m_img_convert_ctx);
		}

	}

	/**
	 * Configure a stream for recording purposes. This method will try to set
	 * up a recording stream at the specified filename with the specified frame
	 * rate and the specified dimensions.
	 */
	int configure(const char* fileName, int fpsRate, int width, int height,
		void* imageBuffer)
	{
		m_width = width;
		m_height = height;

		if ((m_width % 160) || (m_height % 160))
		{
			printf("ffmpegHelper::configure: window dimensions are not "
				"divisible by 160, some codecs may not like this!\n");
		}

		// Auto detect the output format from the name. default is  mpeg.
		m_videoFormat = av_guess_format(NULL, fileName, NULL);

		if (!m_videoFormat)
		{
			printf("Could not deduce output format from file extension. Defaulting to MPEG.\n");
			m_videoFormat = av_guess_format("mpeg", NULL, NULL);
		}

		if (!m_videoFormat)
		{
			printf("ffmpegHelper::configure: Unable to locate a suitable encoding format.\n");
			m_status = 1;
			return m_status;
		}

		// Allocate the output media context.
		m_videoContext = avformat_alloc_context();

		if (!m_videoContext)
		{
			printf("ffmpegHelper::configure: Error allocating video context.\n");
			m_status = 2;
			return m_status;
		}

		m_videoContext->oformat = (AVOutputFormat*)m_videoFormat;

#ifdef _WIN32
		//_snprintf_s(m_videoContext->filename, sizeof(m_videoContext->filename), "%s", fileName);
#else
		snprintf(m_videoContext->filename, sizeof(m_videoContext->filename), "%s", fileName);
#endif

		// video stream using the default format codec and initialize the codec
		m_pAVIFile = NULL;

		if (m_videoFormat->video_codec != AV_CODEC_ID_NONE)
		{
			m_pAVIFile = avformat_new_stream(m_videoContext, 0);

			if (!m_pAVIFile)
			{
				printf("ffmpegHelper::configure: Error allocating video stream.\n");
				m_status = 3;
				return m_status;
			}

			m_pAVIFile->time_base.den = 1;
			m_pAVIFile->time_base.num = 1;

			m_codecContext = m_pAVIFile->codec;

			m_codecParams = m_pAVIFile->codecpar;
			m_codecParams->codec_id = m_videoFormat->video_codec;
			m_codecParams->codec_type = AVMEDIA_TYPE_VIDEO;
			m_codecParams->bit_rate = 7500000;
			m_codecParams->width = m_width;
			m_codecParams->height = m_height;
		}

		// Print encoding format information to the console
		av_dump_format(m_videoContext, 0, fileName, 1);

		if (m_pAVIFile)
		{
			// Find the video encoder
			const AVCodec* codec = avcodec_find_encoder(m_codecParams->codec_id);

			if (!codec)
			{
				printf("ffmpegHelper::configure: Codec not found.\n");
				m_status = 5;
				return m_status;
			}


			avcodec_get_context_defaults3(m_codecContext, codec);

			m_codecContext->time_base.den = fpsRate;
			m_codecContext->time_base.num = 1;
			m_codecContext->gop_size = 12;
			m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
			m_codecContext->width = m_width;
			m_codecContext->height = m_height;

			// If Mpeg1 then set macro block decision mode to rate distortion mode
			if (m_codecParams->codec_id == AV_CODEC_ID_MPEG1VIDEO)
				m_codecContext->mb_decision = FF_MB_DECISION_RD;

			// If the user has requested a raw video format then
			// set the pixel format to AV_PIX_FMT_YUV422P
			if (m_codecParams->codec_id == AV_CODEC_ID_RAWVIDEO)
				m_codecContext->pix_fmt = AV_PIX_FMT_YUV422P;

			// Some formats want stream headers to be separate
			if (!strcmp(m_videoContext->oformat->name, "mp4") ||
				!strcmp(m_videoContext->oformat->name, "mov") ||
				!strcmp(m_videoContext->oformat->name, "3gp"))
			{
				//m_codecContext->flags |= CODEC_FLAG_GLOBAL_HEADER;
			}

			// Open the codec
			if (avcodec_open2(m_codecContext, codec, NULL) < 0)
			{
				printf("ffmpegHelper::configure: Could not open codec.\n");
				m_status = 6;
				return m_status;
			}

			m_videoBuffer = NULL;

			// If it is a raw format then manually allocate enough
			// memory to hold the raw video buffer
			/*if (!(m_videoContext->oformat->flags & AVFMT_RAWPICTURE))
			{
				// Allocate output buffer
				m_unVideoBufferSize = m_width * m_height * 30;
				m_videoBuffer = (uint8_t*)malloc(m_unVideoBufferSize);
			}*/

			// Allocate the video frames
			uint8_t* picture_buf;
			int size;

			m_yuvFrame = av_frame_alloc();

			if (!m_yuvFrame)
			{
				printf("ffmpegHelper::configure: Error allocating video frame.\n");
				m_status = 7;
				return m_status;
			}

			m_yuvFrame->format = m_codecContext->pix_fmt;
			m_yuvFrame->width = m_width;
			m_yuvFrame->height = m_height;

			size = av_image_get_buffer_size(m_codecContext->pix_fmt, m_width, m_height, 1);
			picture_buf = (uint8_t*)malloc(size);

			if (!picture_buf)
			{
				printf("ffmpegHelper::configure: Error allocating video frame.\n");
				m_status = 8;
				return m_status;
			}

			av_image_fill_arrays(m_yuvFrame->data, m_yuvFrame->linesize,
				picture_buf,
				m_codecContext->pix_fmt, m_width, m_height, 1);

			m_rgbFrame = av_frame_alloc();

			if (!m_rgbFrame)
			{
				printf("ffmpegHelper::configure: Error allocating video frame.\n");
				m_status = 9;
				return m_status;
			}

			m_rgbFrame->width = m_width;
			m_rgbFrame->height = m_height;

			size = av_image_get_buffer_size(AV_PIX_FMT_RGB24, m_rgbFrame->width, m_rgbFrame->height, 1);

			av_image_fill_arrays(m_rgbFrame->data, m_rgbFrame->linesize,
				(uint8_t*)imageBuffer,
				AV_PIX_FMT_RGB24, m_width, m_height, 1);
		}

		// Open the output file, if needed.
		if (!(m_videoFormat->flags & AVFMT_NOFILE))
		{
			if (avio_open(&m_videoContext->pb, fileName, AVIO_FLAG_WRITE) < 0)
			{
				printf("ffmpegHelper::configure: Unable to open output file <%s>.\n", fileName);
				m_status = 10;
				return m_status;
			}
		}

		// Write the stream header, if any
		avformat_write_header(m_videoContext, NULL);


		m_status = 0;
		return m_status;
	}

	/**
	 * Retrieves the current status of the AVI library.
	 *
	 * Possible status error codes for return value
	 *
	 * 0 Success - status normal.
	 * 1 ffmpegHelper::configure: Unable to locate a suitable encoding format.
	 * 2 ffmpegHelper::configure: Error allocating video context.
	 * 3 ffmpegHelper::configure: Error allocating video stream.
	 * 4 ffmpegHelper::configure: Invalid output parameters.
	 * 5 ffmpegHelper::configure: Codec not found.
	 * 6 ffmpegHelper::configure: Could not open codec.
	 * 7 ffmpegHelper::configure: Error allocating video frame.
	 * 8 ffmpegHelper::configure: Unable to open output file <file_name>,
	 * 9 ffmpegHelper::encodeFrame: You need to create a stream first.
	 */
	inline int getStatus(void) const { return m_status; }

	/**
	 * Captures a single frame of video to the stream
	 */
	int encodeFrame()
	{
		if (m_bShutdownRequested)
		{
			m_status = 0;
			return m_status;
		}

		if (!m_pAVIFile)
		{
			printf("ffmpegHelper::encodeFrame: You need to create a stream first.\n");
			m_status = 11;
			return m_status;
		}

		// convert RGB to YUV
		if (m_img_convert_ctx == NULL)
		{
			m_img_convert_ctx = sws_getContext(m_width, m_height, AV_PIX_FMT_RGB24, m_width, m_height,
				m_codecContext->pix_fmt, SWS_BICUBIC, NULL, NULL, NULL);
		}

		sws_scale(m_img_convert_ctx, m_rgbFrame->data, m_rgbFrame->linesize, 0,
			m_rgbFrame->height, m_yuvFrame->data, m_yuvFrame->linesize);


		// flip the YUV frame upside down
		unsigned char* s;
		unsigned char* d;
		static unsigned char  b[s_kunMaxLineSize];

		assert(m_yuvFrame->linesize[0] <= s_kunMaxLineSize);

		for (s = m_yuvFrame->data[0], d = m_yuvFrame->data[1] - m_yuvFrame->linesize[0];
			s < d; s += m_yuvFrame->linesize[0], d -= m_yuvFrame->linesize[0])
		{
			memcpy(b, s, m_yuvFrame->linesize[0]);
			memcpy(s, d, m_yuvFrame->linesize[0]);
			memcpy(d, b, m_yuvFrame->linesize[0]);
		}

		assert(m_yuvFrame->linesize[1] <= s_kunMaxLineSize);

		for (s = m_yuvFrame->data[1], d = m_yuvFrame->data[2] - m_yuvFrame->linesize[2];
			s < d; s += m_yuvFrame->linesize[1], d -= m_yuvFrame->linesize[1])
		{
			memcpy(b, s, m_yuvFrame->linesize[1]);
			memcpy(s, d, m_yuvFrame->linesize[1]);
			memcpy(d, b, m_yuvFrame->linesize[1]);
		}

		assert(m_yuvFrame->linesize[2] <= s_kunMaxLineSize);

		for (s = m_yuvFrame->data[2], d = m_yuvFrame->data[2] +
			(m_yuvFrame->data[2] - m_yuvFrame->data[1] - m_yuvFrame->linesize[2]);
			s < d; s += m_yuvFrame->linesize[2], d -= m_yuvFrame->linesize[2])
		{
			memcpy(b, s, m_yuvFrame->linesize[2]);
			memcpy(s, d, m_yuvFrame->linesize[2]);
			memcpy(d, b, m_yuvFrame->linesize[2]);
		}

		// Encode the YUV frame.

		int got_packet;
		AVPacket* pkt = av_packet_alloc();
		pkt->data = m_videoBuffer;
		pkt->size = m_unVideoBufferSize;

		if (avcodec_send_frame(m_codecContext, m_yuvFrame) >= 0) {
			while (avcodec_receive_packet(m_codecContext, pkt) >= 0) {
				if (pkt->pts != AV_NOPTS_VALUE)
					pkt->pts = av_rescale_q(pkt->pts, m_pAVIFile->time_base, m_pAVIFile->time_base);
				if (pkt->dts != AV_NOPTS_VALUE)
					pkt->dts = av_rescale_q(pkt->dts, m_pAVIFile->time_base, m_pAVIFile->time_base);

				// Write the compressed frame in the media file.
				av_write_frame(m_videoContext, pkt);

				av_packet_unref(pkt);
			}
		}

		m_status = 0;
		return m_status;
	}

private:

	int m_status;
	int m_width;
	int m_height;
	bool m_bShutdownRequested;
	AVStream* m_pAVIFile;
	AVFormatContext* m_videoContext;
	const AVOutputFormat* m_videoFormat;
	AVCodecContext* m_codecContext;
	AVCodecParameters* m_codecParams;
	AVFrame* m_rgbFrame;
	AVFrame* m_yuvFrame;
	uint8_t* m_videoBuffer;
	uint32_t m_unVideoBufferSize;
	struct SwsContext* m_img_convert_ctx;
};

typedef std::map<std::string, ffmpegHelper* > ffmpegHelpers;
static ffmpegHelpers s_ffmpegHelpers;

#ifdef __cplusplus
extern "C"
{
#endif

	int __cdecl initializeGLFFMPEG()
	{
		s_bInitialized = true;

		return 1;
	}

	int __cdecl shutdownGLFFMPEG()
	{
		// Iterate through ffmpegHelpers and delete them
		ffmpegHelpers::iterator it = s_ffmpegHelpers.begin();
		ffmpegHelpers::iterator ite = s_ffmpegHelpers.end();

		for (; it != ite; ++it)
			delete it->second;

		s_ffmpegHelpers.clear();
		s_bInitialized = false;

		return 1;
	}

	int __cdecl initializeStream(const char* streamName, int fpsRate,
		int width, int height, void* imageBuffer)
	{
		if (streamName == NULL || fpsRate == 0 ||
			imageBuffer == NULL || width == 0 || height == 0)
		{
			printf("initializeStream : invalid parameter provided\n");
			return 1;
		}

		ffmpegHelper* helper = new ffmpegHelper();

		int ret = helper->configure(streamName, fpsRate, width, height, imageBuffer);

		if (ret != 0)
			return ret;

		s_ffmpegHelpers.insert(ffmpegHelpers::value_type(streamName, helper));

		return 0;
	}

	int __cdecl encodeFrame(const char* streamName)
	{
		// Find the appropriate ffmpegHelper
		ffmpegHelpers::iterator it = s_ffmpegHelpers.find(streamName);

		if (it == s_ffmpegHelpers.end())
			return 1;

		// Call encodeFrame
		it->second->encodeFrame();

		return 0;
	}

	int __cdecl shutdownStream(const char* streamName)
	{
		// Find the appropriate ffmpegHelper
		ffmpegHelpers::iterator it = s_ffmpegHelpers.find(streamName);

		if (it == s_ffmpegHelpers.end())
			return 1;

		// Call its destructor
		delete it->second;

		// Remove it from the ffmpegHelpers list
		s_ffmpegHelpers.erase(it);

		return 0;
	}

	int __cdecl getStatus(const char* streamName)
	{
		// Find the appropriate ffmpegHelper
		ffmpegHelpers::iterator it = s_ffmpegHelpers.find(streamName);

		if (it == s_ffmpegHelpers.end())
			return 1;

		return (*it).second->getStatus();
	}

#ifdef __cplusplus
}
#endif
