#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <nids.h>

#define int_ntoa(x) inet_ntoa(*((struct in_addr *)&x))

int fifofd = -1;
char *fifoname;

void HandleExit(int signum)
{
  printf("Exiting...\n");
  if (fifofd >= 0)
    close(fifofd);
  if (access(fifoname, F_OK) != -1)
    unlink(fifoname);
  exit(0);
}

// struct tuple4 contains addresses and port numbers of the TCP connections
// the following auxiliary function produces a string looking like
// 10.0.0.1,1024,10.0.0.2,23
char *
adres(struct tuple4 addr)
{
  static char buf[256];
  strcpy(buf, int_ntoa(addr.saddr));
  sprintf(buf + strlen(buf), ",%i,", addr.source);
  strcat(buf, int_ntoa(addr.daddr));
  sprintf(buf + strlen(buf), ",%i", addr.dest);
  return buf;
}

void tcp_callback(struct tcp_stream *a_tcp, void **this_time_not_needed)
{
  char buf[1024];
  strcpy(buf, adres(a_tcp->addr)); // we put conn params into buf
  if (a_tcp->nids_state == NIDS_JUST_EST)
  {
    // a_tcp->client.collect++; // we want data received by a client
    a_tcp->server.collect++;
    // a_tcp->server.collect_urg++; // we want urgent data received by a
    // server
    fprintf(stderr, "%s established\n", buf);
    return;
  }
  if (a_tcp->nids_state == NIDS_CLOSE)
  {
    // connection has been closed normally
    fprintf(stderr, "%s closing\n", buf);
    return;
  }
  if (a_tcp->nids_state == NIDS_RESET)
  {
    // connection has been closed by RST
    fprintf(stderr, "%s reset\n", buf);
    return;
  }

  if (a_tcp->nids_state == NIDS_DATA)
  {
    // new data has arrived; gotta determine in what direction
    // and if it's urgent or not

    struct half_stream *hlf;

    if (a_tcp->server.count_new)
    {
      // new data from the client
      hlf = &a_tcp->server;
      // strcat(buf, "(->)");
    }
    else
    {
      return; // ignore data from server
    }

    // srcip,srcport,dip,dport,lenOfData;data
    strcat(buf, ",");
    // fprintf(stderr, "%s", buf);

    char *tmp_buf[10];
    memset(tmp_buf, 0, sizeof(tmp_buf));
    sprintf(tmp_buf, "%d", hlf->count_new);
    strcat(buf, tmp_buf);
    strcat(buf, ";");
    // fprintf(stderr, "%s", buf);

    //if (write(fifofd, buf, strlen(buf)) < 0)
    //{
    //  perror("write fifo failed");
    //  exit(1);
    //}

    fprintf(stderr, "%s", buf); // we print the connection parameters
                                // (saddr, daddr, sport, dport) accompanied
                                // by data flow direction (-> or <-)
    fprintf(stderr, "writing to fifo, len:%d...\n",hlf->count_new);
    if (write(fifofd, hlf->data, hlf->count_new) < 0)
    {
      perror("write fifo failed");
      exit(1);
    }
    write(2, hlf->data, hlf->count_new); // we print the newly arrived data
  }
  return;
}

int main()
{
  // here we can alter libnids params, for instance:
  // nids_params.n_hosts=256;
  printf("Start....\n");
  void(*handleExit) = HandleExit;
  signal(SIGINT, handleExit);

  struct nids_chksum_ctl temp;
  temp.netaddr = 0;
  temp.mask = 0;
  temp.action = 1;
  nids_register_chksum_ctl(&temp, 1);

  nids_params.pcap_filter = "tcp";

  printf("Open fifo...");
  //fifoname = getenv("FIFONIDS");
  fifoname = "/tmp/fifoHttp";
  if(fifoname == NULL){
	  perror("env FIFONIDS not exist\n");
	  exit(1);
  }
  printf("%s\n",fifoname);

  if (access(fifoname, F_OK) != -1)
  {
    printf("fifo exists\n");
  }
  else
  {
    //if (mkfifo(fifoname, 660) < 0)
    //{
    //  perror("make fifo failed. Exiting...\n");
    //  exit(1);
    //}
    //printf("make fifo success\n");
    //if (chown(fifoname, 1001, 1001) < 0){
    //  perror("chown fifo failed. Exiting...\n");
    //  exit(1);
    //}
    //printf("chown success\n");
  }
  printf("Opening fifo...\n");
  if ((fifofd = open(fifoname, O_WRONLY)) < 0)
  {
    perror("Open fifo failed!\n");
    exit(1);
  }
  printf("Open fifo success\n");

  printf("Nids Init....\n");
  if (!nids_init())
  {
    fprintf(stderr, "%s\n", nids_errbuf);
    exit(1);
  }
  printf("Nids register tcp ...\n");
  nids_register_tcp(tcp_callback);
  printf("Nids run!\n");
  nids_run();
  return 0;
}
