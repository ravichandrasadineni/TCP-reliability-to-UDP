#ifndef	urtt_h
#define	urtt_h

#include "unp.h"

struct urtt_info {
  int		rtt_rtt;	/* most recent measured RTT, seconds */
  int		rtt_srtt;	/* smoothed RTT estimator, seconds */
  int		rtt_rttvar;	/* smoothed mean deviation, seconds */
  int		rtt_rto;	/* current RTO to use, seconds */
  uint32_t	rtt_base;	/* #sec since 1/1/1970 at start */
};

#define	URTT_RXTMIN      1	/* min retransmit timeout value, seconds */
#define	URTT_RXTMAX      3	/* max retransmit timeout value, seconds */
#define	URTT_MAXNREXMT 	12	/* max #times to retransmit */

				/* function prototypes */

void	 urtt_init(struct urtt_info *);
int		 urtt_start(struct urtt_info *);
void	 urtt_stop(struct urtt_info *, uint32_t);
int		 urtt_timeout(struct urtt_info *);
uint32_t urtt_ts(struct urtt_info *);


#endif	
