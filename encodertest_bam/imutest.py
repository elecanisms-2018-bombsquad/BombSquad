#
## Copyright (c) 2018, Bradley A. Minch
## All rights reserved.
##
## Redistribution and use in source and binary forms, with or without
## modification, are permitted provided that the following conditions are met: 
## 
##     1. Redistributions of source code must retain the above copyright 
##        notice, this list of conditions and the following disclaimer. 
##     2. Redistributions in binary form must reproduce the above copyright 
##        notice, this list of conditions and the following disclaimer in the 
##        documentation and/or other materials provided with the distribution. 
##
## THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
## AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
## IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
## ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
## LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
## CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
## SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
## INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
## CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
## ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
## POSSIBILITY OF SUCH DAMAGE.
#

import mputest
import sys, time

foo = mputest.mputest()
foo.mpu_init()

while 1:
    sys.stdout.write('\x1b[2J\x1b[1;1f')

    accel = foo.mpu_read_accel()
    gyro = foo.mpu_read_gyro()
#    mag = foo.mpu_read_mag()

    print 'Accelerometer:'
    print '  x = {0:+05.3f}g'.format(accel[0])
    print '  y = {0:+05.3f}g'.format(accel[1])
    print '  z = {0:+05.3f}g'.format(accel[2])

    print '\nGyroscope:'
    print '  x = {0:+08.3f}dps'.format(gyro[0])
    print '  y = {0:+08.3f}dps'.format(gyro[1])
    print '  z = {0:+08.3f}dps'.format(gyro[2])

#    print '\nMagnetometer:'
#    print '  x = {0:+08.3f}uT'.format(mag[0])
#    print '  y = {0:+08.3f}uT'.format(mag[1])
#    print '  z = {0:+08.3f}uT'.format(mag[2])

    t0 = time.clock()
    while time.clock() < t0 + 0.05:
        pass

