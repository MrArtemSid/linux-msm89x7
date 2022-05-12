make O=out ARCH=arm64 msm8937_twrp_defconfig
make -j$(nproc --all) O=out \
	ARCH=arm64 \
	CC=clang \
	CROSS_COMPILE=aarch64-linux-gnu- \
	CROSS_COMPILE_ARM32=arm-linux-gnu-

cat ./out/arch/arm64/boot/Image.gz ./out/arch/arm64/boot/dts/qcom/sdm430-asus-x00p.dtb ./out/arch/arm64/boot/dts/qcom/sdm430-xiaomi-prada.dtb ./out/arch/arm64/boot/dts/qcom/sdm430-xiaomi-land.dtb ./out/arch/arm64/boot/dts/qcom/sdm435-xiaomi-santoni.dtb  ./out/arch/arm64/boot/dts/qcom/sdm425-xiaomi-rolex.dtb > Image.gz-dtb
