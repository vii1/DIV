meta:
  id: map
  file-extension: map
  encoding: ascii #cp850
  endian: le

enums:
  boolean:
    0: false
    1: true

types:
  header:
    seq:
      - id: magic
        contents: ["map",0x1A,0x0D,0x0A,0]
      - id: version
        type: u1
      - id: width
        type: u2
      - id: height
        type: u2
      - id: code
        type: u4
      - id: description
        type: str
        size: 32
  palette:
    enums:
      range_type:
        0: direct
        1: edit1
        2: edit2
        4: edit4
        8: edit8

    types:
      rgb:
        seq:
          - id: r
            type: u1
          - id: g
            type: u1
          - id: b
            type: u1
      range:
        seq:
          - id: n_colors
            type: u1
          - id: type
            type: u1
            enum: range_type
          - id: fixed
            type: u1
            enum: boolean
          - id: black
            type: u1
          - id: colors
            type: u1
            repeat: expr
            repeat-expr: 32
    seq:
      - id: colors
        type: rgb
        repeat: expr
        repeat-expr: 256
      - id: ranges
        type: range
        repeat: expr
        repeat-expr: 16
  cpoint:
    seq:
      - id: x
        type: u2
      - id: y
        type: u2
    
seq:
  - id: header
    type: header
  - id: palette
    type: palette
  - id: n_cpoints
    type: u2
  - id: cpoints
    type: cpoint
    repeat: expr
    repeat-expr: n_cpoints
  - id: pixels
    size: header.width * header.height
  