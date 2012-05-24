#include "ulib.h"

int main( void ) {
	Status status;
	Pid pid;
	char *message = "This is the test message.";

	status = fork( &pid );
	if ( status == SUCCESS ) {
		if ( pid > 0) { //in parent
			sleep(2); //sleep for two seconds
			status = message_send( pid, message, 26 );
			if ( status == SUCCESS ) {
				writef( "User messages_test(Parent) message_send status SUCCESS\n", status);
			} else {
				writef( "User messages_test(Parent) message_send status FAILED(%d)\n", status);
				exit();
			}
		} else { //in child
			char *receivedMessage;
			Uint32 size;
			Pid fromPid;
			//sleep(2);
			writef( "User messages_test(Child) waiting for message\n");
			status = message_receive( &fromPid, (void**) &receivedMessage, &size );
			if ( status == SUCCESS ) {
				writef( "User messages_test(Child) received from %d of size %d: %s\n", fromPid, size, receivedMessage );
			} else {
				writef( "User messages_test(Child) message_receive status FAILED(%d)\n", status);
				exit();
			}
		}
	} else {
		writef( "User messages_test fork status FAILED(%d)\n", status);
	}
}