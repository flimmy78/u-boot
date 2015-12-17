#include <common.h>
#include <command.h>
#include <net.h>
#include <environment.h>
#include <linux/ctype.h>
#include <linux/string.h>
#include <netdev.h>
#include "tftp.h"
#include "bootp.h"
#include "udp.h"

#define TIMEOUT	 	20000UL		/* Seconds to timeout for a lost pkt	*/
#ifndef	CONFIG_NET_RETRY_COUNT
# define TIMEOUT_COUNT	10		/* # of timeouts before giving up  */
#else
# define TIMEOUT_COUNT  (CONFIG_NET_RETRY_COUNT * 2)
#endif

static ulong UdpTimeoutMSecs = TIMEOUT;
static int UdpTimeoutCountMax = TIMEOUT_COUNT;

#define MAX_LINE 200
#define MAX_SECTION 100
#define MAX_NAME 100

static IPaddr_t UdpServerIP;
static int	UdpServerPort;		/* The UDP port at their end		*/
static int	UdpOurPort;		/* The UDP port at our end		*/
static int	UdpTimeoutCount;		

static void UdpSend (void);
static void UdpTimeout (void);

/**********************************************************************/

static void
UdpSend (void)
{
	volatile uchar *	pkt;
	int			len = 0;
	
	/*
	 *	We will always be sending some sort of packet, so
	 *	cobble together the packet headers now.
	 */
	pkt = NetTxPacket + NetEthHdrSize() + IP_HDR_SIZE;
	len = strlen(pkt_data);
	memcpy(pkt, pkt_data, len);
	NetSendUDPPacket(NetServerEther, UdpServerIP, UdpServerPort, UdpOurPort, len);
}


static int inifile_handler(void *user, char *section, char *name, char *value)
{
	
	char *requested_section = (char *)user;
	int i;

	 if ( *name == 's' || *name == 'b' || *name == 't') {
	 	if ( *name == 't' ){
			if(run_command(value, 0) == -1){
				return 0;
			}
			return 1;
		}
		for(i=0; i<10; i++){
			if(run_command(value, 0) == -1){	
				if ( *name == 'b' ) {
					printf("bootm kernel and rootfs failure!\n");
					return 1;
				}
				printf("error, try again After 1 seconds\n");
				udelay(1000000);
				if ((i==9) || ctrlc()) {
					puts ("\nAbort\n");
					return 0;
				}
				continue;
			}
			break;
		}
		
	}
	else{
		setenv(name, value);
	}

	/* success */
	return 1;
}

static void
UdpHandler (uchar * pkt, unsigned dest, unsigned src, unsigned len)
{
	ushort proto;
	char line[MAX_LINE];
	char section[MAX_SECTION] = "";
	char prev_name[MAX_NAME] = "";
	char *start;
	char *end;
	char *name;
	char *value;
	int i, linelen;
	int newline = 1;
	int lineno = 0;
	int error = 0;
	int flag = 1;
	char *udpkt;
	char *udppkt;
	
	udpkt=(char *)malloc(1000*sizeof(char));
	udppkt = strcpy(udpkt,(char *)pkt);
	printf("receive udp packet\n");

	while(flag){
		end = memchr(udppkt, '\n', (size_t)len);
		if( *end != '\n')
			end = memchr(udppkt, '\r', (size_t)len);
		if (end == NULL) {
			if (len == 0)
				return;
			end = udppkt + len;
			newline = 0;
		}
		linelen = min((end - udppkt) + newline, MAX_LINE);
		memcpy(line, udppkt, linelen);
		if (linelen < MAX_LINE)
			line[linelen] = '\0';

		/* prepare the mem vars for the next call */
		len -= (end - udppkt) + newline;
		udppkt += (end - udppkt) + newline;
		
		lineno++;
		start = lskip(rstrip(line));	
loop:	
	    if (*start && *start != ':' && *start != '[' && *start != '#') {
		/* Not a comment, must be a name[=:]value pair */

			end = find_char_or_comment(start, '=');
			if (*end != '=')
				end = find_char_or_comment(start, ':');
			if (*end == '=' || *end == ':') {
				*end = '\0';
				name = rstrip(start);
				value = lskip(end + 1);
				end = find_char_or_comment(value, ';');
				if(*end == ';'){								//batch cmd
					*end = '\0';
					end = find_char_or_comment(value, '\0');
					rstrip(value);
					/* Strip double-quotes */
					if (value[0] == '"' &&
				   		value[strlen(value)-1] == '"') {
						value[strlen(value)-1] = '\0';
 							value += 1;
 					}

					/*
					 * Valid name[=:]value pair found, call handler
				 	*/
					strncpy0(prev_name, name, sizeof(prev_name));
					printf("start %d line:%s %s\n",lineno, name, value);
					if (inifile_handler(section, section, name, value) &&
				  	   !error){		
						start = ++end;
						goto loop;
					}
					else
						error = lineno;
				}
				else{
					end = find_char_or_comment(value, '\0');
					rstrip(value);
					/* Strip double-quotes */
					if (value[0] == '"' &&
				   		value[strlen(value)-1] == '"') {
						value[strlen(value)-1] = '\0';
 							value += 1;
 					}

					/*
					 * Valid name[=:]value pair found, call handler
				 	*/
					strncpy0(prev_name, name, sizeof(prev_name));
					printf("start %d line:%s\n",lineno, value);
					if (!inifile_handler(section, section, name, value) &&
				  	   !error)
							error = lineno;
						
				}
			} 
			else if (!error){
				/* No '=' or ':' found on name[=:]value line */
				error = lineno;
			}
		}
		else if ((*start == ':' && start[1] == ':') || *start == '#') {
			/*
			 * Per Python ConfigParser, allow '#' comments at start
			 * of line
			 */
		}
#if CONFIG_INI_ALLOW_MULTILINE
		else if (*prev_name && *start && start > line) {
			/*
			 * Non-blank line with leading whitespace, treat as
			 * continuation of previous name's value (as per Python
			 * ConfigParser).
			 */
		    
			if (!inifile_handler(section, section, prev_name, start) && !error)
				error = lineno;
		}
#endif
		else if (*start == '[') {
			/* A "[section]" line */
			end = find_char_or_comment(start + 1, ']');
			if (*end == ']') {
				*end = '\0';
				strncpy0(section, start + 1, sizeof(section));
				*prev_name = '\0';
			} else if (!error) {
				/* No ']' found on section line */
				error = lineno;
			}
		} 
		if(error){
			printf("error= %d\n", error);
			break;
		}
		printf("%d line done\n", lineno);
	}
	
	free(udpkt);
	udpkt = NULL;
	
	if ( error ){	
		Upgrade_flag = UPGRADE_FAILED;
		TftpNetReStartCount = 10;
	}
	else{
		Upgrade_flag = UPGRADE_SUCCESS;
		TftpNetReStartCount = 11;
	}	
}


/**
 * restart the current transfer due to an error
 *
 * @param msg	Message to print for user
 */
static void restart(const char *msg)
{
	printf("\n%s; Net starting again\n", msg);
#ifdef CONFIG_MCAST_TFTP
	mcast_cleanup();
#endif
	NetStartAgain();
}

static void
ICMPHandler (unsigned type, unsigned code, unsigned dport,
		 unsigned sport, unsigned len)
{
	
		if (type == ICMP_NOT_REACH && code == ICMP_NOT_REACH_PORT) {
			UdpTimeoutMSecs = 5000UL;
			NetSetTimeout (UdpTimeoutMSecs, UdpTimeout);
	}
	
}

static void
UdpTimeout (void)
{
	if (++UdpTimeoutCount > UdpTimeoutCountMax) {
		if ( ++TftpNetReStartCount > TftpNetReStartCountMax ){
			return;
		}
		switch(UdpTimeoutMSecs){
			case 5000:  
				buzzer_notify(200, 1);
				restart("UDP server died");
				break;
			case 20000:  
				buzzer_notify(200, 2);
				restart("GetPacke Retry count exceeded");
				break;
			default:
				buzzer_notify(200, 3);
				restart("Retry count exceeded");
				break;
		}
		
	} else {
		puts ("T ");
		NetSetTimeout (UdpTimeoutMSecs, UdpTimeout);
		UdpSend ();
	}
}


void
UdpStart (void)
{
	char *ep;             /* Environment pointer */

	/*
	 * Allow the user to choose UDP blocksize and timeout.
	 * UDP protocol has a minimal timeout of 1 second.
	 */

	if ((ep = getenv("udptimeout")) != NULL){
		printf("udptimeout = %d\n", UdpTimeoutMSecs);
		UdpTimeoutMSecs = simple_strtol(ep, NULL, 10);
	}
	if (UdpTimeoutMSecs < 20000) {
		printf("UDP timeout (%ld ms) too low, "
			"set minimum = 20000 ms\n",
			UdpTimeoutMSecs);
		UdpTimeoutMSecs = 20000;
	}

	UdpServerIP = NetServerIP;
	
#if defined(CONFIG_NET_MULTI)
	printf ("Using %s device\n", eth_get_name());
#endif		
	UdpTimeoutCountMax = 5;
	UdpTimeoutCount = 0;
	
	NetSetTimeout (UdpTimeoutMSecs, UdpTimeout);
	NetSetHandler (UdpHandler);
	NetSetIcmpHandler(ICMPHandler);  //NO server
	
	UdpServerPort = 75;
	UdpOurPort = getenv("ourport");
	
	/* zero out server ether in case the server ip has changed */
	memset(NetServerEther, 0, 6);

	UdpSend ();
}
