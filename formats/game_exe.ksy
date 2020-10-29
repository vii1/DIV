meta:
  id: game_exe
  file-extension: exe
  endian: le
  encoding: ascii
  bit-endian: le
    
seq:
  - id: stub
    size: 602
  - id: flags
    type: program_flags
    size: 4
  - id: imem
    type: u4
  - id: imem2
    type: u4
  - id: max_process
    type: u4
  - id: reserved
    type: u4
  - id: iloc1
    type: u4
  - id: iloc
    type: u4
  - id: reserved2
    type: u4
  - id: mem_size
    type: u4
  - id: len_descomp
    type: u4
  - id: data
    size-eos: true
    process: zlib
    
types:
  program_flags:
    seq:
      - id: setup_program
        type: b1
      - id: dummy
        type: b6
      - id: debug_at_start
        type: b1
      - id: dummy2
        type: b1
      - id: ignore_errors
        type: b1
      - id: demo
        type: b1
