#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <signal.h>
#include "public.h"

static int keep_running = 1;

RETSIGTYPE stop_server(int a) 
{
    keep_running = 0;
}
void sprint_mib_oid(char *buf, const oid *name, size_t len)
{
    int             i;
    for (i = 0; i < (int) len; i++)
        buf += sprintf(buf, ".%" NETSNMP_PRIo "u", name[i]);
}

void init_example_mib(void)
{
	init_devinfo();
	init_ifInfoTable();
	init_bridgeTable();
}

int main (int argc, char **argv) 
{
  int agentx_subagent=0; /* change this if you want to be a SNMP master agent */
  int background = 0; /* change this if you want to run in the background */
  int syslog_flag = 1; /* change this if you want to use syslog */

  /* print log errors to syslog or stderr */
  if (syslog_flag)
    snmp_enable_calllog();
  else
    snmp_enable_stderrlog();

  /* we're an agentx subagent? */
  if (agentx_subagent) {
    /* make us a agentx client. */
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
  }

  /* run in background, if requested */
  if (background && netsnmp_daemonize(1, !syslog_flag))
      exit(1);

  /* initialize tcpip, if necessary */
  SOCK_STARTUP;

  /* initialize the agent library */
  init_agent("demomib");

  /* initialize mib code here */

  init_example_mib();  

  /* initialize vacm/usm access control  */
  if (!agentx_subagent) {
      init_vacm_vars();
      init_usmUser();
  }

  /* example-demon will be used to read demomib.conf files. */
  init_snmp("demomib");

  /* If we're going to be a snmp master agent, initial the ports */
  if (!agentx_subagent)
    init_master_agent();  /* open the port to listen on (defaults to udp:161) */

  /* In case we recevie a request to stop (kill -TERM or kill -INT) */
  keep_running = 1;
  signal(SIGTERM, stop_server);
  signal(SIGINT, stop_server);

  syslog(LOG_INFO,"example-demon is up and running.\n");

  /* your main loop here... */
  while(keep_running) {
    /* if you use select(), see snmp_select_info() in snmp_api(3) */
    /*     --- OR ---  */
    agent_check_and_process(1); /* 0 == don't block */
  }

  /* at shutdown time */
  snmp_shutdown("example-demon");
  SOCK_CLEANUP;

  return 0;
}


