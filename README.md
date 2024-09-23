Two programs create and check a blockchain of messages created entirely in C using a provided SHA256 hash and Base64 encoder. The blockchain is a log of messages produced and stored on a text file using addlog, which can be verified using checklog.

addlog begins by creating a loghead.txt file and then a log.txt file if they do not exist. Then, it establishes the first message on a new log.txt file with the word "begin" in place of the base64 encoded hash of a previous message. A b64 encoded hash of this message is then put into loghead.txt. Subsequent messages are formatted with a timestamp, a b64 encoded hash of the last line, and then the user input. loghead.txt is changed accordingly, and the program continues as usual. Due to the medium of containment being a single line, user content cannot contain newline characters, so they are converted into spaces when processed.

checklog checks for both log files and then begins working through log.txt from the first line to check if each message is validated by its successor; if it catches an invalid or modified line, it notifies the user where the modification was via the command line.

This program was created for Linux and comes with a makefile for easy compiling. Syntax is as follows.

./addlog user_content

./checklog

user_content must be one string. For long messages with spaces you can use quotations to encapsulate the content.
