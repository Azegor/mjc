#ifndef __COLOR_OSTREAM_HPP__
#define __COLOR_OSTREAM_HPP__

#include <ostream>
#include <sstream>
#include <string>

namespace co {

/** enums for color/display manipulation */
enum modes {
  normal = 0,
  bold = 1,
  faint = 2,
  italic = 3,
  underline = 4,
  blink_s = 5,
  blink_f = 6,
  negative = 7,
  conceal = 8,
  crossed_out = 9
  // ...
};

enum colors {
  regular = 39,
  black = 30,
  red = 31,
  green = 32,
  yellow = 33,
  blue = 34,
  purple = 35,
  cyan = 36,
  white = 37
};

template <typename OT> class color_ostream;

/**
 * color_output: class that wrapps a string in ANSI color codes
 * at the end of the string the color code is reset to default
 */
class color_output {
  int color, mode;
  std::string color_init;
  static constexpr const char *color_end = "\033[00m";
  std::stringstream buffer;

public:
  color_output(int color, int mode) : color(color), mode(mode), color_init() {
    update_c_init();
  }

  // user defined copy constructor since stringstream is non-copyable
  color_output(const color_output &o)
      : color(o.color), mode(o.mode), color_init(o.color_init) {
    // buffer.str(o.buffer.str()); // to be safe (not necessary)
  }

  void update_c_init() {
    color_init =
        "\033[" + std::to_string(mode) + ";" + std::to_string(color) + "m";
  }

  template <typename T> std::string operator()(T t) {
    buffer.str("");
    buffer << color_init << t << color_end;
    return buffer.str();
  }

  void set(int c, int m) {
    color = c;
    mode = m;
    update_c_init();
  }

  void set_color(int c) {
    color = c;
    update_c_init();
  }

  void set_mode(int m) {
    mode = m;
    update_c_init();
  }
};

// decorators for color_ostream
class color {
  int clr;

public:
  explicit color(int c) : clr(c) {}

private:
  template <typename OT> friend class color_ostream;
  // friend color_ostream<OT> &color_ostream<OT>::operator<<(const color &);
};

class mode {
  int mde;

public:
  explicit mode(int m) : mde(m) {}

private:
  template <typename OT> friend class color_ostream;
  // friend color_ostream<OT> &color_ostream<OT>::operator<<(const mode &);
};

class enable {
  bool enbl;

public:
  explicit enable(int e) : enbl(e) {}

private:
  template <typename OT> friend class color_ostream;
  // friend color_ostream<OT> &color_ostream<OT>::operator<<(const enable &);
};

// broken: ???
template <typename OT> color_ostream<OT> &reset(color_ostream<OT> &os) {
  os.reset();
  return os;
}

/**
 * color_ostream: class that wrapps a std::ostream and colorizes any output that
 *is put into it via operator<<
 * it doesn' support all ostream operations (yet?) but can easily be extended to
 *do so
 *
 */
template <typename OT = std::ostream> class color_ostream {
  using ostream_t = OT;
  using color_ostream_t = color_ostream<OT>;
  ostream_t &out;
  color_output color_out;
  bool enabled;

  // friend color_ostream &mode(color_ostream &, int);
  // friend color_ostream &color(color_ostream &, int);
public:
  color_ostream(ostream_t &o, int color = regular, int mode = normal,
                bool enbld = true)
      : out(o), color_out(color, mode), enabled(enbld) {}

  // color_ostream(const color_ostream &) = default;

  template <typename T> color_ostream_t &operator<<(const T &t) {
    if (enabled)
      out << color_out(t);
    else
      out << t;
    return *this;
  }

  color_ostream_t &operator<<(const mode &m) {
    set_mode(m.mde);
    return *this;
  }

  color_ostream_t &operator<<(const color &c) {
    set_color(c.clr);
    return *this;
  }

  color_ostream_t &operator<<(const enable &e) {
    enabled = e.enbl;
    return *this;
  }

  // to handle functors like "endl"
  // leave with std::ostream for the time being (should be right anyway)
  color_ostream_t &operator<<(std::ostream &(*pf)(std::ostream &)) {
    pf(out);
    return *this;
  }
  // and own manipulators
  color_ostream_t &operator<<(color_ostream_t &(*pf)(color_ostream_t &)) {
    pf(*this);
    return *this;
  }

  inline void set(int cval, int mval) { color_out.set(cval, mval); }
  inline void set_color(int cval) { color_out.set_color(cval); }
  inline void set_mode(int mval) { color_out.set_mode(mval); }
  inline void reset() { color_out.set(regular, normal); }
};

template <typename OT>
color_ostream<OT> make_colored(OT &&ot, int color = regular, int mode = normal,
                               bool enbld = true) {
  return {std::forward<OT>(ot), color, mode, enbld};
}
}

#endif // __COLOR_OSTREAM_HPP__
