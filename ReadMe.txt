/**************Team Members ***********************/
Ravi Chandra Sadineni (109754581)
Harshavardhan Chowdary Ellanti (109596321)
Harika Davala(109762085)
/**************************************************/


The given get_ifi_info already filters out the broadcast addresses. We are doing it in three ways
1) The code given by the steven's specifically creates a separate socket for broadcast address if there doesn't exist one before. We haven't included that section of code in our implementation since we don't want to bind to broadcast address

2) To hold the details we have created an array of structure which contain the following information
		1. sockfd
		2. ipAddr
		3. subnetMask
		4. networkAddress
	please refer to libs/socketinfo.h for the structure
	

Number of retransmission has been moved from urtt_info structure to sliding window. Below is the structure of sliding window


Regarding RTO:
		Modifications Done For RTT
			1.  URTT_RXTMIN = 1000,URTT_RXTMAX = 3000 and  RTT_MAXNREXMT to 12.
			2.  Maintian MilliSecs insteads of Secs.
			3. Change Min Max Function(ranges between 1000 millisecs and 3000 millisecs)
			4. Implemented Karn's algorithm while finding the new time-out.
			5. Moved Number of Retransmissions from rrt_info structure to slidingWindow Structure.
			Please  look into libs/urtt.c and  libs/urtt.h for further clarification
			
The alarm is set for the first packet that is being sent on network. We will wait on the recvMsg call for response. If the time-out(using setItimer) occurs for the packet then sigLongJump is called and then sigSetJump is called, in sigSetJump we are calling urtt_timeout which will effectively time-out on the particular packet we are waiting for, we again call urtt_start for the packet with the updated 'RTO' where we will make sure that the time-out will not exceed 3000ms and should not be less than 1000ms.


TCP Mechanisms implemented :

Sender sliding window: 
	We have the implemented a structure for storing all the necessary fields for the data transmission. The structure is "serverWindowSegement" which present in /libs/bufferHandler.h. The "serverWindowSegement" contains flags for various purposes like for storing timestamp,numberofAcks etc along with "hdr" and "data" having size 24 and 488 bytes respectively. Since we need to send only 512 bytes for each segment we have limited the size.
	After the second handshake we have the server sequence number(generated using random()) from which we know the client acknowledgement number. We are finding the size of the file that we need to transmit and get the total number of segments that the file can be broken down to and we are building segments with the required details(hdr,data). Now we have a window of size sliding window with head pointer pointed to starting node and tail pointing to last pointer of the window. At any point in time head is pointed to the segment that should receive an acknowledgement and tail should be pointed to the last element in the the window. If we get the acknowledgement for the segment for which head is pointed to, then we will move to next node and tail is also moved to next node. If the file size is more than the current window size, we are sliding the window keep the length of sliding window constant.  For more details please look into libs/bufferHandler.c. 

Receiver sliding window:
	We have implemented a structure for storing all the necessary fields for receiving data and bookkeeping . The structure is "clientWindowSegement" which present in /libs/threadUtility.h.  
	Based on the second handshake client will have the sequenceNumber of the first packet of the segment when file is received. We have a header pointer pointed to first node which is expected to be received first, and tail will point to the last node until which we have received. We will create the sliding window based on the input from the client.in.
		clientWindowSegement contains : hdr --> header of the  packet(if it is received already)
									  : data --> data segment of the received packet(if it is received already)
									  : server seq No  --> sequence Number of the packet that is expected to occupy the position
									  : isRecieved --> to check if the packet has already been received.

		This segments are part of a SharedBuffer structure which is shared between producer and consumer thread.
						
	There are three scenarios in this sliding window. 
			1.  Initially when no packet is received head and tail are pointed to  Null. If first packet is received then head is pointed to the first segment and so is tail. 
			2. when we receive any packet other than first packet we will move the tail from the current position to the position of the packet we received and add the packet at the particular position. Intermediate positions are left empty if the packets in that position are not received. Advertising  is (window Size - window size is returned as the count of the nodes from head to the packet which is not received).
			3.  when we receive any packet whose sequence number is between the head and tail we traverse to the position of the packet and insert it at the corresponding position based on the sequence number. Advertising window is calculated similarly as above and Current Ack number will be sent to server.
	
	We are implementing mutex and threads on the above buffer for reading and writing the buffer to make sure that no race conditions occur. We are acquiring lock before we update the buffer in producer thread and release it when done. On consumer side also we will lock the buffer before reading the contents and release when done. 
	
	Persistent Timer :
		 We have timeout mechanism in the server to probe the client in case it receives a segment with Advertising window of Size 0. On 12 retires if the window size still remains 0 we give up.
	
Congestion Control:
	We have implemented the simple method of congestion control which is discussed in class. There are two variables CWIN(Congestion Window) and SSThreshold(SSThreshold) used to represent congestion window size and SSThreshold size respectively. Initial value of CWIN is set to 1 and SSThreshold is NOT SET. CWIN value represent the maximum value of in-flight packets that can be there in network at any given time. There are three scenarios in this
	IF SSThreshold is NOT SET then
	First, initially if we receive an acknowledgement for the segment we will increase the CWIN by 2 for each acknowledgement received until we got time-out or duplicate acknowledgement for three times.
	Second, if we receive time-out then we will reduce the CWIN = 1 and SSThreshold = floor(CWIN/2)[SSThreshold is set to 2 if we get SSThreshold < 2]
	Third, if we receive duplicate acknowledgement three times for the same packet then we are reducing CWIN = floor(CWIN /2) and SSThreshold = floor(CWIN/2)
	IF SSThreshold is SET then
	We are increasing the CWIN by 2 for every acknowledgement received until it reaches SSThreshold. After reaching the SSThreshold we are increasing the value of CWIN by 1 only when the number of acknowledgement received is equal to SSThreshold.
	If we receive time-out then we are setting CWIN = 1 and SSThreshold = floor(CWIN/2)[SSThreshold is set to 2 if we get SSThreshold < 2]
	If we receive duplicate acknowledgements(fast retransmit) then we are setting CWIN = floor(CWIN/2)[CWIN is set to 1 if we get CWIN < 1] and SSThreshold = floor(CWIN/2)[SSThreshold is set to 2 if we get SSThreshold < 2]
	




Point 5:

Closing at Child:
	We are having a finflag in the "messageHeader" structure which will be set for the last segment when transmitted so that the client will know that the current segment is the last segment. Thus the client exits after successfully sending the acknowledgement to last fin segment successfully.
	
Closing at Server:
	The childServer will be waiting for a reply from the client even though it has sent last packet. The Server will exit only when the client acknowledges the last segment. Thus we are making sure that the connection is closed cleanly.
	
Handling Zombies :
	We are having waitpid() in the parent process to catch SIGCHILD which will be called after the successful termination of child. Thus eliminating the zombie process created by childServer.
	
