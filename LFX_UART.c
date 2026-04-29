#include<termios.h>// termios
#include<fcntl.h>// file control
#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>// for timeout part
#include<errno.h>// shows error when certain requirement is failed


int main()
{
  struct termios configurations;
 
  printf("LFX Mentornship UART Task\n");
  char device[] ="/dev/ttyUSB0";// device name
  int fd=open(device,O_RDWR | O_NOCTTY | O_NONBLOCK);// to open device and assign read/write, non-blocking , no controlling terminal
  if(fd==-1)//if device is not present
  {
    printf("Couldn't open port\n");
    return 1;
  }
  // if device is other than tty then gives error
  if(!isatty(fd))
  {
    printf(" Device is not TTY\n");
    close(fd);
  return 1;
  }

  //if didn't get required attributes of termios , displays error.
  if(tcgetattr(fd, &configurations)==-1)
  {
    perror("Termios failed\n");
    close(fd);
    return 1;
  }
  // To set baud rate i.e 9600
  cfsetispeed(&configurations,B9600);//input baud rate
  cfsetospeed(&configurations,B9600);//output baudrate
 
  //Basically, different flags control various parameters of communication
  configurations.c_lflag &= ~(ICANON | ECHO | ECHONL | IEXTEN | ISIG);// enable non-canonical mode for serial port
  configurations.c_iflag &= ~(IGNBRK | BRKINT | ICRNL | INLCR | PARMRK | INPCK | ISTRIP | IXON);// clears these flags
  configurations.c_cflag |= CREAD | CLOCAL;// turns on receiver of serial port
  configurations.c_cflag &= ~CRTSCTS;// turns off RTS/CTS (hardware-based control)
  configurations.c_cflag &= ~PARENB;//zero parity
  configurations.c_cflag &= ~CSTOPB;// one stop bit
  configurations.c_cflag &= ~CSIZE;// clears the bits (bitmasking)
  configurations.c_cflag |= CS8;//Alows 8bits
  configurations.c_oflag &= ~OPOST;//alows no output processing
  //set 0 for purely non-blocking mode
  configurations.c_cc[VMIN]=0;// minimum characters =0 means as soon as something is received print it.
  configurations.c_cc[VTIME]=0;// doesn't wait immediately returns
 
  // attributes of termios not applied
  if(tcsetattr(fd,TCSANOW,&configurations)==-1)
  {
    perror("Attributes not applied\n");
    return 1;
  }
 
  // clears all the garbage that is in the "buffer" and gets ready for new transmission
  if(tcflush(fd, TCIOFLUSH)!=0)
  {
    perror("tcflush");
  }
 
 
 
  char message[] = "Haris here\n";

/* Transmit the string */
int message_transmit = write(fd, message, strlen(message));
//if message being sent less than 0 bytes, then show error
if (message_transmit < 0)
{
    perror("Can't write to port");
    close(fd);
    exit(1);
}

printf("Sent: %s", message);



char buffer[100];// can receive upto 100 bytes of data , ignores bytes >100 and just prints till 100 bytes.
int received=0;
time_t start_time =time(NULL);

// infinite loop keep checking when data is sent and act accordingly
while(1)
{
  int bytes=read(fd,buffer+received, sizeof(buffer)-received-1);
  // same concept as message sent <0
  if(bytes<0)
  {
    if(errno != EAGAIN && errno !=EWOULDBLOCK)
    {
     perror("No data to read");
     close(fd);
     return 1;
    }
  }
  else
  {
    received= received+bytes;// data is received along with number of bytes
  }

// checks if difference between current and start time is 5 sec
  if(time(NULL)-start_time>= 5)
  {
  break;// breaks the loop if no data comes within specified timeout period of 5 sec
  }
 
  usleep(10000);// 10ms delay to prevent CPU from busy waiting
}
  buffer[received]='\0';// null terminate the string
  if(received>0)
  {
    printf("Received (%d bytes):%s\n ",received,buffer);
  }
  else
  {
    printf("No data received with this period");
  }





  close (fd);// closes file descriptor
  return 0;

}
