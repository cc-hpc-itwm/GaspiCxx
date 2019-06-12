#!/bin/bash
#
# Copyright (c) Fraunhofer ITWM - <http://www.itwm.fraunhofer.de/>, 2016
# 
# This file is part of GaspiCxx.
# 
# GaspiCxx is free software; you can redistribute it
# and/or modify it under the terms of the GNU General Public License
# version 3 as published by the Free Software Foundation.
# 
# GaspiCxx is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with GaspiCxx. If not, see <http://www.gnu.org/licenses/>.
#

GTEST_PARAMS=--gtest_output="xml:results.xml"
GTEST_TEST=""
if [ $# -gt 0 ]; then
  GTEST_TEST=--gtest_filter=*$2*
fi
SEGMENT_SIZE=$(expr 128 \* 1024 \* 1024)
TEST_CFG="--segment_id 0 --segment_size ${SEGMENT_SIZE} --queue 0 \
          --group all --num_threads 4"
MFILE=mfile
N=4

# Generate machinefile
rm -f ${MFILE}
for I in `seq 1 $N`
do
  hostname >> ${MFILE}
done

# Run tests
gaspi_cleanup -m ${MFILE}
gaspi_run -m ${MFILE} $1 ${GTEST_PARAMS} \
                         ${GTEST_TEST} \
                         ${TEST_CFG}
gaspi_cleanup -m ${MFILE}

rm -f ${MFILE}

