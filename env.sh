
# remember host python
HOST_PYTHON=`which python`
echo "HOST_PYTHON: $HOST_PYTHON"

# source the configuration for the toolchain
TOOL_CHAIN_DIR=/home/zeno/work/QtEnterprise/Boot2Qt-2.x/beaglebone-eLinux/toolchain
echo "TOOL_CHAIN_DIR: $TOOL_CHAIN_DIR"
. ${TOOL_CHAIN_DIR}/environment-setup-armv7ahf-vfp-neon-poky-linux-gnueabi

# override the path for python, as the toolchain only ships python 2.7.3 but we need >= 2.7.5
HOST_PYTHON_BIN=${TOOL_CHAIN_DIR}/python_bin
if [ ! -d "$HOST_PYTHON_BIN" ] ; then
	mkdir ${HOST_PYTHON_BIN}
	ln -s ${HOST_PYTHON} ${HOST_PYTHON_BIN}/python
fi

unset PYTHONHOME
export PATH=$HOST_PYTHON_BIN:$PATH


