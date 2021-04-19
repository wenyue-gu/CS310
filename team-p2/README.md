
# Stack Smash

The objective of this project is to compromise vulnerable webserver
and use it to give yourself arbitrary shell access.  The source code
for the server you'll be compromising is in webserver.c.

There are five stages to completing the project

1. Find the stack-smash vulnerability in the server code.

2. Run a local copy of the server and use gdb to identify where the
relevant variables are stored in memory.  You'll also be able to
modify the code to help you figure it out - note however that your
exploit will be tested on the unmodified server.

3. Construct a request string that will exploit the vulnerability by
overwriting variables stored on the stack.  We've provided some
shellcode - basically assembly language instructions you'll want to
inject into the servers memory.  When this assembly is run, it will
open a port on your server that will allow the execution of arbitrary
commands.  This shellcode won't work on its own however - it will need
to be enclosed in text that makes it looks like a request the server
can handle.  The request string will also need to correctly overwrite
the return address of the running function to let the shell code run.

4. Mount an attack to "pwn" your local webserver process. Make sure that
you can reliably exploit your local server.

5. Submit.  Bear in mind there are no automated tests for this
assignment - it'll be handgraded.  So make sure it works before you
submit.


# Building and running your local server

Build/run the given webserver like this:

    gcc -m32 -z execstack -fno-stack-protector webserver.c -o webserver
    ./webserver 8080

This starts the server listening on port 8080.

To make this server exploitable, we're compiling in a way that
disables some stack smashing protections that are built into modern
gccs.  The OS also has a built in feature called Address Space Layout
Randomization (ASLR) - that would normally make our lives quite
difficult...but the webserver code is built in a nonstandard way that
fixes the stack at a particular address in memory.  We're making life
easier for you with this assignment - but even on a system with these
other features the principles of a buffer overflow attack remain the
same.
    
*Safe stacks: if you choose to smash, smash responsibly.* This
activity is fun and healthy but it can be dangerous. Practice good
hygiene: please do not leave these processes running when they are not
in active use. Use protection: run your processes in a contained
environment (container or VM). Don't use public machines: if someone
else hacks one of your processes (easy by design) they will control
your user account.  OIT Security and CS Staff should not be finding
vulnerable processes lying around the network. Have fun, but stay
safe!

# Connecting to the server

You can connect to your server using a web browser BUT this can be a
little difficult in a docker image which doesn't expose its ports
externally.  If your configure your docker right and visit a URL like
http://localhost:8080, it returns a short default web page
(index.html) announcing itself.  However, this web server is very
simple, and it is unstable under attack, and browsers may expect too
much from it. Some browsers behave erratically with this lab.

It is easier to use the "netcat" utility (i.e., the nc command) to send
messages to your server. 

Install netcat like this:

    apt install netcat

Netcat connects to the server and sends its stdin to the server over
the socket, and outputs whatever it receives from the socket to its
stdout. For example, to request the file index.html from your server
using HTTP, you could type in:

    echo -e "GET /index.html HTTP" | nc localhost 8080

You'll want to do this in another shell attached to the same docker
that is actually running the server.  Start a second shell like this
(where cps310 is the name of the docker image you have started or
run):

    docker exec -it cps310-1 bash

You can also run a bare netcat and type directly into its stdin. This
might be useful when your attack is a portbind shell and you want to
connect to the attack port and type shell commands. The verbose option
nc -v is useful to see connection status. For example, type nc -v
YOUR_SERVER YOUR_PORT, hit return, and if you get a successful
connection type: GET /index.html HTTP. You should receive the HTML
source for the default web page. If the webserver returns an HTTP 404
error (not found), make sure you are running the webserver with the
correct current directory. See the source code for other reasons it
might return a 404.

# Launching your attack

Your objective is to send the webserver a request string with embedded
shellcode instructions that causes the webserver process to execute your
shellcode. The shellcode takes over the process and lets the attacker
use the process to attack the system.

The included file shellcode.c provides some example shellcode.  It's
probably a good starting place for you to think about your payload.
You'd run it like this:

    gcc shellcode.c -o shellcode
    ./shellcode | nc localhost 8080
    
This won't work by itself though - you can see if you try it the
webserver rejects it as an invalid request.  I needs to look like a
properly formatted request - see the code of websever to understand
what a proper request looks like.  Just being valid isn't enough
either though - it needs to constructed in a way to exploit the buffer
overflow bug in the server.  Its only when you can start overwriting
random parts of memory with your code that an attack is possible.
Usually being able to cause the webserver to segfault is a good start.
From there, watch the video lecture on stack overflows to get the rest
of the strategy, or look at some of resources we've linked at the
bottom of the page.

The suggested shellcode will "portbind a shell in port 5074". That
means that the shellcode does a bind() system call for port 5074, does
an accept() syscall to accept an incoming connection, dups the
connection socket to its stdin and stdout, and then execs a shell
program from a standard file path on the victim system.

So: after the webserver is hacked, the attacker can connect to the bind
port (e.g., with nc localhost 5074) and input the attacker's chosen shell
commands. The hacker lingo for this concept is /bindshell/. Be sure you
understand how those syscalls work to construct the attack.

# How to submit

You will submit several files but the most important is called
PAYLOAD.  The way we will test your exploit is like this:

    <start the webserver>
    cat PAYLOAD | nc localhost 8080  # we'll ^C cause this will hang if its effective
    nc localhost 5074
    # enter our test commands and inspect results to ensure we have shell access

PAYLOAD should contain the full exploit string you use to attack your
server.  If you generated your string by running shellcode you can
just  "./shellcode > PAYLOAD" to produce an appropriate file.

You should also submit all source files you used to generate your
exploit (e.g. shellcode.c) , and a text file called README.txt with a
short explanation of your method of attack and any experience that
might be useful or interesting.

If you cannot get the full exploit working, a PAYLOAD that
successfully crashes the webserver is worth 40% credit.

# Notes and hints

  * If the webserver returns an HTTP 404 (not found), then your attack
    did not succeed in smashing the stack: either the string was not
    long enough to smash the RA, or the server rejected it before
    copying. A smashed server never returns: it has lost its way
    (smashed RA) and cannot get back to the code that sends a response.
  * If a connect attempt fails with "connection refused", it means the
    server kernel rejected the attempt because no local process is
    listening on the requested port.
  * If your webserver "hangs" after you attack it, that is the desired
    behavior. It is likely blocked in accept() on your bind port.
    Connect to your bind port.
  * If you get a "connection refused" on your expected bind (attack)
    port, it means the attack failed.
  * If a connect to your bind/attack port succeeds, but the process
    crashes immediately, it is likely because the end of your shellcode
    is too close to the top of the stack, and the shellcode's *push*
    instructions overwrote itself.
  * Don't forget to byteswap your RA: send the least significant byte
    first in the attack string.
  * If your attack fails with 404 and the webserver prints "unsupported
    command", it may be because you forgot the / before the filename or
    there is a null character somewhere in the shellcode. 


# Resources

Please post to Piazza with any difficulties you are having.

The instructor and TA can post tips that may point you and others in the
right direction.

Read the document "Smashing the Stack for Fun and Profit"
<http://inst.eecs.berkeley.edu/~cs161/fa08/papers/stack_smashing.pdf>
for an introduction to call stack vulnerabilities.

Look at "Shellcoding for Linux and Windows Tutorial"
<http://www.vividmachines.com/shellcode/shellcode.html> for shellcode
samples for Linux. Past students have found this to be extremely useful.

