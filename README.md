# RuuviRepeater

This firmware project was created using [Particle Developer Tools](https://www.particle.io/developer-tools/) and is compatible with all [Particle Devices](https://www.particle.io/devices/).

BLE repeater for RuuviTags

Works for me, YMMV

The program listens for correct manufacturer id (and advertising data length) and rebroadcasts all such messaces as a temporary beacon with identical MAC address.