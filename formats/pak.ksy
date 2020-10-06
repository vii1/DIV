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
        type: s4
      - id: compressed
        type: s4
      - id: uncompressed
        type: s4

seq:
  - id: header
    type: header
  - id: id
    type: u4
    repeat: expr
    repeat-expr: 3
  - id: num_files
    type: u4
  - id: files
    type: file

