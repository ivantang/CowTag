#!/bin/bash

pushd `dirname $0` > /dev/null
SELF_PATH=`pwd -P`
popd > /dev/null

cfg_file_name="global_cfg.h"
cfg_file_path="${SELF_PATH}/${cfg_file_name}"

temp_cfg_file_name="global_cfg.h.template"
temp_cfg_file_path="${SELF_PATH}/${temp_cfg_file_name}"

sed "s/{timestamp}/$(date +%s)/" ${temp_cfg_file_path} > ${cfg_file_path}

