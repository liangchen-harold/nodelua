NodeLua
=====================

NodeLua is a cross-platform firmware based on the lua language trying to make IoT programming easier. Not only an interpreter, but with a Web IDE, Cloud APIs, which makes you creating a real 'thing' running on your customers home more robustious and easier. It is currently running on ESP8266 and planned to support more chips, based on the lua language, nodejs-like APIs, but 10 times faster and 100 times smaller than nodejs. Which made it easily be fit into modules with only a few hundred kilobytes of Flash/RAM memory.

Major highlights:

* Access to all of the resources on the chip: GPIO, PWM, ADC, Timer, WIFI config, TCP server and client. Compile toolchain and the time wasting burning process are no longer needed.
* A very easy to use web based IDE development environment. You can program and load codes into modules over-the-air.
* Cloud APIs to storage the datas generating by the sensors on the modules(You can post data through only 2 lines of code!), to handle customer accounts, pairing, permissions, sharing devices with family and friends, control actions, triggers, timers send from customer’s phone or other sources.

Known issues:

* The RAM size on ESP8266 is really tight for the Lua interpreter, currently there are only ~30KB free memory could be used both by NodeLua and your codes, which means dozens lines of code will used up all the heap space.

You are welcome to be a early bird and help us to improve it!


Project site
===
http://nodelua.org

Docs
===
Full API documents are on the project site: http://nodelua.org/doc/api-references/wifi/

Usage
===
Try:

You can follow the tutorial to get start: http://nodelua.org/doc/tutorial/make-your-first-node/

Compile:

//TODO: to be done

Credits
===
NodeLua was written by Harold L. and released under the Apache License.
