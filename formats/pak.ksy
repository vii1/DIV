meta:
  id: pak
  file-extension: pak
  encoding: ascii #cp850
  endian: le

types:
  header:
    seq:
      - id: magic
        contents: ["dat", 0x1A, 0x0D, 0x0A, 0]
      - id: version
        type: u1
  file:
    seq:
      - id: filename
        type: strz
        size: 16
      - id: offset
        type: u4
      - id: compressed
        type: u4
      - id: uncompressed
        type: u4
    instances:
      file:
        pos: offset
        size: compressed
        type:
          switch-on: compressed < uncompressed
          cases:
            true: compressed_file(compressed)
            false: raw_file(compressed)

  raw_file:
    params:
      - id: size
        type: u4
    seq:
      - id: contents
        size: size
        
  compressed_file:
    params:
      - id: size
        type: u4
    seq:
      - id: contents
        size: size
        process: zlib
        
seq:
  - id: header
    type: header
  - id: crc
    type: u4
    repeat: expr
    repeat-expr: 3
  - id: num_files
    type: u4
  - id: files
    type: file
    repeat: expr
    repeat-expr: num_files
