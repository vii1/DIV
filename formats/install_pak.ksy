meta:
  id: install_pak
  encoding: ascii
  endian: le

seq:
  - id: magic
    contents: ['stp',0x1a,0x0d,0x0a,0,0]
  - id: num_files
    type: u4
  - id: files
    type: file_desc
    repeat: expr
    repeat-expr: num_files
    
types:
  file_desc:
    seq:
      - id: name
        type: strz
        size: 16
      - id: offset
        type: u4
      - id: z_size
        type: u4
      - id: u_size
        type: u4
        
    instances:
      file:
        pos: offset + 8
        size: z_size
        type:
          switch-on: z_size < u_size
          cases:
            true: compressed_file(z_size)
            false: raw_file(z_size)
            
  compressed_file:
    params:
      - id: size
        type: u4
    seq:
      - id: contents
        size: size
        process: zlib
        
  raw_file:
    params:
      - id: size
        type: u4
    seq:
      - id: contents
        size: size
