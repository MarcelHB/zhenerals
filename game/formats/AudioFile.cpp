extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
}

#include "../common.h"
#include "../Logging.h"
#include "AudioFile.h"

constexpr size_t FORMAT_BUFFER_SIZE = 8192;

namespace ZH {

AudioFile::AudioFile(std::istream& stream) : stream(stream) {}

static int readFunction(void* opaque, uint8_t* buf, int bufSize) {
  auto& stream = *reinterpret_cast<std::istream*>(opaque);
  if (stream.eof()) {
    return AVERROR_EOF;
  }
  stream.read(reinterpret_cast<char*>(buf), bufSize);

  auto num = stream.gcount();
  return num > 0 ? num : AVERROR_EOF;
};

std::shared_ptr<Audio::SoundData> AudioFile::parseSoundData() {
  TRACY(ZoneScoped);

  auto buffer = reinterpret_cast<unsigned char*>(av_malloc(FORMAT_BUFFER_SIZE));
  std::shared_ptr<AVIOContext> avioContext(
      avio_alloc_context(
          buffer
        , FORMAT_BUFFER_SIZE
        , 0
        , reinterpret_cast<void*>(static_cast<std::istream*>(&stream))
        , &readFunction
        , nullptr
        , nullptr
      )
    , [](auto p) {
        av_free(p->buffer);
        av_free(p);
      }
  );

  std::shared_ptr<AVFormatContext> avFormat {avformat_alloc_context(), &avformat_free_context};
  auto avFormatPtr = avFormat.get();
  avFormat->pb = avioContext.get();
  avformat_open_input(&avFormatPtr, "none", nullptr, nullptr);

  if (avFormatPtr->streams == nullptr) {
    WARN_ZH("AudioFile", "File does not look like media");
    return {};
  }

  auto streamFormat = avFormatPtr->streams[0]->codecpar;
  if (streamFormat->codec_type != AVMEDIA_TYPE_AUDIO) {
    WARN_ZH("AudioFile", "File does not look like audio");
    return {};
  }

  Audio::SoundFormat format;
  auto codec = avcodec_find_decoder(streamFormat->codec_id);
  if (codec == nullptr) {
    WARN_ZH("AudioFile", "File format is not supported");
    return {};
  }

  std::shared_ptr<AVCodecContext> codecContext {
      avcodec_alloc_context3(codec)
    , [](auto p) { avcodec_free_context(&p); }
  };
  if (codecContext == nullptr) {
    WARN_ZH("AudioFile", "Could not initialize codec");
    return {};
  }

  avcodec_parameters_to_context(codecContext.get(), avFormatPtr->streams[0]->codecpar);
  codecContext->request_sample_fmt = AV_SAMPLE_FMT_S16;

  if (avcodec_open2(codecContext.get(), codec, nullptr) < 0) {
    WARN_ZH("AudioFile", "Could not initialize codec");
    return {};
  }

  bool planar = av_sample_fmt_is_planar(codecContext->sample_fmt);

  AVPacket *avPacket = av_packet_alloc();

  std::vector<char> fullBuffer;
  fullBuffer.reserve(4096);

  std::vector<char> fullBuffer2;
  if (planar) {
    fullBuffer2.reserve(4096);
  }

  std::shared_ptr<AVFrame> frame {
      av_frame_alloc()
    , [](auto p) { av_frame_free(&p); }
  };

  uint16_t i = 0;
  size_t totalSamples = 0;
  uint8_t sampleSize = 1;

  SwrContext *swr = nullptr;
  bool useSwr = false;

  while (av_read_frame(avFormatPtr, avPacket) >= 0) {
    auto ret = avcodec_send_packet(codecContext.get(), avPacket);

    while (avcodec_receive_frame(codecContext.get(), frame.get()) == 0) {
      if (i == 0) {
        auto frameFormat = static_cast<AVSampleFormat>(frame->format);
        sampleSize = av_samples_get_buffer_size(nullptr, 1, 1, frameFormat, 1);
        format.numChannels = streamFormat->ch_layout.nb_channels;
        format.numSamplesPerSec = streamFormat->sample_rate;
        format.bits = sampleSize * 8;

        if (frameFormat == AV_SAMPLE_FMT_FLT
            || frameFormat == AV_SAMPLE_FMT_DBL
            || frameFormat == AV_SAMPLE_FMT_FLTP
            || frameFormat == AV_SAMPLE_FMT_DBLP) {
          format.bits = 16;
          sampleSize = 2;

          useSwr = true;
          swr_alloc_set_opts2(
              &swr
            , &streamFormat->ch_layout
            , AV_SAMPLE_FMT_S16P
            , format.numSamplesPerSec
            , &streamFormat->ch_layout
            , frameFormat
            , format.numSamplesPerSec
            , 0
            , nullptr
          );
          swr_init(swr);
        }
      }
      totalSamples += frame->nb_samples;

      // linesize[0] gives zero-padded, aligned size, nb_samples * sampleSize actual size
      auto frameSize = frame->nb_samples * sampleSize;
      auto lastSize = fullBuffer.size();
      fullBuffer.resize(fullBuffer.size() + frameSize);

      if (planar) {
        fullBuffer2.resize(fullBuffer2.size() + frameSize);
      }

      if (useSwr) {
        uint8_t* data[] = {
            reinterpret_cast<uint8_t*>(fullBuffer.data() + lastSize)
          , reinterpret_cast<uint8_t*>(fullBuffer2.data() + lastSize)
        };
        swr_convert(
            swr
          , reinterpret_cast<uint8_t**>(&data)
          , frame->nb_samples
          , frame->extended_data
          , frame->nb_samples
        );
      } else {
        std::copy(
            frame->extended_data[0]
          , frame->extended_data[0] + frameSize
          , fullBuffer.begin() + lastSize
        );
      }

      if (planar && !useSwr) {
        std::copy(
            frame->extended_data[1]
          , frame->extended_data[1] + frameSize
          , fullBuffer2.begin() + lastSize
        );
      }

      av_frame_unref(frame.get());
      ++i;
    }

    av_packet_unref(avPacket);
  }

  av_packet_free(&avPacket);
  if (useSwr) {
    swr_free(&swr);
  }

  if (planar) {
    std::vector<char> mergeBuffer(fullBuffer.size() * 2);

    for (size_t i = 0; i < totalSamples; ++i) {
      std::copy(
          fullBuffer.cbegin() + i * sampleSize
        , fullBuffer.cbegin() + i * sampleSize + sampleSize
        , mergeBuffer.begin() + i * 2 * sampleSize
      );
      std::copy(
          fullBuffer2.cbegin() + i * sampleSize
        , fullBuffer2.cbegin() + i * sampleSize + sampleSize
        , mergeBuffer.begin() + i * 2 * sampleSize + sampleSize
      );
    }

    return std::make_shared<Audio::SoundData>(std::move(format), std::move(mergeBuffer));
  } else {
    return std::make_shared<Audio::SoundData>(std::move(format), std::move(fullBuffer));
  }
}

}
