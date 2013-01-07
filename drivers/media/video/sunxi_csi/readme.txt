===========================================

Version: V1_11

Author:  raymonxiu

Date:     2012-12-31 15:35:29

Description:

newest module list:(X = 0 or 1)

insmod sun4i_csiX.ko ccm="gc0307" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="gc0308" i2c_addr=0x42
insmod sun4i_csiX.ko ccm="gt2005" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="gc2035" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="hi253" i2c_addr=0x40
insmod sun4i_csiX.ko ccm="ov5640" i2c_addr=0x78
insmod sun4i_csiX.ko ccm="s5k4ec" i2c_addr=0x5a


V1_00
CSI:Initial version for linux 3.3
1) Support DVP CSI and MIPI CSI

V1_10
CSI: Fix OV5640/GC0307 bugs; Optimizing standby and probe
1)Add sensor write try count to 3 in ov5640
2)Delete sensor set ae delayed work in ov5640
3)Move power ldo request and power on sequency to work queue in probe
4)Add semaphore in resume and open to ensure open being called after resume
5)Optimize power on timming in gc0307

V1_11
CSI: Add unlock when csi driver is opened twice; Optimizing ov5640 and gc2035
1) Add unlock when csi driver is opened twice
2) Force regulator disable when release
3) Optimizing GC2035 power on sequency
4) OV5640 disable internal LDO when initial
5) OV5640 recheck when af fw download is failed
6) OV5640 add IO oe disable before S_FMT and IO OE enable after S_FMT
