/* dgramclnt.c:
 *
 * Example datagram client:
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "mkaddr.h"
#include <getopt.h>

#define SPAWN_THREADS

#ifdef SPAWN_THREADS

#include <pthread.h>  // for thread Items

// the struct below is passed to the two threads.
typedef struct
{
   int done;         /* flag for termination */
   int fd;           /* file handle for logging */
   int socket;       /* Socket  */

}datablock_t;

#endif

static void ffPrintBuff(uint8_t * buffer, int32_t bufferSize,uint8_t * Address);
int  ffReadFileToBuffer(char * filename, char **buf,int *  bufsz);

int g_Debug = 0;
#define DEBUG(x) ((x & g_Debug) == x)
#define DEBUG_SEND_PACKET 0x01


void print_usage(void)
{
   printf("UDP client - send a Packet\n");
   printf("-s w,x,y,z    Server IP Address\n");
   printf("-p <port #>   Server port\n");
   printf("-b a.b.c.d    Bind IP Address  \n");
   printf("-c <port>     Bind port  \n");
   printf("-f <Data File>  Hex File containing Packet Data\n");
   printf("-l <count>      Number of times to send Packet Data\n");
   printf("-r            Wait for Response\n");
   printf("-m            Open Socket w/ SO_REUSEADDR (MUST Bind)\n");
   printf("-d            debug value  1 -print send packet data\n");
   printf("\n");
   printf("example: \n");
   printf("./socket_client_udp -s 172.4.1.1 -p9099 -b172.4.1.2 -c9005  -fpacketdata -l 1 \n");
   printf("\n");
   printf(" --> If talking to an EBB, you may need to add an ARP entry (for the fictious IP address of the EBB\n");
   printf("  > arp -s 172.4.1.1 00:03:fb:ac:2A:20 \n");
   printf("\n");
}

#ifdef SPAWN_THREADS
pthread_t g_tid[2];             // posix thread array


#endif



char *g_SndPktData    = NULL;             /* send buffer ptr */
int  g_SndPktDataSz   = 0;     /* send buffer Size */
int  g_SndPktDataLoop = 1;     /* send buffer loops */

 
/*
 * This function reports the error and
 * exits back to the shell:
 */
static void
bail(const char *on_what) {
    fputs(strerror(errno),stderr);
    fputs(": ",stderr);
    fputs(on_what,stderr);
    fputc('\n',stderr);
    if(g_SndPktData != NULL) free(g_SndPktData);
    exit(1);
}


#ifdef SPAWN_THREADS

// TX THREAD
int tx(void* arg)
{
    datablock_t * pdb =(datablock_t *)arg;
//    char c;
//    char * pc = &c ;
//    int r;

    printf("Starting Tx Thread\n");
    printf("  Serial Handle: %d\n",pdb->fd);
    while( pdb->done == 0 )
    {

       usleep(10000);    // let the OS have the machine if noting to go out.

    }
    printf("Ending Tx Thread\n");
    return 1;
}




// RX_THREAD
int rx(void* arg)
{
    datablock_t * pdb =(datablock_t *)arg;
    printf("Starting Rx Thread\n");

    int z;
    unsigned int sz;  
    struct sockaddr_in adr;            /* AF_INET */
    char dgram[512];               /* Recv buffer */
// for time stamps
   time_t rawtime ;
   struct tm * timeinfo;


    int s = pdb->socket;

    while( pdb->done == 0) // this will be set by teh back door
    {
 
        /*
         * Wait for a response:
         */
        sz = sizeof adr;
        z = recvfrom(s,                /* Socket */
            dgram,           /* Receiving buffer */
            sizeof dgram,   /* Max recv buf size */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr,     /* Addr */
            &sz);             /* Addr len, in & out */
        if ( z < 0 )
            bail("recvfrom(2)");

        dgram[z] = 0;          /* null terminate */

        /*
         * Report Result:
         */
#if 0
        printf("\nResult from %s port %u :'%s'\n",
            inet_ntoa(adr.sin_addr),
            (unsigned)ntohs(adr.sin_port),
            dgram);
        printf("Enter format string :");
        fflush(stdout);
#else
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        printf("c%d:%d:%d %s:%u %s\n",timeinfo->tm_hour,timeinfo->tm_min
                               ,timeinfo->tm_sec
                               , inet_ntoa(adr.sin_addr)
                               ,(unsigned)ntohs(adr.sin_port)
                               ,dgram);


#endif

    }
    printf("End of  Rx Thread\n");
    return 1;
}






// assign functions to worker threads created
void *WorkerThread(void *arg)
{
pthread_t id = pthread_self();

   if(pthread_equal(id,g_tid[0]))
   {
       // tx
       tx(arg);
   }
   else
  {
      // rx
      rx(arg);
  }
  return NULL;

}
#endif


#ifndef TRUE
#define TRUE 1
#endif
//#define ENABLE_SO_REUSEADDR
#define FILE_NAME_SIZE 512
#define IP_ADDR_STRING_SIZE 64

int
main(int argc,char **argv) {
    int z;
    int r;
    unsigned int sz; 
    int loop; 
    char   srvr_addr[IP_ADDR_STRING_SIZE];
    struct sockaddr_in adr_srvr;       /* AF_INET */
    int                adr_srvr_port;  /* port  */
    int    len_inet;                   /* length  */
    struct sockaddr_in adr;            /* AF_INET */

    char   bind_addr[IP_ADDR_STRING_SIZE];
    struct sockaddr_in adr_bind;      /* AF_INET */
    int    len_bind;                  /* length  */
    int    adr_bind_port;             /* port  */

    int s;                         /* Socket  */
    char dgram[512];               /* Recv buffer */
    int SoReUseAddr = 0;
    static int so_reuseaddr = TRUE;
    int option = 0;
    int WaitForResponse = 0;

    char SndFileName[FILE_NAME_SIZE]; /* send buffer file name */

#ifdef SPAWN_THREADS
    int i = 0;
    int err;
    datablock_t db;

// Initialize Default Values
    memset((void*)&db,0,sizeof(datablock_t));


#endif

    /* Use default address: */
    strcpy(srvr_addr, "127.0.0.23");  // Default Server Address
    adr_srvr_port = 9090;             // Default Server Port

    memset(bind_addr,0,IP_ADDR_STRING_SIZE); //Default Bind Address 0(client select)
    adr_bind_port = 0;                // bind port address 
                                      //  needed to change port characteristics
                                      //     =0  // bind to any port

    memset(SndFileName,0,FILE_NAME_SIZE);

#if 0
    // Print Command Line Arguments
    for( z = 0 ; z < argc ; z++)
    {
       printf("arg[%d] = %s\n",z,argv[z]);
    }
#endif
 
 while ((option = getopt(argc, argv,"hrl:b:s:p:f:c:d:")) != -1) {
        switch (option) {
             case 'm' : SoReUseAddr=1 ;     // configure to bind w/ SO_REUSEADDR
                 break;
             case 'r' : WaitForResponse=1 ; // send only(0) or wait for response (1)
                 break;
             case 'p' : adr_srvr_port = atoi(optarg); // Server IP Port
                 break;
             case 's' : strcpy(srvr_addr,optarg);     // Server Ip Address
                 break;             
             case 'b' : strcpy(bind_addr,optarg);     // Interface to Bind To
                 break;
             case 'c' : adr_bind_port = atoi(optarg);   // Bind Port
                 break;
             case 'd' : g_Debug = atoi(optarg);         // Debug Value
                 break;
             case 'f' : strcpy(SndFileName,optarg);    // CLI or pkt data from File
                 break;
             case 'l' : g_SndPktDataLoop = atoi(optarg);// #of times to send File Data
                 break;
             case 'h' :                               // Print Help
             default: 
                 print_usage(); 
                 exit(EXIT_FAILURE);
        }
    }
    printf("Command Line Arguments:\n");
    printf("  %16s Server IP (machine we want to connect to)\n",srvr_addr);
    printf("              %4d Server Port\n",adr_srvr_port);
    printf("  %16s Bind IP\n",(bind_addr[0] == 0?"Default Port":bind_addr));
    printf("              %4d bind Port\n",adr_bind_port);
    printf("  %16s      Src Pkt Data\n",SndFileName[0]==0?"cli input":SndFileName);
    if( SndFileName[0] !=0 )
    {
        printf("                  Loop %s %d Times Data File name\n",SndFileName,g_SndPktDataLoop);
    }
    printf("  %16s Wait for Response\n",(WaitForResponse == 0?"no":"yes"));
    printf("  %16s so_reuseaddr\n",(SoReUseAddr == 0?"no":"yes"));
    printf("\n");

    if(g_Debug != 0)
    {
       printf(" 0x%4x  g_Debug\n",g_Debug);
    }


     /*
     * Create a socket address, to use
     * to contact the server with:  
     */

    memset(&adr_srvr,0,sizeof adr_srvr);
    adr_srvr.sin_family = AF_INET;
    adr_srvr.sin_port = htons(adr_srvr_port);
    adr_srvr.sin_addr.s_addr =
        inet_addr(srvr_addr);

    if ( adr_srvr.sin_addr.s_addr == INADDR_NONE )
        bail("bad address.");

    len_inet = sizeof adr_srvr;

   printf("Server to connect to %s  port %d\n",srvr_addr,adr_srvr_port);

    /*
     * Create a UDP socket to use:
     */
    s = socket(AF_INET,SOCK_DGRAM,0);
    if ( s == -1 )
        bail("socket()");

    printf("Socket Handle:  %d\n",s);
//<fs>
    if ( bind_addr[0] != 0x00 ) 
    {
        /* Addr on cmdline: */
       // bind_addr = argv[2];
       // adr_bind_port = 9091;
        printf("My interface IP:  %s  port %d\n",bind_addr,adr_bind_port);

        memset(&adr_bind,0,sizeof adr_bind);
        adr_bind.sin_family = AF_INET;
        adr_bind.sin_port = htons(adr_bind_port);
        adr_bind.sin_addr.s_addr = inet_addr(bind_addr);

        if ( adr_bind.sin_addr.s_addr == INADDR_NONE )
           bail("bad bind address.");

        len_bind = sizeof adr_bind;


        if( SoReUseAddr != 0)  // a 4th argument should force an error on second routine
        {
           printf(" setsocket opt SO_REUSE_ADDR\n");
           /*
           * Allow multiple listeners on the
           * broadcast address:
           */
           z = setsockopt(s,
             SOL_SOCKET,
             SO_REUSEADDR,
             &so_reuseaddr,
             sizeof so_reuseaddr);

             if ( z == -1 )
               bail("setsockopt(SO_REUSEADDR)");
         }
     /*
      * Bind our socket to the broadcast address:
      */
//      printadr("adr_bind",&adr_bind);
//      printf("    len_bind  %d\n",len_bind);

        z = bind(s,
           (struct sockaddr *)&adr_bind,
            len_bind);

        if ( z == -1 )
           bail("bind(2)");

      } // end bind address supplied

    // Check if Command Line or send from FIle
    if(SndFileName[0] == 0) // do Command Line Interface
    {

#ifdef SPAWN_THREADS

//  Launch Tx and Rx Thread
    db.done   = 0;  // Init Flag to run.
    db.fd     = 0;  // Read/Write File Handle.
    db.socket = s;  // pass the socket handle to the THreads
       
    while(i < 2)
    {
        err = pthread_create(&(g_tid[i]), NULL, &WorkerThread, &db);
        if (err != 0)
            printf("\ncan't create thread :[%s]", strerror(err));
        else
            printf("\n Thread created successfully\n");
        i++;
    }

#endif 




      printf(" ->enter \"quit\"  to exit\n");
      /*
       * Prompt user for input data to send:
       */
      fputs("\njust type, data send on <cr> ",stdout);
      
    for (;;) {
        if ( !fgets(dgram,sizeof dgram,stdin) )
            break;                        /* EOF */

        z = strlen(dgram);

        if ( z > 0 && dgram[z-1] == '\n' )
            dgram[z-1] = 0;   /* Stomp out newline */
        // if debug flag set, print out what we should be sending.
        if(DEBUG(DEBUG_SEND_PACKET))
        {
           printf(" chars to Send:  %d\n",z);
           for( r = 0 ; r < z ; r++)
              printf("%d  %c 0x%02x\n",r,dgram[r],dgram[r]);
           printf(" strlen  %d\n",(int)strlen(dgram));
        }

        /*
         * Send format string to server:
         */
        z = sendto(s,   /* Socket to send result */
            dgram, /* The datagram result to snd */
            (strlen(dgram)+1), /* The datagram lngth */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr_srvr,/* addr */
            len_inet);  /* Server address length */
        if ( z < 0 )
            bail("sendto(2)");

        /*
         * Test if we asked for a server shutdown:
         */
        if ( !strcasecmp(dgram,"QUIT") )
            break;          /* Yes, we quit too */

#ifndef SPAWN_THREADS
        if(WaitForResponse == 1)
        {
 
        /*
         * Wait for a response:
         */
        sz = sizeof adr;
        z = recvfrom(s,                /* Socket */
            dgram,           /* Receiving buffer */
            sizeof dgram,   /* Max recv buf size */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr,     /* Addr */
            &sz);             /* Addr len, in & out */
        if ( z < 0 )
            bail("recvfrom(2)");

        dgram[z] = 0;          /* null terminate */

        /*
         * Report Result:
         */
        printf("Result from %s port %u :\n\t'%s'\n",
            inet_ntoa(adr.sin_addr),
            (unsigned)ntohs(adr.sin_port),
            dgram);
        } // end wait for response
#endif   
      }  // end wait CLI loop


#ifdef SPAWN_THREADS
     db.done = 1;   // flag the threads to terminate

#endif
     }
   else   // send a fixed file
     {
     // open File
     r = ffReadFileToBuffer( SndFileName, &g_SndPktData, &g_SndPktDataSz);
     if(r != 0)
     {
        bail("Failed to Open packet Data File");
     }
      printf("Packet Data to Send: %s\n",SndFileName);
      ffPrintBuff((uint8_t *)g_SndPktData ,g_SndPktDataSz,0);
      printf("\n");
      printf("Send packet data %d times\n",g_SndPktDataLoop);

      for (loop = 0 ;loop < g_SndPktDataLoop ; loop++) 
      {
         printf("Sending %d time: ",loop+1);


        /*
         * Send format string to server:
         */
        z = sendto(s,     /* Socket to send result */
            g_SndPktData,   /* The datagram result to snd */
            g_SndPktDataSz, /* The datagram lngth */
            0,            /* Flags: no options */
            (struct sockaddr *)&adr_srvr,/* addr */
            len_inet);  /* Server address length */
        if ( z < 0 )
            bail("sendto(2)");
        printf("Sent %d  bytes",z);
        if ( z != g_SndPktDataSz)
            printf("Failed to send all bytes, pktsize: %d\n",g_SndPktDataSz);
        printf("\r");
 
        if(WaitForResponse == 1)
        {
        /*
         * Wait for a response:
         */
        sz = sizeof adr;
        z = recvfrom(s,                /* Socket */
            dgram,           /* Receiving buffer */
            sizeof dgram,   /* Max recv buf size */
            0,               /* Flags: no options */
            (struct sockaddr *)&adr,     /* Addr */
            &sz);             /* Addr len, in & out */
        if ( z < 0 )
            bail("recvfrom(2)");

        dgram[z] = 0;          /* null terminate */

        /*
         * Report Result:
         */
        printf("Result from %s port %u :\n\t'%s'\n",
            inet_ntoa(adr.sin_addr),
            (unsigned)ntohs(adr.sin_port),
            dgram);
        } 
      }// end loop
     free (g_SndPktData); // free the packet buffer

  } // end if send file
    /*
     * Close the socket and exit:
     */
    close(s);
    putchar('\n');
    printf("done\n");

    return 0;
}

int  ffReadFileToBuffer(char * filename, char **buf,int *  bufsz)
{
        int     len = 0 ;
        int      i;
        int      j;
        int     off;
        char *  buffptr;
        FILE     *fp = NULL;

        fp = fopen (filename, "r");
        if (fp == NULL) {
                printf ("Unable to open file %s\n", filename);
                return (EXIT_FAILURE);
        }
        fseek (fp, 0, SEEK_END);
        len = ftell (fp);
        if (len <= 0) {
                printf ("File %s empty\n", filename);
                return (EXIT_FAILURE);
        }
        fseek (fp, 0, SEEK_SET);
        buffptr = malloc (len);
        if (buffptr == NULL) {
                printf ("Unable to alloc memory\n");
                return (EXIT_FAILURE);
        }
        j = len;
        off = 0;
        while (j > 0) {
                i = fread (buffptr + off, 1, j, fp);
                off += i;
                j -= i;
        }
        fclose(fp);
        *buf = buffptr;
        *bufsz = len;
        return 0;

}


#define fsprint printf
#define LLU long long unsigned

static void ffPrintBuff(uint8_t * buffer, int32_t bufferSize,uint8_t * Address)
{
    uint8_t * tmpptr0  = buffer;
    uint8_t * tmpptr1  = tmpptr0;
    int64_t  i          = 0 ;
    int64_t  m          = 0 ;
    int64_t  n          = 0 ;
    int64_t  j          = 0 ;
    int64_t  PrintCount = 0 ;   // used as counter to denote when to print \nadderss
    int64_t  BlankCnt   = 0 ;


    // align the lead
    BlankCnt = (unsigned long)Address & 0x000000000f;

    // print the lead
    if( BlankCnt != 0)  // if 0 jump to main body
    {
        for ( PrintCount = 0 ; PrintCount < BlankCnt ; PrintCount++ )
        {
            if( PrintCount == 0) // space between fields
            {
                fsprint("\n%016x",(unsigned)((unsigned long)Address & ~0x000000000f));
                tmpptr1 = tmpptr0;
            }
            if( (PrintCount % 8) == 0)
            {
                fsprint(" ");
            }
            fsprint("   ");
        }
        PrintCount--;  // remove last increment of printcount
        // print PrintCount data
        for ( m = 0  ; (PrintCount < 0xf) && (i < bufferSize); i++, m++,PrintCount++)
        {
            if(PrintCount % 8 == 0)
            {
                fsprint(" ");
            }
            fsprint(" %02x",(unsigned char)(*tmpptr0++));
            Address++;
        }

        // special case here when count is less than one line and not starting at zero
        if ( i == bufferSize)
        {
            // print out the last space
            for (      ; (PrintCount < 0x0f) ; PrintCount++ )
            {
                if( PrintCount  % 8 == 0)
                {
                    fsprint(" ");
                }
                fsprint("   ");
            }
            // print PrintCount text space
            for ( PrintCount = 0 ; (PrintCount < BlankCnt) ; PrintCount++ )
            {
                if( PrintCount == 0)   // space between fields
                {
                    fsprint(" ");
                }
                else if( PrintCount  % 8 == 0)
                {
                    fsprint(" ");
                }
                fsprint(" ");
            }
            // print PrintCount characters
            for( n = 0 ; (n < m) ; n++)
            {
                if( n == 0 ) printf(" ");
                if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                    fsprint("%c",*tmpptr1);
                else
                    fsprint(".");
                tmpptr1++;
            }
            printf("\n");
            return;
        } // end i == bufferSize

        // print PrintCount text space
        for ( PrintCount = 0 ; (PrintCount < BlankCnt) ; PrintCount++ )
        {
            if( PrintCount == 0)   // space between fields
            {
                fsprint(" ");
            }
            else if( PrintCount  % 8 == 0)
            {
                fsprint(" ");
            }
            fsprint(" ");
        }
        // print PrintCount characters
        n = 0;
        for( n = 0 ; (PrintCount <= 0xf) && (n < m) ; n++,PrintCount++)
        {
            if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                fsprint("%c",*tmpptr1);
            else
                fsprint(".");
            tmpptr1++;
        }
    }

    // print the body
    PrintCount = 0;
    for (   ; i < bufferSize ; i++)
    {
        if( PrintCount == 0 )
        {
            fsprint("\n%016llx",((LLU)Address & ~0x0f));
            tmpptr1 = tmpptr0;
        }
        if(PrintCount % 8 == 0)
        {
            fsprint(" ");
        }
        fsprint(" %02x",(unsigned char)(*tmpptr0++));
        Address++;
        PrintCount ++;
        if( PrintCount  > 0x0f)
        {
            PrintCount = 0;
            for( j = 0 ; j < 16 ; j++)
            {
                if( j == 0 ) printf("  ");
                if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
                    fsprint("%c",*tmpptr1);
                else
                    fsprint(".");
                tmpptr1++;
            }
        }
    }

    // print out the last space
    m = PrintCount;
    for (      ; (PrintCount <= 0x0f) ; PrintCount++ )
    {
        if( PrintCount  % 8 == 0)
        {
            fsprint(" ");
        }
        fsprint("   ");
    }

    // print PrintCount characters
    for( n = 0 ; (n < m) ; n++)
    {
        if( n == 0 ) printf("  ");
        if((*tmpptr1 >=0x20) && (*tmpptr1 <= 0x7e))
            fsprint("%c",*tmpptr1);
        else
            fsprint(".");
        tmpptr1++;
    }
    fsprint("\n");
}

