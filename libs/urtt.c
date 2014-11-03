
#include	"urtt.h"
#define	RTT_RTOCALC(ptr) (((ptr)->rtt_srtt/8) +  ((ptr)->rtt_rttvar))

#define	URTT_RXTMIN      1000	/* min retransmit timeout value, milliSeconds */
#define	URTT_RXTMAX      3000	/* max retransmit timeout value, milliSeconds */
#define	URTT_MAXNREXMT 	12	/* max #times to retransmit */

 int urtt_minmax(int rto)
{
	if (rto < URTT_RXTMIN)
		rto = URTT_RXTMIN;
	else if (rto > URTT_RXTMAX)
		rto = URTT_RXTMAX;
	return(rto);
}

void urtt_init( urtt_info *ptr)
{
	struct timeval	tv;
	Gettimeofday(&tv, NULL);
	ptr->rtt_base = tv.tv_sec;		/* # sec since 1/1/1970 at start */
	ptr->rtt_rtt    = 0;
	ptr->rtt_srtt   = 0;
	ptr->rtt_rttvar = 3000; // 4 * 750
	ptr->rtt_rto = urtt_minmax(RTT_RTOCALC(ptr));
	ptr->prtt_rto = urtt_minmax(RTT_RTOCALC(ptr));
}

// has to be checked
uint32_t urtt_ts( urtt_info *ptr)
{
	uint32_t		ts;
	struct timeval	tv;

	Gettimeofday(&tv, NULL);
	ts = ((tv.tv_sec - ptr->rtt_base) * 1000) + (tv.tv_usec / 1000);
	return(ts);
}



int urtt_start( urtt_info *ptr,  int numOfTransmissions)
{
	if(numOfTransmissions == 0)
		ptr->prtt_rto = ptr->rtt_rto;
	return ptr->rtt_rto ;		
}


void urtt_stop( urtt_info *ptr, uint32_t ms, int numOfTransmissions)
{
	if((numOfTransmissions) > 0) {
		ptr->rtt_rto = ptr->prtt_rto;
		return;
	}
	double		delta;
	ptr->rtt_rtt = ms ;		

	delta = ptr->rtt_rtt - ptr->rtt_srtt/8;
	ptr->rtt_srtt += delta ;		

	if (delta < 0.0)
		delta = -delta;				

	ptr->rtt_rttvar += (delta - (ptr->rtt_rttvar) / 4);	/* h = 1/4 */

	ptr->rtt_rto = urtt_minmax(RTT_RTOCALC(ptr));
}


/* include rtt_timeout */
int urtt_timeout( urtt_info *ptr, int* numOfTransmissions)
{
	(*numOfTransmissions) ++;
	if (*numOfTransmissions >= URTT_MAXNREXMT) {
		 return 1;
	}
	ptr->rtt_rto = urtt_minmax(ptr->rtt_rto * 2);		/* next RTO */
	return(0);
}


