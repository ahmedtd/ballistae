#include <iostream>

#include "libballistae/zipstream.hh"

namespace ballistae {

zipreader_error zipreader::open(std::istream *in, std::size_t bufsize) {
  this->in = in;

  this->buffer.resize(bufsize);

  this->stream.zalloc = Z_NULL;
  this->stream.zfree = Z_NULL;
  this->stream.opaque = Z_NULL;
  this->stream.avail_in = 0;
  this->stream.next_in = Z_NULL;

  if (inflateInit(&this->stream) != Z_OK) {
    return zipreader_error::error_decompressing;
  }

  this->in->read(this->buffer.data(), this->buffer.size());
  this->stream.avail_in = this->in->gcount();
  this->stream.next_in = reinterpret_cast<unsigned char *>(this->buffer.data());

  return zipreader_error::ok;
}

void zipreader::close() { inflateEnd(&this->stream); }

zipreader_error zipreader::read(char *buf, std::size_t n) {
  // Pull n bytes from the decompressor, refilling this->buffer from this->in
  // whenever necessary.

  this->stream.avail_out = n;
  this->stream.next_out = reinterpret_cast<unsigned char *>(buf);
  while (true) {
    int flush_mode = this->in->eof() ? Z_FINISH : Z_NO_FLUSH;

    int inflate_status = inflate(&this->stream, flush_mode);
    this->last_read_size = n - this->stream.avail_out;
    if (inflate_status == Z_STREAM_END) {
      // The zip stream has ended.

      // If we still have data in the buffer, we should error.
      if (this->stream.avail_in != 0) {
        this->stream.avail_out = 0;
        this->stream.next_out = Z_NULL;
        return zipreader_error::error_early_eof;
      }

      // If there was more data in the input stream, we should error.
      this->in->read(this->buffer.data(), this->buffer.size());
      this->stream.avail_in = this->in->gcount();
      this->stream.next_in =
          reinterpret_cast<unsigned char *>(this->buffer.data());
      if (!this->in->eof() || this->stream.avail_in != 0) {
        this->stream.avail_out = 0;
        this->stream.next_out = Z_NULL;
        return zipreader_error::error_early_eof;
      }

      this->stream.avail_out = 0;
      this->stream.next_out = Z_NULL;
      return zipreader_error::error_eof;
    }
    if (inflate_status != Z_OK) {
      this->stream.avail_out = 0;
      this->stream.next_out = Z_NULL;
      return zipreader_error::error_decompressing;
    }

    // Feed the decompressor more if possible.  We could get a zero-length read
    // if the underlying stream has already gone EOF.
    //
    // TODO(ahmedtd): Ensure we don't stall out if the underlying stream goes
    // EOF but the zip stream hasn't finished.
    if (this->stream.avail_in == 0) {
      this->in->read(this->buffer.data(), this->buffer.size());
      this->stream.avail_in = this->in->gcount();
      this->stream.next_in =
          reinterpret_cast<unsigned char *>(this->buffer.data());
    }

    if (this->stream.avail_out == 0) {
      this->stream.avail_out = 0;
      this->stream.next_out = Z_NULL;
      return zipreader_error::ok;
    }
  }
}

zipwriter_error zipwriter::open(std::ostream *out, std::size_t bufsize) {
  this->out = out;

  this->buffer.resize(bufsize);

  this->stream.zalloc = Z_NULL;
  this->stream.zfree = Z_NULL;
  this->stream.opaque = Z_NULL;
  if (deflateInit(&this->stream, 0)) {
    return zipwriter_error::error_compressing;
  }

  this->stream.avail_out = this->buffer.size();
  this->stream.next_out =
      reinterpret_cast<unsigned char *>(this->buffer.data());

  return zipwriter_error::ok;
}

zipwriter_error zipwriter::close() {
  // We have no more input bytes, but we need to run the compressor with
  // Z_FINISH until it returns Z_STREAM_END.
  this->stream.avail_in = 0;
  this->stream.next_in = Z_NULL;
  while (true) {
    int deflate_status = deflate(&this->stream, Z_FINISH);
    if (deflate_status == Z_STREAM_ERROR) {
      // TODO(ahmedtd): Preserve section of this->buffer that hasn't been
      // written to this->out?
      deflateEnd(&stream);
      return zipwriter_error::error_compressing;
    }

    // Flush output buffer if possible
    if (this->stream.avail_out != this->buffer.size()) {
      this->out->write(this->buffer.data(),
                       this->buffer.size() - this->stream.avail_out);
      if (!out->good()) {
        deflateEnd(&stream);
        return zipwriter_error::error_output_full;
      }
      this->stream.avail_out = this->buffer.size();
      this->stream.next_out =
          reinterpret_cast<unsigned char *>(this->buffer.data());
    }

    if (deflate_status == Z_STREAM_END) {
      deflateEnd(&stream);
      return zipwriter_error::ok;
    }
  }
}

zipwriter_error zipwriter::write(char *buf, std::size_t n) {
  // Feed n bytes to the compressor, emptying this->buffer whenever there's some
  // output.

  this->stream.avail_in = n;
  this->stream.next_in = reinterpret_cast<unsigned char *>(buf);
  while (this->stream.avail_in > 0) {
    int deflate_status = deflate(&this->stream, Z_NO_FLUSH);
    if (deflate_status == Z_STREAM_ERROR) {
      // TODO(ahmedtd): Preserve section of this->buffer that hasn't been
      // written to this->out?
      this->stream.avail_in = 0;
      this->stream.next_in = Z_NULL;
      return zipwriter_error::error_compressing;
    }

    // Flush output buffer if possible
    if (this->stream.avail_out != this->buffer.size()) {
      this->out->write(this->buffer.data(),
                       this->buffer.size() - this->stream.avail_out);
      if (!out->good()) {
        this->stream.avail_in = 0;
        this->stream.next_in = Z_NULL;
        return zipwriter_error::error_output_full;
      }
      this->stream.avail_out = this->buffer.size();
      this->stream.next_out =
          reinterpret_cast<unsigned char *>(this->buffer.data());
    }
  }

  this->stream.avail_in = 0;
  this->stream.next_in = Z_NULL;
  return zipwriter_error::ok;
}

}  // namespace ballistae
