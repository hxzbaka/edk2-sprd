function platform_build_bootimg() {
        python3 "${ROOTDIR}/tools/mkbootimg.py" \
        --kernel "${WORKSPACE}/uefi-${DEVICE}-kernel" \
        --base 0x00000000 \
        --pagesize 2048 \
        --kernel_offset 0x00008000 \
        --ramdisk_offset 0x05400000 \
        --second_offset 0x00f00000 \
        --tags_offset 0x00000100 \
        --cmdline "console=ttyS1,115200n8" \
        --os_version "${BOOTIMG_OS_VERSION}" \
        --os_patch_level "${BOOTIMG_OS_PATCH_LEVEL}" \
        --header_version "${BOOTIMG_HEADER_VERSION}" \
        --output "${OUTDIR}/boot-${DEVICE}${EXT}.img" \
        || return "$?"
}