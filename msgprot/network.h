#ifndef network_h
#define network_h

int tcp_connect(char *name, unsigned short int port);
int tcp_listen(unsigned short int port, unsigned int backlog);
int tcp_accept(int socket);
char *socket_peer_name(int socket);
unsigned short int socket_peer_port(int socket);
ssize_t tcp_write(int fd, const void *buf, size_t len);

#endif