meta:
  id: install
  file-extension: exe
  encoding: ascii
  endian: le
  imports:
    - fpg
    - fnt
    
types:
  tail:
    seq:
      - id: size_fpg_z
        type: u4
      - id: size_fpg
        type: u4
      - id: size_fnt1_z
        type: u4
      - id: size_fnt1
        type: u4
      - id: size_fnt2_z
        type: u4
      - id: size_fnt2
        type: u4
      - id: data_size
        type: u4
  
  install_data:
    seq:
      - id: app_name
        type: strz
      - id: copyright
        type: strz
      - id: pack_name
        type: strz
      - id: default_dir
        type: strz
      - id: msg_fin
        type: strz
      - id: msg_help
        type: strz
      - id: msg_disk_1
        type: strz
      - id: msg_disk_2
        type: strz
      - id: error
        type: strz
        repeat: expr
        repeat-expr: 9
      - id: total_len
        type: u4
      - id: pack_size
        type: u4
      - id: create_dir
        type: u4
      - id: include_setup
        type: u4
      - id: segundo_font
        type: u4
      - id: install_fpg
        size: _root.tail.size_fpg_z
        process: zlib
        type: fpg
      - id: small_fnt
        size: _root.tail.size_fnt1_z
        process: zlib
        type: fnt
      - id: big_fnt
        size: _root.tail.size_fnt2_z
        process: zlib
        type: fnt
        if: segundo_font != 0
        
instances:
  tail:
    pos: _io.size - 28
    type: tail
  
  install_data:
    pos: _io.size - tail.data_size
    type: install_data
