//
//  scu_init_script.h
//  sh2
//
//  Created by Antonio Malara on 25/02/2018.
//  Copyright © 2018 Antonio Malara. All rights reserved.
//

#ifndef scu_init_script_h
#define scu_init_script_h

{ opWrite, 0x00f00001, 0x15 },
{ opWrite, 0x00f00001, 0x40 },
{ opWrite, 0x00f00001, 0x4e },
{ opWrite, 0x00f00001, 0x15 },
{ opWrite, 0x00f00001, 0x05 },

{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0xee },

{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x66 }, // SCU read byte 1

{ opRead,  0x00f00001, serial_ack++ },
{ opWrite, 0x00f00000, 0x10 },

{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x10 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x70 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x03 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0xf3 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x00 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x04 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x2b },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 2
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 3
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 4
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 5
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 7
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },


{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 12
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 15
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 5
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 6
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 7
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

// --

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 13
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 14
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 8
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 9
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x66 }, // SCU read byte 11
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x00 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opWrite, 0x00f00000, 0x00 },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },
{ opRead,  0x00f00001, serial_ack++ },

{ opRead,  0x00f00000, 0x31 }, // SCU read byte 10

{ opWrite, 0x00f00001, 0x14 },

#endif /* scu_init_script_h */
