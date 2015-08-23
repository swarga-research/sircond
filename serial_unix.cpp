//
// SERIAL.CPP
//

#include "pch.h"
#include <termios.h>
#include "serial.h"

//!
//! \file serial_unix.cpp
//! \brief UNIX implementation of the serial port abstraction class.
//!

namespace sr
{

//!
//! \brief UNIX version of the serial port abstraction.
//!
//========================================================================
class UNIXSerialPort : public CSerialPort
{
public:
	UNIXSerialPort ();
	int32_t Open (const string& device);
	int32_t SetDataRate (uint32_t baud);
	int32_t Send (const uint8_t* data, size_t size, uint32_t timeout);
	int32_t Recv (uint8_t* data, size_t maxSize, uint32_t timeout);
	void Close ();

private:
	~UNIXSerialPort ();

	string m_device;
	int m_fd;
};

//========================================================================
UNIXSerialPort::UNIXSerialPort () :
	m_fd(-1)
{

}

//========================================================================
int32_t UNIXSerialPort::SetDataRate (uint32_t baud)
{
    tcflag_t rate_flags = B9600;
    struct termios tio;

    if (m_fd == -1)
    {
        return ErrorInvalidPort;
    }

    switch (baud)
    {
        case 9600U:
            rate_flags = B9600;
        break;

        case 57600U:
            rate_flags = B57600;
        break;

        case 115200U:
            rate_flags = B115200;
        break;
    }

    if (tcgetattr(m_fd, &tio) != 0)
    {
        return ErrorUnspecified;
    }
    tio.c_cflag = rate_flags | CS8 | CLOCAL | CREAD;
    tcflush(m_fd, TCIFLUSH);
    if (tcsetattr(m_fd, TCSANOW, &tio) != 0)
    {
        return ErrorUnspecified;
    }
    return 0;
}

//========================================================================
int32_t UNIXSerialPort::Open (const string& device)
{
    struct termios newtio;

    if (m_fd != -1)
    {
        return ErrorPortInUse;
    }

    m_fd = open(device.c_str(), O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd == -1)
    {
        return ErrorInvalidPort;
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    // SET INPUT MODE: NON-CANONICAL, NO ECHO
    newtio.c_lflag = 0;

    // NO TIMEOUT
    newtio.c_cc[VTIME] = 0;
    newtio.c_cc[VMIN] = 1;

    tcflush(m_fd, TCIFLUSH);
    tcsetattr(m_fd, TCSANOW, &newtio);

    return 0;
}

//========================================================================
void UNIXSerialPort::Close ()
{

    if (m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }
}

//========================================================================
int32_t UNIXSerialPort::Send (const uint8_t* data, size_t size, uint32_t timeout)
{

    if (m_fd == -1)
    {
        return ErrorInvalidPort;
    }

    fd_set fds;
    int result;
    timeval tv;
    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;
    int n = select(m_fd + 1, 0, &fds, 0, &tv);

    if (n == 0)
    {
        return ErrorTimeout;
    }

    result = write(m_fd, data, size);
    if (result < 0)
    {
        return ErrorTransmitError;
    }
    return result;
}

//========================================================================
int32_t UNIXSerialPort::Recv (uint8_t* data, size_t maxSize, uint32_t timeout)
{
    fd_set fds;
    int result;
    timeval tv;

    if (m_fd == -1)
    {
        return ErrorInvalidPort;
    }

    FD_ZERO(&fds);
    FD_SET(m_fd, &fds);
    tv.tv_sec = 0;
    tv.tv_usec = timeout * 1000;
    int n = select(m_fd + 1, &fds, 0, 0, &tv);

    if (n == 0)
    {
        return ErrorTimeout;
    }

    result = read(m_fd, data, maxSize);
    if (result < 0)
    {
        return ErrorReceiveError;
    }

    return result;
}

//========================================================================
UNIXSerialPort::~UNIXSerialPort ()
{

	Close();
}

//========================================================================
CSerialPort* CSerialPort::New ()
{

	return new UNIXSerialPort;
}

//========================================================================
CSerialPort::~CSerialPort ()
{

}

}

