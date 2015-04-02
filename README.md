# E (Discrete event simulator)
====

# What is E?
E contains a delayed message scheduler and supplementing libraries for various simulations.
E is implemented with C++11 without any external libraries. So it can be run on any platform that supports C++11 standard.

## Delayed message
E encapsulates all events as "delayed messages".
Each message is an empty C++ object and can be extended for additional information.
Here are examples of useful appliance of delayed messages.

* Timer
	- A message with t delay can work as an alarm after t.
	- A message may contain additional information such as 'callback handler function' or 'source of alarm'.
* Pipe
	- When receiving a message, make a copy of it.
	- Pass the message to the other end.
* Wire (with propagation delay)
	- Same as pipe, but add aditional delay before sending it to the other end.
* Wireless medium
	- From a source node, calculate propagation delay to all other nodes.
	- According to the delay, broadcast messages to others.
	- Signal power can be also attached to the messages.
* Reflection of a mirror (laser)
	- From the light source, calculate the distance/angle to the mirror.
	- Send a delayed message which contains the information of the light.
	- When the mirror receives the message, calculate the outward angle (negate inward angle),
	distance to the destination, and send the copied message.
* Job execution of a processor
	- When a job is raised, calculate the execution time and create a delayed self message.
	- When the job is finished, update the run/idle time of the processor.
	- If another job preempts current job, cancel the self message and update run/idle for partial execution.
	After that, create a new message for the preempting job.

## Delayed message convention
* Create before send
	- When forwarding a message, do not reuse it. Make a copy of the message.
	- Do not "free" the original message to be forwarded.
* Take care of your "created" message
	- The creator deallocates the message.
* We provide interface for sending/receiving messages
	- MessageReceived: A module sent a message to you (including self-messages).
	Here, you may create a instant message response for your convenience.
	This message is sent via the return value of the "received" function.
	- MessageFinished: Your message is handled by other module (after the delay you requested).
	The instance message is passed by the parameter of "MessageFinished" handler.
	If you sent a instance message, it should be also freed here 
	(of course, in this case, there won't be a instance message response for "MessageFinished").
	- MessageCanceled: Message should be freed, but not sent to the destination.
	
If a set of modules are in your control, you may not follow the convention.
However, you should follow this for communication with other modules.


# Applications
E contains some useful applications for various kind of simulations.

## KENSv3 (KAIST Educational Network System)
KENS has been used for programming assignment for CS341: Introduction to Computer Network at KAIST (https://http://an.kaist.ac.kr/courses/2014/cs341/).
