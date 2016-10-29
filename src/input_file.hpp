/*
 * MIT License
 *
 * Copyright (c) 2016 morrisfeist
 * Copyright (c) 2016 tpriesner
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef INPUT_FILE_H
#define INPUT_FILE_H

#include <fstream>
#include <istream>
#include <memory>

#include "error.hpp"

class InputFile {
public:
  InputFile(std::string file, std::istream *is = nullptr)
      : fileName(std::move(file)), istream(is), ownsStream(false) {}

  InputFile(const InputFile &o) = delete;
  InputFile(InputFile &&o) {
    fileName = std::move(o.fileName);
    istream = o.istream;
    ownsStream = o.ownsStream;

    o.istream = nullptr;
    o.ownsStream = false;
  };

  ~InputFile() {
    if (ownsStream) {
      delete istream;
    }
  }

  const std::string &getFilename() const { return fileName; }

  std::istream *getStream() const {
    if (!istream) {
      ownsStream = true;
      auto inputFileStream =
          std::make_unique<std::ifstream>(fileName, std::ios::binary);
      if (!inputFileStream->is_open()) {
        throw ArgumentError("Cannot open File '" + fileName + "'");
      }
      istream = inputFileStream.release();
    }
    return istream;
  }

private:
  std::string fileName;
  mutable std::istream *istream;
  mutable bool ownsStream = false;
};

#endif // INPUT_FILE_H
