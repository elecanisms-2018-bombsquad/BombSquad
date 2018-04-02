/*
** Copyright (c) 2018, Bradley A. Minch
** All rights reserved.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are met:
**
**     1. Redistributions of source code must retain the above copyright
**        notice, this list of conditions and the following disclaimer.
**     2. Redistributions in binary form must reproduce the above copyright
**        notice, this list of conditions and the following disclaimer in the
**        documentation and/or other materials provided with the distribution.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
** AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
** IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
** ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
** LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
** CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
** SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
** INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
** CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
** ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
*/

#include "elecanisms.h"
// #include "usb.h"
#include <stdio.h>

#define s1248       0   // 539
#define s124        1   // 556
#define s128        2   // 576
#define s12         3   // 595
#define s148        4   // 618
#define s14         5   // 639
#define s18         6   // 664
#define s1          7   // 688      //these tw0
#define s248        8   // 695      // are closest
#define s24         9   // 721
#define s28         10  // 748
#define s2          11  // 775
#define s48         12  // 803
#define s4          13  // 830
#define s8          14  // 856
#define s           15  // 881
#define soops       16  // other


void ledoff(void){ LED1 = 0; LED2 = 0; LED3 = 0; }

uint16_t state;

int16_t main(void) {
    init_elecanisms();



while(1){
    uint8_t val = read_analog(A0_AN);

    if(val > 500 && val < 548){state = s1248;   }
    if(val > 548 && val < 566){state = s124;    }
    if(val > 566 && val < 608){state = s128;    }
    if(val > 608 && val < 629){state = s12;     }
    if(val > 629 && val < 650){state = s148;    }
    if(val > 650 && val < 675){state = s14;     }
    if(val > 675 && val < 691){state = s18;     }
    if(val > 691 && val < 708){state = s1;      }
    if(val > 708 && val < 732){state = s248;    }
    if(val > 732 && val < 765){state = s24;     }
    if(val > 765 && val < 790){state = s28;     }
    if(val > 790 && val < 816){state = s2;      }
    if(val > 816 && val < 842){state = s48;     }
    if(val > 842 && val < 864){state = s4;      }
    if(val > 864 && val < 900){state = s8;      }
    if(val > 900 && val < 920){state = s;       }
    if(val > 920 || val < 548){state = soops;   }

    switch(state){
        case soops:
            ledoff();
            LED1 = 1;
        break;

        case s1248:
            ledoff();
            LED2 = 1;
        break;

        case s124:
            ledoff();
            LED3 = 1;
        break;

        // default:
        //     state = soops;

    } // end of switch
} //end of while loop


} // end of main
