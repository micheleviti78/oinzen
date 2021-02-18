/*
 *
 * (C) 2014 David Lettier.
 *
 * http://www.lettier.com/
 *
 * NTP client.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
//#netinet/in.h is not included in LwIP
#include <netdb.h>

#define NTP_TIMESTAMP_DELTA 2208988800ull

#define LI(packet)   (uint8_t) ((packet.li_vn_mode & 0xC0) >> 6) // (li   & 11 000 000) >> 6
#define VN(packet)   (uint8_t) ((packet.li_vn_mode & 0x38) >> 3) // (vn   & 00 111 000) >> 3
#define MODE(packet) (uint8_t) ((packet.li_vn_mode & 0x07) >> 0) // (mode & 00 000 111) >> 0

/* Local headers */
/* To be included for network stack */
#include "lwip.h"
/* For printf */
#include "diag.h"
/* for whateverelse */
#include "main.h"

static const char servers[4][18] ={"0.de.pool.ntp.org",
                                  "1.de.pool.ntp.org",
                                  "2.de.pool.ntp.org",
                                  "3.de.pool.ntp.org"};

void ntp_client(void *argument)
{
  int32_t sockfd, n; // Socket file descriptor and the n return result from writing/reading from the socket.
  const uint32_t portno = 123; // NTP UDP port number.
  const uint32_t _fuse = 3600; // the time is in UT, we add 3600 seconds
  static struct sockaddr_in serv_addr = {0}; // Server address data structure.
  struct hostent *server;      // Server data structure.

  // Structure that defines the 48 byte NTP packet protocol.
  typedef struct
  {
    uint8_t li_vn_mode;      // Eight bits. li, vn, and mode.
                             // li.   Two bits.   Leap indicator.
                             // vn.   Three bits. Version number of the protocol.
                             // mode. Three bits. Client will pick mode 3 for client.

    uint8_t stratum;         // Eight bits. Stratum level of the local clock.
    uint8_t poll;            // Eight bits. Maximum interval between successive messages.
    uint8_t precision;       // Eight bits. Precision of the local clock.

    uint32_t rootDelay;      // 32 bits. Total round trip delay time.
    uint32_t rootDispersion; // 32 bits. Max error aloud from primary clock source.
    uint32_t refId;          // 32 bits. Reference clock identifier.

    uint32_t refTm_s;        // 32 bits. Reference time-stamp seconds.
    uint32_t refTm_f;        // 32 bits. Reference time-stamp fraction of a second.

    uint32_t origTm_s;       // 32 bits. Originate time-stamp seconds.
    uint32_t origTm_f;       // 32 bits. Originate time-stamp fraction of a second.

    uint32_t rxTm_s;         // 32 bits. Received time-stamp seconds.
    uint32_t rxTm_f;         // 32 bits. Received time-stamp fraction of a second.

    uint32_t txTm_s;         // 32 bits and the most important field the client cares about. Transmit time-stamp seconds.
    uint32_t txTm_f;         // 32 bits. Transmit time-stamp fraction of a second.

  } ntp_packet;              // Total: 384 bits or 48 bytes.

  // Create and zero out the packet. All 48 bytes worth.
#if 0
  ntp_packet packet = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
  memset( &packet, 0, sizeof( ntp_packet ) );
#else
    static ntp_packet packet = {0};
/* Using static, packet goes in .bss and initialized to zero.
    {
        uint32_t *_p = (uint32_t*) &packet;
        register const uint32_t _z = 0x0;
        for (uint32_t i=0; i<12; i++){
            *_p++ = _z;
        }
    }
*/
#endif
  // Set the first byte's bits to 00,011,011 for li = 0, vn = 3, and mode = 3. The rest will be left set to zero.
  *((char *) &packet + 0) = 0x1b; // Represents 27 in base 10 or 00011011 in base 2.

  // Create a UDP socket, convert the host-name to an IP address, set the port number,
  // connect to the server, send the packet, and then read in the return packet.
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // Create a UDP socket.

    if ( sockfd < 0 ) {
        RAW_DIAG("ERROR opening socket");
    }

    for (uint32_t i=0; i<4; i++){
        server = gethostbyname(servers[i]); // Convert URL to IP.
        if (server == NULL) {
            RAW_DIAG( "[ ERROR ] host");
        }
        else {
            i = 4;
        }
    }

  // Zero out the server address structure.
  /* initialized at zero because static
   bzero((char*) &serv_addr, sizeof(serv_addr));
  */
  serv_addr.sin_family = AF_INET;

  // Copy the server's IP address to the server address structure.
  bcopy((char*)server->h_addr, (char*) &serv_addr.sin_addr.s_addr, server->h_length );

  // Convert the port number integer to network big-endian style and save it to the server address structure.
  serv_addr.sin_port = htons(portno);

  // Call up the server using its IP address and port number.
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0 ) {
        RAW_DIAG("[ ERROR ] connect");
    }

  // Send it the NTP packet it wants. If n == -1, it failed.
  n = write(sockfd, (char*) &packet, sizeof(ntp_packet));

    if (n < 0) {
        RAW_DIAG("[ ERROR ] write");
    }

  // Wait and receive the packet back from the server. If n == -1, it failed.
  n = read(sockfd, (char*) &packet, sizeof(ntp_packet));

    if (n < 0) {
        RAW_DIAG("[ ERROR ] read");
    }
    
  // These two fields contain the time-stamp seconds as the packet left the NTP server.
  // The number of seconds correspond to the seconds passed since 1900.
  // ntohl() converts the bit/byte order from the network's to host's "endianness".
  packet.txTm_s = ntohl( packet.txTm_s ); // Time-stamp seconds.
  packet.txTm_f = ntohl( packet.txTm_f ); // Time-stamp fraction of a second.

  // Extract the 32 bits that represent the time-stamp seconds (since NTP epoch) from when the packet left the server.
  // Subtract 70 years worth of seconds from the seconds since 1900.
  // This leaves the seconds since the UNIX epoch of 1970.
  // (1900)------------------(1970)**************************************(Time Packet Left the Server)
  time_t txTm = (time_t) ( packet.txTm_s - NTP_TIMESTAMP_DELTA + _fuse);

  // Print the time we got from the server, accounting for local timezone and conversion from UTC time.
  //   uint32_t _beg = DWT->CYCCNT;
#if 0
 /* approx 8K clocks */
  sprintf((char*) argument, "<p>Time: %s</p>", ctime((const time_t*) &txTm));
#else
    /* 6.5K clocks and no malloc */
    ctime_r((const time_t*) &txTm, argument);
#endif
   /*
   uint32_t _end = DWT->CYCCNT;
    _end -= _beg;
    char _qaz[16];
    sprintf(_qaz, "%lu", _end);
    RAW_DIAG(_qaz);
   */
#ifdef DEBUG
  RAW_DIAG(argument);
#endif
    close(sockfd);
}