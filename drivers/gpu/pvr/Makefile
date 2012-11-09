ccflags-y = -DLINUX -D__linux__ -DANDROID -DPVR_BUILD_DIR="\"sunxi_android\""
ccflags-y += -Idrivers/gpu/pvr \
             -Idrivers/gpu/pvr/include4  \
             -Idrivers/gpu/pvr/services4/include \
             -Idrivers/gpu/pvr/services4/srvkm/include \
             -Idrivers/gpu/pvr/services4/srvkm/env/linux \
             -Idrivers/gpu/pvr/services4/srvkm/hwdefs \
             -Idrivers/gpu/pvr/services4/srvkm/devices/sgx \
             -Idrivers/gpu/pvr/services4/srvkm/common \
             -Idrivers/gpu/pvr/services4/srvkm/bridged \
             -Idrivers/gpu/pvr/services4/srvkm/bridged/sgx \
             -Idrivers/gpu/pvr/services4/system/include \
             -Idrivers/gpu/pvr/services4/system/sunxi 

ccflags-y += \
            -DSGX_FEATURE_MP_CORE_COUNT=1 \
            -DPVR_BUILD_TYPE="\"release\"" \
            -DPVRSRV_MODNAME="\"pvrsrvkm\"" \
            -DSGXCORE=544   \
            -DSGX544  \
            -DSUPPORT_SGX544  \
            -DSUPPORT_SGX  \
            -DSGX_CORE_REV=115 \
            -DTRANSFER_QUEUE \
            -DPVR_SECURE_HANDLES \
            -DDISPLAY_CONTROLLER=dc_sunxi \
            -DPVR_LINUX_MEM_AREA_POOL_MAX_PAGES=10800 \
            -DPVR_LINUX_MEM_AREA_USE_VMAP \
            -DPVR_LINUX_MEM_AREA_POOL_ALLOW_SHRINK  \
            -DRELEASE  \
            -DSUPPORT_PERCONTEXT_PB \
            -DSUPPORT_HW_RECOVERY \
            -DSUPPORT_ACTIVE_POWER_MANAGEMENT \
            -DSUPPORT_SGX_HWPERF \
            -DSUPPORT_SGX_LOW_LATENCY_SCHEDULING  \
            -DSUPPORT_SGX_NEW_STATUS_VALS  \
            -DSUPPORT_DBGDRV_EVENT_OBJECTS  \
            -DSGX_FEATURE_SYSTEM_CACHE  \
            -DSGX_FEATURE_MP \
            -DSYS_USING_INTERRUPTS \
            -DSUPPORT_LINUX_X86_WRITECOMBINE \
            -DSUPPORT_LINUX_X86_PAT \
            -DSYS_SGX_ACTIVE_POWER_LATENCY_MS=50 \
            -DSYS_CUSTOM_POWERLOCK_WRAP  \
            -DPVR_LINUX_USING_WORKQUEUES \
            -DPVR_LINUX_MISR_USING_PRIVATE_WORKQUEUE  \
            -DPVR_LINUX_TIMERS_USING_WORKQUEUES  \
            -DLDM_PLATFORM  \
            -DPVR_LDM_DRIVER_REGISTRATION_NAME="\"pvrsrvkm\"" \
            -DSUPPORT_DC_CMDCOMPLETE_WHEN_NO_LONGER_DISPLAYED \
			-DSUPPORT_LARGE_GENERAL_HEAP \
			-DSUPPORT_MEMINFO_IDS


pvrsrvkm-y := \
	services4/srvkm/env/linux/osfunc.o \
	services4/srvkm/env/linux/mutils.o \
	services4/srvkm/env/linux/mmap.o \
	services4/srvkm/env/linux/module.o \
	services4/srvkm/env/linux/pdump.o \
	services4/srvkm/env/linux/proc.o \
	services4/srvkm/env/linux/pvr_bridge_k.o \
	services4/srvkm/env/linux/pvr_debug.o \
	services4/srvkm/env/linux/mm.o \
	services4/srvkm/env/linux/mutex.o \
	services4/srvkm/env/linux/event.o \
	services4/srvkm/env/linux/osperproc.o \
	services4/srvkm/common/buffer_manager.o \
	services4/srvkm/common/devicemem.o \
	services4/srvkm/common/deviceclass.o \
	services4/srvkm/common/handle.o \
	services4/srvkm/common/hash.o \
	services4/srvkm/common/lists.o \
	services4/srvkm/common/mem.o \
	services4/srvkm/common/mem_debug.o \
	services4/srvkm/common/metrics.o \
	services4/srvkm/common/osfunc_common.o \
	services4/srvkm/common/pdump_common.o \
	services4/srvkm/common/perproc.o \
	services4/srvkm/common/power.o \
	services4/srvkm/common/pvrsrv.o \
	services4/srvkm/common/queue.o \
	services4/srvkm/common/ra.o \
	services4/srvkm/common/refcount.o \
	services4/srvkm/common/resman.o \
	services4/srvkm/bridged/bridged_support.o \
	services4/srvkm/bridged/bridged_pvr_bridge.o \
	services4/system/sunxi/sysconfig.o \
	services4/system/sunxi/sysutils.o

# SUPPORT_SGX==1 only

pvrsrvkm-y += \
	services4/srvkm/bridged/sgx/bridged_sgx_bridge.o \
	services4/srvkm/devices/sgx/sgxinit.o \
	services4/srvkm/devices/sgx/sgxpower.o \
	services4/srvkm/devices/sgx/sgxreset.o \
	services4/srvkm/devices/sgx/sgxutils.o \
	services4/srvkm/devices/sgx/sgxkick.o \
	services4/srvkm/devices/sgx/sgxtransfer.o \
	services4/srvkm/devices/sgx/mmu.o \
	services4/srvkm/devices/sgx/pb.o

ccflags-y += \
	-Idrivers/gpu/pvr/services4/3rdparty/dc_sunxi \
	-Idrivers/video/
	
dc_sunxi-y := \
	services4/3rdparty/dc_sunxi/dc_sunxi_displayclass.o \
	services4/3rdparty/dc_sunxi/dc_sunxi_linux.o

obj-$(CONFIG_PVR_SGX) += pvrsrvkm.o
obj-$(CONFIG_PVR_SGX) += dc_sunxi.o
