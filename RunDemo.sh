#!/usr/bin/env bash
# Use on linux and mac, build and run simple demo like ./RunDemo.sh annotation etc.

execdir=`pwd`

SYS_NAME=`uname`
if [ "$SYS_NAME" = "Darwin" ]; then
suffix_proj="mac"
elif [ "$SYS_NAME" = "Linux" ]; then
suffix_proj="linux"
else
suffix_proj="linux"
fi

SYS_MACHINE=`uname -m`
machine_config=${SYS_MACHINE}
if [ -z $machine_config ]; then
machine_config="x86_64"
fi

if [ "$SYS_MACHINE" = "x86_64" ]; then
machine_config="64"
elif [ "$SYS_MACHINE" = "i686" ]; then
machine_config="32"
elif [ "$SYS_MACHINE" = "i386" ]; then
machine_config="32"
elif [ "$SYS_MACHINE" = "armv7l" ]; then
machine_config="armv7"
elif [ "$SYS_MACHINE" = "aarch64" ]; then
machine_config="armv8"
elif [ "$SYS_MACHINE" = "arm64" ]; then
machine_config="arm64"
else
machine_config="64"
fi

if [ -z $1 ]; then
echo "Please input parameter."
exit 1
fi

for i in $@; do
if [ "$i" == "dbg" ]; then
IS_USE_DEBUG=1
continue
elif [ "$i" == "all" ]; then
PROJ_BUILD_ALL=1
fi
done

if [ -z $IS_USE_DEBUG ]; then
build_config=rel
else
build_config=dbg
fi

if [ -z $PROJ_BUILD_ALL ]; then
dir_input_arrs=$@
else
dir_input_arrs=`ls`
fi

proj_names=()
for i in ${dir_input_arrs[@]}; do
if [ -f $i/$i.cpp ]; then
proj_names[${#proj_names[@]}]=$i
fi
done

if [ ${#proj_names[@]} == 0 ]; then
echo "Please input correct parameter."
exit 1
fi

echo "Os:" ${suffix_proj}
echo "build_config:" ${build_config}
echo "proj_names length:" ${#proj_names[@]}
echo "proj_names:" ${proj_names[@]}

if [ -f "CMakeCache.txt" ]; then
  rm CMakeCache.txt
fi

for i in ${proj_names[@]}; do
cmake . -DPRJ_NAME=$i && make
done

for i in ${proj_names[@]}; do
if [ "$SYS_NAME" = "Darwin" ]&&[ ${i} = "dwg2pdf" ]; then
  export LD_LIBRARY_PATH=$DWG_ENGINE_PATH:$LD_LIBRARY_PATH
fi
echo && echo $i:
./${i}_${suffix_proj}${machine_config}
done

