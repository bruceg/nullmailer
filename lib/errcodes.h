#ifndef NULLMAILER__ERRCODES__H__
#define NULLMAILER__ERRCODES__H__

// Temporary errors
#define ERR_USAGE 1		// Invalid command-line arguments
#define ERR_HOST_NOT_FOUND 2	// gethostbyname failed with HOST_NOT_FOUND
#define ERR_NO_ADDRESS 3	// gethostbyname failed with NO_ADDRESS
#define ERR_GHBN_TEMP 5		// gethostbyname failed with TRY_AGAIN
#define ERR_SOCKET 6		// socket failed
#define ERR_CONN_REFUSED 7	// connect failed with ECONNREFUSED
#define ERR_CONN_TIMEDOUT 8	// connect failed with ETIMEDOUT
#define ERR_CONN_UNREACHABLE 9	// connect failed with ENETUNREACH
#define ERR_CONN_FAILED 10	// connect failed
#define ERR_PROTO 11		// unexpected result from server
#define ERR_MSG_OPEN 12		// could not open the message
#define ERR_MSG_READ 13		// reading the message failed
#define ERR_MSG_WRITE 14	// writing the message failed
#define ERR_EXEC_FAILED 15	// executing a program failed
#define ERR_MSG_TEMPFAIL 16	// server temporarily failed to receive
#define ERR_UNKNOWN 17		// Arbitrary error code
#define ERR_CONFIG 18		// Error reading a config file
#define ERR_BIND_FAILED 19      // Failed to bind source address

// Permanent errors
#define ERR_GHBN_FATAL 33	// gethostbyname failed with NO_RECOVERY
#define ERR_MSG_REFUSED 34	// server refused the message
#define ERR_MSG_PERMFAIL 35	// server permanently failed to receive

#define ERR_PERMANENT_FLAG 32

extern const char* errorstr(int);

#endif // NULLMAILER__ERRCODES__H__
