meta:
  id: wld
  file-extension: wld
  encoding: ascii #cp850
  endian: le
  
types:
  point:
    seq:
      - id: active
        type: u4
      - id: x
        type: s4
      - id: y
        type: s4
      - id: links
        type: u4
  wall:
    seq:
      - id: active
        type: u4
      - id: type
        type: s4
      - id: p1
        type: s4
      - id: p2
        type: s4
      - id: front_region
        type: s4
      - id: back_region
        type: s4
      - id: texture
        type: s4
      - id: texture_top
        type: s4
      - id: texture_bottom
        type: s4
      - id: fade
        type: u4  # 0-16
  region:
    seq:
      - id: active
        type: u4
      - id: type
        type: s4
      - id: floor_height
        type: s4
      - id: ceil_height
        type: s4
      - id: floor_tex  
        type: s4
      - id: ceil_tex
        type: s4
      - id: fade
        type: u4  # 0-16
  flag:
    seq:
      - id: active
        type: u4
      - id: x
        type: s4
      - id: y
        type: s4
      - id: number
        type: s4
  vpe_map:
    types:
      vpe_point:
        seq:
          - id: type  # siempre 0
            type: u4
          - id: x
            type: s4
          - id: y
            type: s4
          - id: path  # siempre -1
            type: s2
          - id: link  # siempre -1
            type: s2
      vpe_region:
        seq:
          - id: type  # siempre 0
            type: u4
          - id: floor_height
            type: s2
          - id: ceil_height
            type: s2
          - id: below # siempre -1
            type: s2
          - id: above # siempre -1
            type: s2
          - id: floor_tex
            type: s4
          - id: ceil_tex
            type: s4
          - id: eff   # siempre "NO_NAME"
            type: strz
            size: 9
          - size: 1   # padding
          - id: fade  # 0-16
            type: s2
          - id: tag   # siempre 0
            type: s2
      vpe_wall:
        seq:
          - id: type
            type: u4
          - id: p1
            type: s2
          - id: p2
            type: s2
          - id: front   # front region
            type: s2
          - id: back    # back region
            type: s2
          - id: top_tex
            type: s4
          - id: mid_tex
            type: s4
          - id: bot_tex
            type: s4
          - id: eff     # siempre "NO_NAME"
            type: strz
            size: 9
          - size: 1     # padding
          - id: fade    # 0-16
            type: s2
          - id: tex_x   # siempre 0
            type: s2
          - id: tex_y   # siempre 0
            type: s2
          - id: mass    # siempre 0
            type: s2
          - id: tag     # siempre 0
            type: s2
      vpe_flag:
        seq:
          - id: x
            type: s4
          - id: y
            type: s4
          - id: number
            type: s4
      vpe_move:
        seq:
          - id: x
            type: s2
          - id: y
            type: s2
          - id: z
            type: s2
          - id: t
            type: s2
    seq:
      - id: id_str
        contents: ["DAT",0]
      - id: num_points
        type: s2
      - id: num_regions
        type: s2
      - id: num_walls
        type: s2
      - id: num_flags
        type: s2
      - id: points
        type: vpe_point
        repeat: expr
        repeat-expr: num_points
      - id: regions
        type: vpe_region
        size: 36
        repeat: expr
        repeat-expr: num_regions
      - id: walls
        type: vpe_wall
        repeat: expr
        repeat-expr: num_walls
      - id: flags
        type: vpe_flag
        repeat: expr
        repeat-expr: num_flags
      - id: title     # siempre "Mapa1"
        type: strz
        size: 24
      - id: palette   # siempre "paleta"
        type: strz
        size: 9
      - size: 3       # padding
      - id: screen_tex  # siempre 0
        type: s4
      - id: back_tex
        type: s4
      - id: back_eff
        type: strz
        size: 9
      - size: 1       # padding
      - id: back_angle  # siempre 120
        type: s2
      - id: act_view    # siempre 0
        type: s2
      - id: force       # siempre { x:200, y:200, z:400, t:100 }
        type: vpe_move
seq:
  - id: magic
    contents: ["wld",0x1A,0x0D,0x0A,0x01,0]
  - id: offset_vpe
    type: u4
  - id: path
    type: strz
    size: 256
  - id: name
    type: strz
    size: 16
  - id: numero
    type: s4
  - id: fpg_path
    type: strz
    size: 256
  - id: fpg_name
    type: strz
    size: 16  
  - id: num_points
    type: s4
  - id: points
    type: point
    repeat: expr
    repeat-expr: num_points
  - id: num_walls
    type: s4
  - id: walls
    type: wall
    repeat: expr
    repeat-expr: num_walls
  - id: num_regions
    type: s4
  - id: regions
    type: region
    repeat: expr
    repeat-expr: num_regions
  - id: num_flags
    type: s4
  - id: flags
    type: flag
    repeat: expr
    repeat-expr: num_flags
  - id: fondo
    type: s4

instances:
  wld_vpe:
    type: vpe_map
    pos: offset_vpe + 12