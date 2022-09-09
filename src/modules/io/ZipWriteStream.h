/**
 * @file
 */

#pragma once

#include "Stream.h"

struct mz_stream_s;

namespace io {

/**
 * @see BufferedZipReadStream
 * @see ZipReadStream
 * @see WriteStream
 * @ingroup IO
 */
class ZipWriteStream : public io::WriteStream {
private:
	struct mz_stream_s *_stream;
	io::WriteStream &_outStream;
	uint8_t _out[256 * 1024];
	int64_t _pos = 0;

public:
	/**
	 * @param outStream The buffer that receives the writes for the compressed data.
	 * @param level The compression level (0 is no compression, 1 is the best speed, 9 is the best compression).
	 */
	ZipWriteStream(io::WriteStream &outStream, int level = 6);
	virtual ~ZipWriteStream();

	/**
	 * @param buf The buffer to add to the output stream.
	 * @param size The size of the buffer.
	 * @return @c -1 on error - otherwise the amount of bytes that were written to the output stream.
	 * The amount of bytes written are usually less than the given input buffer size, as the bytes
	 * that are written to the output buffer are compressed already.
	 * @note The write() call doesn't flush pending writes into the output buffer.
	 * @see flush()
	 */
	int write(const void *buf, size_t size) override;
	/**
	 * @brief Returns the compressed written bytes that went into the given output stream
	 */
	int64_t pos() const;
	/**
	 * @brief Returns the compressed written bytes that went into the given output stream
	 */
	int64_t size() const;

	/**
	 * @brief Flush the pending stream data into the output stream
	 *
	 * @note This method is automatically called in the destructor
	 */
	bool flush() override;
};

inline int64_t ZipWriteStream::pos() const {
	return _pos;
}

inline int64_t ZipWriteStream::size() const {
	return _pos;
}

} // namespace io
