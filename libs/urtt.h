#ifndef	urtt_h
#define	urtt_h

#include "unp.h"

typedef struct {
  unsigned int		rtt_rtt;	/* most recent measured RTT, seconds */
  unsigned int		rtt_srtt;	/* smoothed RTT estimator, seconds */
  unsigned int		rtt_rttvar;	/* smoothed mean deviation, seconds */
  unsigned int		rtt_rto;    /* current RTO to use, seconds */
  unsigned int		prtt_rto;   /* previous RTO to use, seconds */
  uint32_t	rtt_base;	/* #sec since 1/1/1970 at start */
}urtt_info ;

				/* function prototypes */

void	 urtt_init( urtt_info *);
int		 urtt_start( urtt_info *, int);
void	 urtt_stop( urtt_info *, uint32_t, int );
int		 urtt_timeout( urtt_info *, int*);
uint32_t urtt_ts( urtt_info *);


#endif	
