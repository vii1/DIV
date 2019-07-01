meta:
  id: ifs
  file-extension: ifs
  endian: le

types:
  header:
    seq:
      - id: magic
        contents: ["IFS",0]
      - id: offset8
        type: u4
      - id: offset10
        type: u4
      - id: offset12
        type: u4
      - id: offset14
        type: u4
      - id: offset128
        type: u4
  tabla_ifs:
    seq:
      - id: desp
        type: u4
      - id: size
        type: u4
    instances:
      letra:
        pos: desp
        size: size
        type: letra
  letra:
    seq:
      - id: alto
        type: u2
      - id: inc_y
        type: u2
      - id: ancho
        type: u2
      - id: pixels
        type: u2
      - id: buffer
        size: alto * ancho
seq:
  - id: header
    type: header

instances:
  tabla8:
    pos: header.offset8
    type: tabla_ifs
    repeat: expr
    repeat-expr: 256
  tabla10:
    pos: header.offset10
    type: tabla_ifs
    repeat: expr
    repeat-expr: 256
  tabla12:
    pos: header.offset12
    type: tabla_ifs
    repeat: expr
    repeat-expr: 256
  tabla14:
    pos: header.offset14
    type: tabla_ifs
    repeat: expr
    repeat-expr: 256
  tabla128:
    pos: header.offset128
    type: tabla_ifs
    repeat: expr
    repeat-expr: 256