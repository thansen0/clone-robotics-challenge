#ifndef SERVER_HPP
#define SERVER_HPP

/*
 * Reads in command line arguments and sets them as static variables
 */
void cli_args(int argc, char* argv[]);

/*
 * Generates random payload data and sets generation time
 */
Payload_IMU_t calcRandPayload();

#endif // SERVER_CPP
