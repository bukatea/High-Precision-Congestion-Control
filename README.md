# XCP<sub>INT</sub> NS-3 simulator
This is an NS-3 simulator for XCP<sub>INT</sub> based off of HPCC: High Precision Congestion Control (SIGCOMM'19). It also includes the implementation of DCQCN, TIMELY, DCTCP, PFC, ECN, Broadcom shared buffer switch, and of course HPCC. The HPCC paper can be found at: https://rmiao.github.io/publications/hpcc-li.pdf.

It is based on NS-3 version 3.17.

If you have any questions, please contact Rui Miao (miao.rui@alibaba-inc.com).

# NOTE: This is a modification of https://github.com/alibaba-edu/High-Precision-Congestion-Control to include XCP<sub>INT</sub>. This is meant to be built on Ubuntu 16.04, and, if build on this OS, there will be no need to install gcc-5 or g++-5, as it will already come preinstalled. In this case, all that you have to do is run the configure and run commands as described below, and compilation should proceed without issue. The following (until "Quick Start") are instructions on running simulations specific to XCP<sub>INT</sub> added post-fork.

The topology file is set up as follows (specify the name of this file in `mix/config.txt`, by default it is `mix/topology.txt`):

    <number of nodes> <number of switches> <number of links>
    <id of switch 1> <id of switch 2> ... <id of switch <number of switches>>
    <src1> <dst1> <rate> <delay> <error_rate>
    <src2> <dst2> <rate> <delay> <error_rate>
    ...
    <src<number of links>> <dst<number of links>> <rate> <delay> <error_rate>
    (More lines can be present, but they will not be used as input. Only <number of links> lines will be processed after the switch ids.)
    
The flow file is set up as follows (specify the name of this file in `mix/config.txt`, by default it is `mix/flow.txt`):

    <number of flows>
    <src1> <dst1> 3 <dst_port1> <size (number of bytes)> <start_time1>
    <src2> <dst2> 3 <dst_port2> <size (number of bytes)> <start_time2>
    ...
    <src<number of flows>> <dst<number of flows>> 3 <dst_port<number of flows>> <size (number of bytes)> <start_time<number of flows>>
    
Note that the size of the flow is in bytes and the default values are divisible by the packet payload size (by default s<sub>i</sub> = 1000 B). This can be changed in `mix/config.txt`.

All other configs are by default in `mix/config.txt`. To understand the config file format, refer to `mix/config_doc.txt`. For any further configuration changes, refer to this file. The code that takes in this config file and starts simulations is `scratch/third.cc`. To make any further changes to the setup of the simulations, refer to that file.

Also note that for convenience's sake I made it so that each sender (node) can have at most 1 flow specified in the flow file (this is an `NS_ASSERT`). This is because of some NS-3 C++-related issues with binding parameters to member functions as a callback. If needed, this can be changed, but honestly from what I've read I think this is fine. HPCC and XCP never use simulation scenarios where a sender has more than one flow at a time. To add another flow just add another node.

To run simulations, first run the configure command and then the run command as described below in "Build" and "Run". As stated above, if you are running this on Ubuntu 16.04 there is no need to install gcc-5 or g++-5 explicitly and set it using `CC=` and `CXX=`.

Note that `run.py` is currently not configured with XCP<sub>INT</sub> yet, but I think that this is not needed yet because we are still in the testing phase of the protocol itself. Once we are ready to run simulations, I will modify it to accept XCP<sub>INT</sub> as a CC protocol and have it generate traces accordingly.

As a final important implementation detail, note that there are currently two branches of the code: master and per-packet. The reason for this is that "master" is the implementation where instead of setting the per-packet feedback and incrementing the rate (by extension cwnd) every packet ACK for a control interval, it just aggregates them all into a per-flow feedback and increments the rate once every control interval (each control interval dictates this feedback). On the other hand, the "per-packet" branch is the "correct" implementation where the per-packet feedback is calculated every control interval and used to increment the rate (cwnd) every packet ACK. I realize that the "master" branch is an implementation contrary to what is described in the paper, but the reason this is included is because this actually has better performance than the per-packet version, for reasons we have to investigate deeper later.

In the actual CC code (contained in `point-to-point/model/rdma-hw.{cc,h}`, `point-to-point/model/rdma-queue-pair.{cc,h}`, and `point-to-point/model/switch-node.{cc,h}`), there are debug print statements for XCP<sub>INT</sub> denoted by `std::cerr` or `std::cout` that you can use to investigate some stuff.

## Quick Start

### Build
`./waf configure`

Please note if gcc version > 5, compilation will fail due to some ns3 code style.  If this what you encounter, please use:

`CC='gcc-5' CXX='g++-5' ./waf configure`

### Experiment config
Please see `mix/config.txt` for example. 

`mix/config_doc.txt` is a explanation of the example (texts in {..} are explanations).

### Run
The direct command to run is:
`./waf --run 'scratch/third mix/config.txt'`

We provide a `run.py` for automatically *generating config* and *running experiment*. Please `python run.py -h` for usage.
Example usage:
`python run.py --cc hp --trace flow --bw 100 --topo topology --hpai 50`

## Files added/edited based on NS3
The major ones are listed here. There could be some files not listed here that are not important or not related to core logic.

`point-to-point/model/qbb-net-device.cc/h`: the net-device RDMA

`point-to-point/model/pause-header.cc/h`: the header of PFC packet

`point-to-point/model/cn-header.cc/h`: the header of CNP

`point-to-point/model/qbb-header.cc/h`: the header of ACK

`point-to-point/model/qbb-channel.cc/h`: the channel of qbb-net-device

`point-to-point/model/qbb-remote-channel.cc/h`

`point-to-point/model/rdma-driver.cc/h`: layer of assigning qp and manage multiple NICs

`point-to-point/model/rdma-queue-pair.cc/h`: queue pair

`point-to-point/model/rdma-hw.cc/h`: the core logic of congestion control

`point-to-point/model/switch-node.cc/h`: the node class for switch

`point-to-point/model/switch-mmu.cc/h`: the mmu module of switch

`network/utils/broadcom-egress-queue.cc/h`: the multi-queue implementation of a switch port

`network/utils/custom-header.cc/h`: a customized header class for speeding up header parsing

`network/utils/int-header.cc/h`: the header of INT

`applications/model/rdma-client.cc/h`: the application of generating RDMA traffic
