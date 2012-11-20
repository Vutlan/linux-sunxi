#obj-y		+= nfd/

#obj-$(CONFIG_SUN4I_NANDFLASH_TEST)		+= nfd/
#obj-$(CONFIG_SUN4I_NANDFLASH_TEST)		+= nandtest/

obj-$(CONFIG_SUN4I_NANDFLASH_TEST)  += nand.o
nand-y := nfd/nand_blk.o nfd/nand_osal_for_linux.o\
lib/src/format/nand_format.o\
lib/src/logic/mbr.o lib/src/logic/bad_manage.o lib/src/logic/logic_ctl.o lib/src/logic/mapping.o lib/src/logic/mapping_base.o lib/src/logic/merge.o lib/src/logic/read_reclaim.o lib/src/logic/wear_levelling.o lib/src/logic/logic_cache.o\
lib/src/scan/nand_scan.o lib/src/scan/nand_id.o\
lib/src/physic/nand_phy.o lib/src/physic/nand_simple_r.o lib/src/physic/nand_simple_w.o\
lib/src/nfc/nfc_r.o lib/src/nfc/nfc_w.o\
nandtest/nand_test.o
