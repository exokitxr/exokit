#include <exout>

#ifdef LUMIN

std::streamsize ExoutStreambuf::xsputn(const char_type *s, std::streamsize size) override  {
  memcpy(buf + i, s, size);

  flush(size);

  return size;
}

int_type ExoutStreambuf::overflow(int_type c) override {
  buf[i] = c;

  flush(1);

  return c;
}

void ExoutStreambuf::flush(std::streamsize size) {
  for (ssize_t j = i; j < i + size; j++) {
    if (buf[j] == '\n') {
      buf[j] = 0;
      ML_LOG(Info, "%s", buf + lineStart);

      lineStart = j + 1;
    }
  }

  i += size;

  if (i >= STDIO_BUF_SIZE) {
    ssize_t lineLength = i - lineStart;
    memcpy(buf, buf + lineStart, lineLength);
    i = lineLength;
    lineStart = 0;
  }
}

ExoutStreambuf exstream;
std::ostream exout(&exstream);
std::ostream &exerr = exout;

#endif
