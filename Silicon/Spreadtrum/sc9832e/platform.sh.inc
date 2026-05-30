function platform_build_bootimg(){
		${ROOTDIR}/tools/repackimg.sh \
		--kernel "${WORKSPACE}/uefi-${DEVICE}-kernel" \
		--ramdisk ${ROOTDIR}/ramdisk.cpio.gz \
		--dt "${ROOTDIR}/Platform/${VENDOR_NAME}/${SOC_PLATFORM_L}/FdtBlob_compat/${PLATFORM_NAME}.dtb" \
		--pagesize "4096" \
		--kernel-offset 0x00008000 \
		--ramdisk-offset 0x05400000 \
		--tags-offset 0x00000100 \
		--os-version "${BOOTIMG_OS_VERSION}" \
		--os-patch-level "${BOOTIMG_OS_PATCH_LEVEL}" \
		--header-version "${BOOTIMG_HEADER_VERSION}" \
		--output "${OUTDIR}/boot-${DEVICE}${EXT}.img" \
		||return "$?"
}