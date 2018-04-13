Optimized D2D Scheduler
===============================

## Introduction

It is estimated that by 2020 25 [1] to 50 [2] billion objects will be connected to the Internet. The majority of these devices will be battery powered and communicate wirelessly, especially in the context of the IoT, requiring approaches to exchange content in an energy efficient way.
Nowadays the vast majority of information is transported via the internet to a cloud service, even though devices being interested in the information may be in close proximity. Examples are the exchange of photos or other multimedia by using instant messengers on mobiles between people standing next to each other or the exchange of presentations / papers on a conference via email even though the persons are in the audience.
The transfer of information via the Internet to a communication partner in the vicinity does not only cause additional load in the infrastructure, e.g. cellular network, WiFi cell, due to the fact that the data has to be transferred to and received from Internet, but also consumes additional energy and causes higher channel occupancy since the data is usually exchanged with a much slower speed than if it would be transferred locally between devices.

## Description

Our approach targets a local content exchange between information providers and information consumers using common wireless technologies. Devices are connected either via cellular network or via WiFi access points to the Internet. In order to support the discovery of suitable communication partners and to schedule the exchange of content the process is supported by the infrastructure.

To enable the exchange of content the devices have to describe the content they are willing to provide as well as content they are interested in and report the description in the form of meta-data to an Internet based controller. Additionally, the description of the content may contain a quality indicator. Furthermore, the location of the devices is provided either by each device itself or by an external service keeping track of the movement of devices. Devices may report additional information, such as battery level, available storage, wireless communication capacities, etc. to the controller.
The controller collects the content announcements and content interests of the devices as well as additional device information and location information. Using this information the controller generates a schedule specifying the content exchange between devices.  The time is slotted in so called superslots. Schedules are computed for each superslot which may be tens of seconds in length due to the fact that devices require time in order to setup and join networks before exchanging content.
In the first step a list of potential content exchanges is created based on the interests of content consumers, the available content of content providers and an estimation of the connectivity between the devices. In the next step for each of the entries in the list of potential exchanges an utility value is computed using a quality indicator of the content and an estimation of the link quality. Furthermore, in order to prevent a starvation of entries the utility value of previous iterations is also considered. In the last step a D2D exchange schedule is computed using  the provided algorithm. 


## Usage

A scenario can either be described by directly setting the Gurobi variables in the code or by modifying the following files.

Devices are described in the devices.dat file by their device ID, a list of supported channel IDs as well as a list of content IDs which is available at the beginning of scheduling round.
```
#deviceID (list of supported channel IDs) (list of available content IDs) (list of interested content IDs)
0	0		0,1	-
1	0,1	-	0,1
2	0,1	-	0
(...)
```

Channels are described by their ID in channels.dat.
```
#channelID
0
1
2
(...)
```

Content entries are described in the content.dat by IDs and their size.
```
#contentID contentSize
0	2
1 	10
(...)
```

The intial state of each device at the beginning of a round is defined in state.dat. If a device is in Client mode the deviceID of the corresponding AP has to be provided. If a device is in AP mode the channelID of the operating channel has to be provided.
```
#deviceID deviceState channelID/deviceID of AP/dummy (for Idle)
# 0 = Idle(default), 1 = AP, 2 = Client
0	0	0
1   1   0
(...)
```

All potential exchanges are described in the utility.dat. Each potential exchange consist of a deviceID of the provider, a deviceID of the consumer, a contentID and an utility value that will be gained if the exchanged is scheduled.
```
#contentProviderID contentConsumerID contentID utilityValue
0	1	0	10
1	2	0	5
```

The link speeds between device for each channel is defined in link_speed.dat A link speed of zero corresponds to no connectivity.
```
#contentProviderID contentConsumerID  channelID linkSpeed(0=no connectivity)
0	1	0	1
1	2	0	1
1	2	1	1
```

In case the transmission of a device on a channel causes interference on one or multiple channels, it may be described in the interference.dat. 
```
#transmittingDeviceID transmittingChannelID receivingDeviceID (List of interferred channel IDs)
0 	0 	1 	0,1,2
```


## Requirements

This implementation requires the Gurobi Solver

http://www.gurobi.com/

## Contact

Niels Karowski (karowski@tkn.tu-berlin.de)

## References

[1]	G. P. Release, "http://www.gartner.com/newsroom/id/2905717."

[2]	D. Evans, "The internet of things: How the next evolution of the internet is changing everything," CISCO white paper, vol. 1, pp. 1-11, 2011.


