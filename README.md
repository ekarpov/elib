# elib

A general-purpose low memory footprint portable library with useful utilities for cross-platform software development 

## License

This software is released under the [MIT license](https://github.com/nullable/libxmlquery/blob/master/LICENSE).

## Installation

```console
git clone https://github.com/ekarpov/elib.git
cd elib
make install
```

For custom installtion path:
```console
make install INSTALL_DIR=/path/to/install
```

## Features

* Fast and low memory footprint
* Plain C code without any extra dependencies, completely free 
* Replacable standard library for non-standard systems
* Modules
  * [Core](docs/elibc.md) - containers like list and array, sorting, memory buffer management, string manipulations and system functions
  * [Encoders](docs/encoders.md) - efficient json and xml encoders
  * [Hash](docs/hash.md) - hash functions
  * [HTTP](docs/http.md) - HTTP protocol parser and encoder
  * [Parsers](docs/parsers.md) - efficient in-place json, xml, escape and entity parsers
  * [Text](docs/text.md) - text encoding and decoding including utf8, utf 16 and base64 support
  * [Time](docs/time.md) - time format parsers and encoders with Unix time, ISO-8601 and RFC 1123 time format support
  * [WBXML](docs/wbxml.md) - WAP Binary XML decoder and encoder
