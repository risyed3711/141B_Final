I implemented the logger at the trace level, to add support for all possible levels of the logger. I initialized an 
Furthermore, I decided to print messages to std::cout ,  because this is a very versatile place to put messages, 
as I felt that the color of the message did enough to show whether it was an error, general information, etc. 
Next, I created a Logger.cpp file to initialize the Logger::logger in order to get it working, by setting it 
to nullptr. Without doing this, the singleton wasnt being recognized, so I had to allocate memory for it by initializing
it. 

At this point, the logger class itself was functional. The next thing I did was add a logger data member to both the virtual
processor and handler class. This way, every single handler and processor would have access to the logger, 
since they inherit from these classes. Once these were implemented, the next step I took was to add logging information
when a handler is found, and also when the relevant processor is processing the correct command. This concluded
my primary implementation of the logger. I was able to ipmlement this and it worked, it showed proper functionality and 
gave useful information.

One step I took to refactor my code before the final was to fix a bug which I discovered in the wipeBlock method, 
which started failing after I implemented remoting. In this message, essentially, if it was accessed through many pointers, 
instead of directly, which was incurred by remoting, the reading/writing from the stream got corrupted. I fixed this
issue by using one of the mini machines I had, which was the "readBlock" and "writeBlock" method which we made
in BlockIO, in which I cleverly wiped the stream when I first implemented it. This fixed my bug, and made my wipeBlock method
both cleaner to read and more efficient to run.