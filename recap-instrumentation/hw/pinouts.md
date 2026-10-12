# RECAP sensor pinouts

## Flow rate sensor (SFM3000, Sensirion)

       ➔➔➔➔➔➔➔   [Positive air flow direction]
    ╭─╮
    │ ╰────────╮
    │          │ [Sensor bottom view]
    │          │ 1. SDA
    │          │ 2. GND
    │   1234   │ 3. VDD
    ╰───┊┊┊┊─╮ │ 4. SCL
        ┊┊┊┊ ╰─╯
        ┊┊┊┊
        1234     [PCB header]
                 1. SDA
                 2. GND
                 3. 5V
                 4. SCL

## Pressure sensor (SPD810, Sensirion)

### Pinouts:

    ╭──────────╮ [Sensor bottom view]
    │  H    L  │
    │  ◌    ◌  │ 1. SCL
    │          │ 2. VDD
    │   4321   │ 3. GND
    ╰───┊┊┊┊───╯ 4. SDA
        ┊┊┊┊
        ┊┊┊┊
        4321     [PCB header]
                 1. SCL
                 2. 3V3
                 3. GND
                 4. SDA

### High/low sensor holes:

    [Front view]
    ╭──────────╮
    │          │
    │          │
    │  H    L  │
    │  ◎    ◎  │
    ╰──────────╯

## H2S sensor (968-045, Spec Sensors)

               [Front of sensor]
    ╭────────╮ 1. N/C
    ╰──┬──┬──╯ 2. RXD
       │  │    3. TXD
       │  │    4. N/C
    ╭──┴──┴──╮ 5. N/C
    ╰────────╯ 6. GND
     12345678  7. N/C
      ┊┊  ┊ ┊  8. V+ (3.0 -- 3.6V)
      ┊┊  ┊ ┊
      ┊┊  ┊ ┊
     12345678  [PCB header]
               1. N/C
               2. H2S_RXD
               3. H2S_TXD
               4. N/C
               5. N/C
               6. GND
               7. N/C
               8. 3V3

## Mask microphone

Shield and ring/tip swapped by the cable inside enclosure.

    [Mic.]       [Enclosure]
    S     R T     S     R T
    ─────┰─┰─╮    ─────┰─┰─╮
    ─────┸─┸─╯    ─────┸─┸─╯
        ╎ ╎ ╎         ╎ ╎ ╎
    GND ┘ ╎ ╎     MIC ┘ ╎ ╎
    MIC ╌╌┴╌┘     GND ╌╌┴╌┘

## Ear microphone

The adapter converts the microphone's TRRS plug into a TRS.

Shield and ring/tip swapped by the cable inside enclosure.

    [Mic.]          [Adapter]     [Enclosure]
    S     R R T     S     R T     S     R T
    ─────┰─┰─┰─╮    ─────┰─┰─╮    ─────┰─┰─╮
    ─────┸─┸─┸─╯    ─────┸─┸─╯    ─────┸─┸─╯
        ╎ ╎ ╎ ╎         ╎ ╎ ╎         ╎ ╎ ╎
    MIC ┘ ╎ ╎ ╎     GND ┘ ╎ ╎     MIC ┘ ╎ ╎
    GND ╌╌┘ ╎ ╎     MIC ╌╌┴╌┘     GND ╌╌┴╌┘
    N/C ╌╌╌╌┴╌┘
