
#include	"urtt.h"
#define	RTT_RTOCALC(ptr) (((ptr)->rtt_srtt/8) +  ((ptr)->rtt_rttvar))

static int urtt_minmax(int rto)
{
	if (rto < RTT_RXTMIN)
		rto = RTT_RXTMIN;
	else if (rto > RTT_RXTMAX)
		rto = RTT_RXTMAX;
	return(rto);
}

void urtt_init(struct urtt_info *ptr)
{
	struct timeval	tv;
	Gettimeofday(&tv, NULL);
	ptr->rtt_base = tv.tv_sec;		/* # sec since 1/1/1970 at start */
	ptr->rtt_rtt    = 0;
	ptr->rtt_srtt   = 0;
	ptr->rtt_rttvar = 3000; // 4 * 750
	ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
}

// has to be checked
uint32_t urtt_ts(struct urtt_info *ptr)
{
	uint32_t		ts;
	struct timeval	tv;

	Gettimeofday(&tv, NULL);
	ts = ((tv.tv_sec - ptr->rtt_base) * 1000) + (tv.tv_usec / 1000);
	return(ts);
}



int urtt_start(struct rtt_info *ptr)
{
	return ptr->rtt_rto ;		
}


void urtt_stop(struct rtt_info *ptr, uint32_t ms)
{
	double		delta;

	ptr->rtt_rtt = ms ;		

	delta = ptr->rtt_rtt - ptr->rtt_srtt/8;
	ptr->rtt_srtt += delta ;		

	if (delta < 0.0)
		delta = -delta;				

	ptr->rtt_rttvar += (delta - (ptr->rtt_rttvar) / 4);	/* h = 1/4 */

	ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr));
}


/* include rtt_timeout */
int urtt_timeout(struct rtt_info *ptr)
{
	ptr->rtt_rto *= 2;		/* next RTO */

	return(0);
}


