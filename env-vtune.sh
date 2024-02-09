# To use: 
#
# xzl@granger1$ source env-vtune.sh
#

source /opt/intel/vtune_profiler/vtune-vars.sh
export INTEL_LIBITTNOTIFY64=/opt/intel/vtune_profiler/lib64/runtime/libittnotify_collector.so


# adopted from p3 env.sh
# will gen deterministic ports instead
p1-gen-hash-ports() {
    export MYPORT=`echo -n ${USER} | md5sum | cut -c1-8 | printf "%d\n" 0x$(cat -) | awk '{printf "%.0f\n", 50000 + (($1 / 0xffffffff) * 10000)}'`
    echo "set vtune-backend port: ${MYPORT}"
}


usage () {
    cat << EOF
AVAILABLE COMMANDS 
------------------
    p1-run              Run qemu with the kernel (./kernel8.img)
    p1-run-log          Run qemu with the kernel, meanwhile writing qemu logs to qemu.log
    p1-run-debug        Run qemu with the kernel, GDB support on 

    (see env-qemu.sh for more commands)
EOF
}

echo `whereis vtune`
echo `whereis vtune-backend`
p1-gen-hash-ports
# usage

